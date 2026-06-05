module;
#include <tchar.h>

#include <cstdint>
#include <memory>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "src/base/color.h"

export module whiteboard:font;

import :atlas;

export namespace gui {

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
  math::Rect UV;
  math::Point Offset;
  int32_t Advance = 0;
};

struct WBSYMBOL {
  WBATLASHANDLE Handle = 0;
  int16_t OffsetX = 0, OffsetY = 0;
  uint16_t SizeX = 0, SizeY = 0;
  int16_t Advance = 0;
  uint16_t Char = 0;

  math::Rect calculatedContentRect;
};

struct WBKERNINGDATA {
  uint16_t First, Second;
  int16_t Amount;
};

class KerningPair {
 public:
  uint32_t first_, second_;

  KerningPair();
  KerningPair(uint16_t a, uint16_t b);

  constexpr friend bool operator==(const KerningPair& lhs,
                                   const KerningPair& rhs) = default;
};

uint32_t DictionaryHash(const KerningPair& i);

struct DHash {
  std::size_t operator()(const KerningPair& i) const {
    return DictionaryHash(i);
  }
};

class DrawAPI;

class FontDescription {
  friend class Font;

 public:
  FontDescription();
  ~FontDescription();

  // 32 bit raw image data
  bool LoadBMFontBinary(uint8_t* Binary, int32_t BinarySize,
                        const uint8_t* Image, int32_t XRes, int32_t YRes,
                        std::vector<int>* enabledGlyphs);
  bool LoadBMFontBinary(uint8_t* Binary, int32_t BinarySize,
                        const uint8_t* Image, int32_t XRes, int32_t YRes) {
    std::vector<int> eg;
    return LoadBMFontBinary(Binary, BinarySize, Image, XRes, YRes, &eg);
  }
  // 32 bit raw image data
  bool LoadBMFontText(uint8_t* Binary, int32_t BinarySize, const uint8_t* Image,
                      int32_t XRes, int32_t YRes,
                      std::vector<int>* enabledGlyphs);
  bool LoadBMFontText(uint8_t* Binary, int32_t BinarySize, const uint8_t* Image,
                      int32_t XRes, int32_t YRes) {
    std::vector<int> eg;
    return LoadBMFontText(Binary, BinarySize, Image, XRes, YRes, &eg);
  }

 private:
  std::unique_ptr<uint8_t[]> image_;
  int32_t x_res_ = 0, y_res_ = 0;

  std::vector<WBSYMBOLINPUT> alphabet_;
  std::vector<WBKERNINGDATA> kerning_data_;

  int32_t line_height_ = 0;
  int32_t base_ = 0;
};

class Font {
 public:
  explicit Font(Atlas* Atlas);
  virtual ~Font();
  bool Initialize(FontDescription* Description, TCHAR missing_char_ = 'o');

  int32_t GetLineHeight();
  int32_t GetBase();
  int32_t GetOffsetX(TCHAR Char);
  int32_t GetOffsetY(TCHAR Char);
  int32_t GetCenterWidth(
      int32_t x1, int32_t x2, std::string_view Text,
      WBTEXTTRANSFORM Transform = WBTEXTTRANSFORM::WBTT_NONE);
  int32_t GetCenterHeight(int32_t y1, int32_t y2);
  math::Point GetCenter(std::string_view Text, math::Rect Rect,
                        WBTEXTTRANSFORM Transform = WBTEXTTRANSFORM::WBTT_NONE);
  int32_t GetMedian();

  int32_t WriteChar(DrawAPI* DrawApi, int Char, int32_t x, int32_t y,
                    CColor Color = CColor(0xffffffff));
  int32_t Write(DrawAPI* DrawApi, std::string_view String, int32_t x, int32_t y,
                CColor Color = CColor(0xffffffff),
                WBTEXTTRANSFORM Transform = WBTEXTTRANSFORM::WBTT_NONE,
                bool DoKerning = true);
  int32_t WriteChar(DrawAPI* DrawApi, int Char, const math::Point& p,
                    CColor Color = CColor(0xffffffff));
  int32_t Write(DrawAPI* DrawApi, std::string_view String, const math::Point& p,
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

  math::Point GetTextPosition(std::string_view String,
                              const math::Rect& Container,
                              WBTEXTALIGNMENTX XAlign, WBTEXTALIGNMENTY YAlign,
                              WBTEXTTRANSFORM Transform, bool DoKerning = true);

  char ApplyTextTransform(const char* Text, const char* CurrPos,
                          WBTEXTTRANSFORM Transform);

  void ConvertToUppercase();

 private:
  void AddSymbol(uint16_t Char, WBATLASHANDLE Handle, const math::Size& Size,
                 const math::Point& Offset, int32_t Advance,
                 math::Rect contentRect);
  void AddKerningPair(uint16_t First, uint16_t Second, int16_t Amount);
  uint16_t ApplyTextTransformUtf8(const char* Text, char const*& CurrPos,
                                  WBTEXTTRANSFORM Transform);

  Atlas* atlas_;
  int32_t alphabet_size_ = 0;
  std::unique_ptr<WBSYMBOL[]> alphabet_;
  std::unordered_map<KerningPair, int16_t, DHash> kerning_;

  int32_t line_height_ = 0;
  int32_t base_ = 0;

  int32_t offset_x_char_ = 0;
  int32_t height_x_char_ = 0;

  TCHAR missing_char_ = 0;
};

}  // namespace gui
