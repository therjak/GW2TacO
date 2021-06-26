#pragma once
#include "Atlas.h"

enum WBTEXTALIGNMENTX
{
  WBTA_CENTERX,
  WBTA_LEFT,
  WBTA_RIGHT,
};

enum WBTEXTALIGNMENTY
{
  WBTA_CENTERY,
  WBTA_TOP,
  WBTA_BOTTOM,
};

enum WBTEXTTRANSFORM
{
  WBTT_NONE = 0,
  WBTT_CAPITALIZE,
  WBTT_UPPERCASE,
  WBTT_LOWERCASE,
};

struct WBSYMBOLINPUT
{
  TU16 Char;
  CRect UV;
  CPoint Offset;
  int32_t Advance;
};

struct WBSYMBOL
{
  WBATLASHANDLE Handle;
  int16_t OffsetX, OffsetY;
  TU16 SizeX, SizeY;
  int16_t Advance;
  TU16 Char;

  CRect calculatedContentRect;
};

struct WBKERNINGDATA
{
  TU16 First, Second;
  int16_t Amount;
};

class CWBKerningPair
{
public:

  TU32 First, Second;

  INLINE TBOOL operator==( const CWBKerningPair &k );
  CWBKerningPair();
  CWBKerningPair( TU16 a, TU16 b );
};

INLINE TU32 DictionaryHash( const CWBKerningPair &i );

class CWBDrawAPI;

class CWBFontDescription
{
  friend class CWBFont;

  TU8 *Image;
  int32_t XRes, YRes;

  CArray<WBSYMBOLINPUT> Alphabet;
  CArray<WBKERNINGDATA> KerningData;

  int32_t LineHeight;
  int32_t Base;

public:

  CWBFontDescription();
  ~CWBFontDescription();

  TBOOL LoadBMFontBinary( TU8 *Binary, int32_t BinarySize, TU8 *Image, int32_t XRes, int32_t YRes, CArray<int>& enabledGlyphs = CArray<int>() ); //32 bit raw image data
  TBOOL LoadBMFontText( TU8 *Binary, int32_t BinarySize, TU8 *Image, int32_t XRes, int32_t YRes, CArray<int>& enabledGlyphs = CArray<int>() ); //32 bit raw image data
};

class CWBFont
{
  CAtlas *Atlas;
  //CDictionary<TU16,WBSYMBOL> Alphabet;
  int32_t AlphabetSize = 0;
  WBSYMBOL *Alphabet;
  CDictionary<CWBKerningPair, int16_t> Kerning;

  int32_t LineHeight;
  int32_t Base;

  int32_t Offset_X_Char;
  int32_t Height_X_Char;

  TCHAR MissingChar;

  void AddSymbol( TU16 Char, WBATLASHANDLE Handle, CSize &Size, CPoint &Offset, int32_t Advance, CRect contentRect );
  void AddKerningPair( TU16 First, TU16 Second, int16_t Amount );

public:

  CWBFont( CAtlas *Atlas );
  virtual ~CWBFont();
  TBOOL Initialize( CWBFontDescription *Description, TCHAR MissingChar = _T( 'o' ) );

  int32_t GetLineHeight();
  int32_t GetBase();
  int32_t GetOffsetX( TCHAR Char );
  int32_t GetOffsetY( TCHAR Char );
  int32_t GetCenterWidth( int32_t x1, int32_t x2, TCHAR *Text, WBTEXTTRANSFORM Transform = WBTT_NONE );
  int32_t GetCenterWidth( int32_t x1, int32_t x2, CString &Text, WBTEXTTRANSFORM Transform = WBTT_NONE );
  int32_t GetCenterHeight( int32_t y1, int32_t y2 );
  CPoint GetCenter( TCHAR *Text, CRect Rect, WBTEXTTRANSFORM Transform = WBTT_NONE );
  CPoint GetCenter( CString &Text, CRect Rect, WBTEXTTRANSFORM Transform = WBTT_NONE );
  int32_t GetMedian();

	int32_t WriteChar(CWBDrawAPI *DrawApi, int Char, int32_t x, int32_t y, CColor Color = 0xffffffff);
  int32_t Write( CWBDrawAPI *DrawApi, TCHAR *String, int32_t x, int32_t y, CColor Color = 0xffffffff, WBTEXTTRANSFORM Transform = WBTT_NONE, TBOOL DoKerning = true );
  int32_t Write( CWBDrawAPI *DrawApi, CString &String, int32_t x, int32_t y, CColor Color = 0xffffffff, WBTEXTTRANSFORM Transform = WBTT_NONE, TBOOL DoKerning = true );
	int32_t WriteChar(CWBDrawAPI *DrawApi, int Char, CPoint &p, CColor Color = 0xffffffff);
  int32_t Write( CWBDrawAPI *DrawApi, TCHAR *String, CPoint &p, CColor Color = 0xffffffff, WBTEXTTRANSFORM Transform = WBTT_NONE, TBOOL DoKerning = true );
  int32_t Write( CWBDrawAPI *DrawApi, CString &String, CPoint &p, CColor Color = 0xffffffff, WBTEXTTRANSFORM Transform = WBTT_NONE, TBOOL DoKerning = true );
  int32_t GetWidth( TU16 Char, TBOOL Advance = true ); //if Advance is set to false this returns the width of the image in pixels
  int32_t GetWidth( TCHAR *String, TBOOL AdvanceLastChar = true, WBTEXTTRANSFORM Transform = WBTT_NONE, TBOOL DoKerning = true, TBOOL firstCharHack = false );
  int32_t GetWidth( CString &String, TBOOL AdvanceLastChar = true, WBTEXTTRANSFORM Transform = WBTT_NONE, TBOOL DoKerning = true, TBOOL firstCharHack = false );

  int32_t GetHeight( TU16 Char );
  int32_t GetHeight( TCHAR* String );
  int32_t GetHeight( CString& String );

  CPoint GetTextPosition( CString &String, CRect &Container, WBTEXTALIGNMENTX XAlign, WBTEXTALIGNMENTY YAlign, WBTEXTTRANSFORM Transform, TBOOL DoKerning = true );
  CPoint GetTextPosition( TCHAR *String, CRect &Container, WBTEXTALIGNMENTX XAlign, WBTEXTALIGNMENTY YAlign, WBTEXTTRANSFORM Transform, TBOOL DoKerning = true );

  INLINE TU16 ApplyTextTransformUtf8( _TUCHAR *Text, _TUCHAR *&CurrPos, WBTEXTTRANSFORM Transform );
  INLINE _TUCHAR ApplyTextTransform( _TUCHAR *Text, _TUCHAR *CurrPos, WBTEXTTRANSFORM Transform );

  void ConvertToUppercase();

};
