#include "src/base/image_decompressor.h"

#include <comdef.h>
#include <olectl.h>

#include <memory>

#include "src/base/logger.h"

#define HIMETRIC_INCH 2540

std::unique_ptr<uint8_t[]> DecompressImage(const uint8_t* ImageData,
                                           int32_t ImageDataSize,
                                           int32_t& XSize, int32_t& YSize) {
  if (!ImageData || !ImageDataSize) return nullptr;

  XSize = YSize = 0;
  LPPICTURE gpPicture = nullptr;

  LPVOID pvData = nullptr;
  HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, ImageDataSize);

  if (!hGlobal) return nullptr;

  pvData = GlobalLock(hGlobal);
  if (!pvData) {
    GlobalFree(hGlobal);
    return nullptr;
  }

  memcpy(pvData, ImageData, ImageDataSize);

  GlobalUnlock(hGlobal);

  if (GetLastError() != NO_ERROR) {
    GlobalFree(hGlobal);
    return nullptr;
  }

  LPSTREAM pstm = nullptr;
  HRESULT res = CreateStreamOnHGlobal(hGlobal, TRUE, &pstm);
  if (res != S_OK) {
    _com_error err(res);
    LOG_ERR("[base] CreateStreamOnHGlobal failed (%s)", err.ErrorMessage());
    return nullptr;
  }

  res = OleLoadPicture(pstm, ImageDataSize, FALSE, IID_IPicture,
                       reinterpret_cast<LPVOID*>(&gpPicture));

  if (res != S_OK) {
    if (res != 0x800A01E1)  // this is given when a png is loaded through
                            // oleloadpicture
    {
      _com_error err(res);
      LOG_ERR("[base] OleLoadPicture failed (%s)", err.ErrorMessage());
    }
    pstm->Release();
    GlobalFree(hGlobal);
    return nullptr;
  }

  pstm->Release();
  GlobalFree(hGlobal);

  if (!gpPicture) return nullptr;

  OLE_XSIZE_HIMETRIC hmWidth;
  OLE_YSIZE_HIMETRIC hmHeight;

  gpPicture->get_Width(&hmWidth);
  gpPicture->get_Height(&hmHeight);

  HDC hdc = GetDC(nullptr);
  HDC mdc = CreateCompatibleDC(hdc);

  XSize = MulDiv(hmWidth, GetDeviceCaps(mdc, LOGPIXELSX), HIMETRIC_INCH);
  YSize = MulDiv(hmHeight, GetDeviceCaps(mdc, LOGPIXELSY), HIMETRIC_INCH);

  auto Image = std::make_unique<uint8_t[]>(XSize * YSize * 4);
  memset(Image.get(), 0, XSize * YSize * 4);

  HBITMAP bm = CreateCompatibleBitmap(hdc, XSize, YSize);
  BITMAPINFO bmi;
  bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
  bmi.bmiHeader.biWidth = XSize;
  bmi.bmiHeader.biHeight = YSize;
  bmi.bmiHeader.biPlanes = 1;
  bmi.bmiHeader.biBitCount = 32;
  bmi.bmiHeader.biCompression = BI_RGB;

  HGDIOBJ oldbm = SelectObject(mdc, bm);
  RECT r = {0, 0, XSize, YSize};

  FillRect(mdc, &r, static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH)));

  SetBkMode(mdc, TRANSPARENT);

  res = gpPicture->Render(mdc, 0, YSize - 1, XSize, -YSize, 0, hmHeight,
                          hmWidth, -hmHeight, &r);

  if (res != S_OK) {
    _com_error err(res);
    LOG_ERR("[base] gpPicture->Render failed (%s)", err.ErrorMessage());
    Image.reset();
  } else {
    GetDIBits(mdc, bm, 0, YSize, Image.get(), &bmi, DIB_RGB_COLORS);
  }

  SelectObject(mdc, oldbm);
  DeleteDC(mdc);
  DeleteObject(bm);
  ReleaseDC(nullptr, hdc);

  gpPicture->Release();

  for (int32_t x = 0; x < XSize * YSize; x++) Image[x * 4 + 3] = 255;

  return Image;
}
