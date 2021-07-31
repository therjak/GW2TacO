#pragma once
#include <cstdint>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "../BaseLib/Color.h"
#include "Atlas.h"

enum class WBTEXTALIGNMENTX : uint8_t {
  WBTA_CENTERX,
  WBTA_LEFT,
  WBTA_RIGHT,
};

enum class WBTEXTALIGNMENTY : uint8_t {
  WBTA_CENTERY,
  WBTA_TOP,
  WBTA_BOTTOM,
};

enum class WBTEXTTRANSFORM : uint8_t {
  WBTT_NONE = 0,
  WBTT_CAPITALIZE,
  WBTT_UPPERCASE,
  WBTT_LOWERCASE,
};

struct WBSYMBOLINPUT {
  uint16_t Char = 0;
  CRect UV;
  CPoint Offset;
  int32_t Advance = 0;
};

struct WBSYMBOL {
  WBATLASHANDLE Handle = 0;
  int16_t OffsetX = 0, OffsetY = 0;
  uint16_t SizeX = 0, SizeY = 0;
  int16_t Advance = 0;
  uint16_t Char = 0;

  CRect calculatedContentRect;
};

struct WBKERNINGDATA {
  uint16_t First, Second;
  int16_t Amount;
};

class CWBKerningPair {
 public:
  uint32_t First, Second;

  CWBKerningPair();
  CWBKerningPair(uint16_t a, uint16_t b);
};

inline bool operator==(const CWBKerningPair& lhs, const CWBKerningPair& rhs) {
  return lhs.First == rhs.First && lhs.Second == rhs.Second;
}

INLINE uint32_t DictionaryHash(const CWBKerningPair& i);

struct DHash {
  std::size_t operator()(const CWBKerningPair& i) const {
    return DictionaryHash(i);
  }
};

class CWBDrawAPI;

class CWBFontDescription {
  friend class CWBFont;

  std::unique_ptr<uint8_t[]> Image;
  int32_t XRes = 0, YRes = 0;

  std::vector<WBSYMBOLINPUT> Alphabet;
  std::vector<WBKERNINGDATA> KerningData;

  int32_t LineHeight = 0;
  int32_t Base = 0;

 public:
  CWBFontDescription();
  ~CWBFontDescription();

  bool LoadBMFontBinary(
      uint8_t* Binary, int32_t BinarySize, uint8_t* Image, int32_t XRes,
      int32_t YRes,
      std::vector<int>& enabledGlyphs);  // 32 bit raw image data
  bool LoadBMFontBinary(uint8_t* Binary, int32_t BinarySize, uint8_t* Image,
                        int32_t XRes, int32_t YRes) {
    std::vector<int> eg;
    return LoadBMFontBinary(Binary, BinarySize, Image, XRes, YRes, eg);
  }
  bool LoadBMFontText(
      uint8_t* Binary, int32_t BinarySize, uint8_t* Image, int32_t XRes,
      int32_t YRes,
      std::vector<int>& enabledGlyphs);  // 32 bit raw image data
  bool LoadBMFontText(uint8_t* Binary, int32_t BinarySize, uint8_t* Image,
                      int32_t XRes, int32_t YRes) {
    std::vector<int> eg;
    return LoadBMFontText(Binary, BinarySize, Image, XRes, YRes, eg);
  }
};

class CWBFont {
  CAtlas* Atlas;
  int32_t AlphabetSize = 0;
  std::unique_ptr<WBSYMBOL[]> Alphabet;
  std::unordered_map<CWBKerningPair, int16_t, DHash> Kerning;

  int32_t LineHeight = 0;
  int32_t Base = 0;

  int32_t Offset_X_Char = 0;
  int32_t Height_X_Char = 0;

  TCHAR MissingChar = 0;

  void AddSymbol(uint16_t Char, WBATLASHANDLE Handle, CSize& Size,
                 const CPoint& Offset, int32_t Advance, CRect contentRect);
  void AddKerningPair(uint16_t First, uint16_t Second, int16_t Amount);
  INLINE uint16_t ApplyTextTransformUtf8(const char* Text, char const*& CurrPos,
                                         WBTEXTTRANSFORM Transform);

 public:
  explicit CWBFont(CAtlas* Atlas);
  virtual ~CWBFont();
  bool Initialize(CWBFontDescription* Description, TCHAR MissingChar = _T('o'));

  int32_t GetLineHeight();
  int32_t GetBase();
  int32_t GetOffsetX(TCHAR Char);
  int32_t GetOffsetY(TCHAR Char);
  int32_t GetCenterWidth(
      int32_t x1, int32_t x2, std::string_view Text,
      WBTEXTTRANSFORM Transform = WBTEXTTRANSFORM::WBTT_NONE);
  int32_t GetCenterHeight(int32_t y1, int32_t y2);
  CPoint GetCenter(std::string_view Text, CRect Rect,
                   WBTEXTTRANSFORM Transform = WBTEXTTRANSFORM::WBTT_NONE);
  int32_t GetMedian();

  int32_t WriteChar(CWBDrawAPI* DrawApi, int Char, int32_t x, int32_t y,
                    CColor Color = CColor(0xffffffff));
  int32_t Write(CWBDrawAPI* DrawApi, std::string_view String, int32_t x,
                int32_t y, CColor Color = CColor(0xffffffff),
                WBTEXTTRANSFORM Transform = WBTEXTTRANSFORM::WBTT_NONE,
                bool DoKerning = true);
  int32_t WriteChar(CWBDrawAPI* DrawApi, int Char, const CPoint& p,
                    CColor Color = CColor(0xffffffff));
  int32_t Write(CWBDrawAPI* DrawApi, std::string_view String, const CPoint& p,
                CColor Color = CColor(0xffffffff),
                WBTEXTTRANSFORM Transform = WBTEXTTRANSFORM::WBTT_NONE,
                bool DoKerning = true);
  int32_t GetWidth(
      uint16_t Char,
      bool Advance = true);  // if Advance is set to false this returns the
                             // width of the image in pixels
  int32_t GetWidth(std::string_view String, bool AdvanceLastChar = true,
                   WBTEXTTRANSFORM Transform = WBTEXTTRANSFORM::WBTT_NONE,
                   bool DoKerning = true, bool firstCharHack = false);

  int32_t GetHeight(uint16_t Char);
  int32_t GetHeight(std::string_view String);

  CPoint GetTextPosition(std::string_view String, const CRect& Container,
                         WBTEXTALIGNMENTX XAlign, WBTEXTALIGNMENTY YAlign,
                         WBTEXTTRANSFORM Transform, bool DoKerning = true);

  INLINE char ApplyTextTransform(const char* Text, const char* CurrPos,
                                 WBTEXTTRANSFORM Transform);

  void ConvertToUppercase();
};
