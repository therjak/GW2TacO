#include "BasePCH.h"
#include "Font.h"
#include "DrawAPI.h"

#include <vector>
#include <algorithm>

INLINE uint32_t DictionaryHash( const CWBKerningPair &i )
{
  return i.First + ( i.Second << 16 );
}

INLINE TBOOL CWBKerningPair::operator==( const CWBKerningPair &k )
{
  return First == k.First && Second == k.Second;
}

CWBKerningPair::CWBKerningPair()
{
  First = Second = 0;
}

CWBKerningPair::CWBKerningPair( uint16_t a, uint16_t b )
{
  First = a;
  Second = b;
}

CWBFontDescription::CWBFontDescription()
{
  Image = NULL;
}

CWBFontDescription::~CWBFontDescription()
{
  SAFEDELETE( Image );
}

TBOOL CWBFontDescription::LoadBMFontBinary( uint8_t *Binary, int32_t BinarySize, uint8_t *img, int32_t xr, int32_t yr, std::vector<int>& enabledGlyphs )
{
  if ( !img || !Binary || BinarySize <= 0 || xr <= 0 || yr <= 0 ) return false;

  XRes = xr;
  YRes = yr;
  Image = new uint8_t[ xr*yr * 4 ];
  memcpy( Image, img, xr*yr * 4 );

#pragma pack(push,1)

  struct BMCOMMON
  {
    uint16_t lineHeight;
    uint16_t base;
    uint16_t scaleW;
    uint16_t scaleH;
    uint16_t pages;
    uint8_t bitField;
    uint8_t alphaChnl;
    uint8_t redChnl;
    uint8_t greenChnl;
    uint8_t blueChnl;
  };

  struct BMCHAR
  {
    uint32_t id;
    uint16_t x;
    uint16_t y;
    uint16_t width;
    uint16_t height;
    int16_t xoffset;
    int16_t yoffset;
    int16_t xadvance;
    uint8_t page;
    uint8_t chnl;
  };

  struct BMKERNINGDATA
  {
    uint32_t first;
    uint32_t second;
    int16_t amount;
  };

#pragma pack(pop)

  CStreamReaderMemory m;
  m.Open( Binary, BinarySize );

  if ( m.ReadByte() != 'B' || m.ReadByte() != 'M' || m.ReadByte() != 'F' || m.ReadByte() != 3 )
  {
    LOG( LOG_ERROR, _T( "[gui] Error loading font data: bad fileformat or unsupported version" ) );
    return false;
  }

  while ( !m.eof() )
  {
    int32_t BlockType = m.ReadByte();
    int32_t BlockSize = m.ReadDWord();
    uint8_t *BlockData = new uint8_t[ BlockSize ];

    if ( m.Read( BlockData, BlockSize ) != BlockSize )
    {
      LOG( LOG_ERROR, _T( "[gui] Error loading font data: unable to read block data" ) );
      return false;
    }

    switch ( BlockType )
    {
    case 1: //info block, ignored
      break;
    case 2: //common data
    {
      if ( BlockSize != sizeof( BMCOMMON ) )
      {
        LOG( LOG_ERROR, _T( "[gui] Error loading font data: common block size doesn't match" ) );
        return false;
      }
      BMCOMMON *cmn = (BMCOMMON*)BlockData;
      if ( cmn->pages > 1 )
      {
        LOG( LOG_ERROR, _T( "[gui] Error loading font data: only single page fonts are supported" ) );
        return false;
      }
      LineHeight = cmn->lineHeight;
      Base = cmn->base;
    }
    break;
    case 3: //page name list
      break;
    case 4: //characters
    {
      if ( BlockSize % sizeof( BMCHAR ) != 0 )
      {
        LOG( LOG_ERROR, _T( "[gui] Error loading font data: character block size doesn't match" ) );
        return false;
      }

      BMCHAR *c = (BMCHAR*)BlockData;

      for ( uint32_t x = 0; x < BlockSize / sizeof( BMCHAR ); x++ )
      {
        if ( enabledGlyphs.empty() || std::find(enabledGlyphs.begin(), enabledGlyphs.end(),  c[ x ].id ) != enabledGlyphs.end() )
        {
          WBSYMBOLINPUT s;
          s.Char = c[ x ].id;
          s.Advance = c[ x ].xadvance;
          s.Offset = CPoint( c[ x ].xoffset, c[ x ].yoffset );
          s.UV = CRect( c[ x ].x, c[ x ].y, c[ x ].x + c[ x ].width, c[ x ].y + c[ x ].height );

          if ( c[ x ].id >= 0 && c[ x ].id <= 0xffff )
            Alphabet.push_back(s);
        }
      }
    }
    break;
    case 5: //kerning data
    {
      if ( BlockSize % sizeof( BMKERNINGDATA ) != 0 )
      {
        LOG( LOG_ERROR, _T( "[gui] Error loading font data: kerning block size doesn't match" ) );
        return false;
      }

      BMKERNINGDATA *k = (BMKERNINGDATA*)BlockData;

      for ( uint32_t x = 0; x < BlockSize / sizeof( BMKERNINGDATA ); x++ )
      {
        WBKERNINGDATA d;
        d.First = k[ x ].first;
        d.Second = k[ x ].second;
        d.Amount = k[ x ].amount;
        KerningData.push_back(d);
      }
    }
    break;

    default:
      LOG( LOG_ERROR, _T( "[gui] Error loading font data: unknown block type" ) );
      return false;
    }


    SAFEDELETEA( BlockData );
  }

  return true;
}

bool ReadInt( const std::string& s, std::string val, int32_t& result )
{
  int p = s.find( val + "=" );
  if ( p == s.npos )
    return false;
  return std::sscanf(s.substr( p ).c_str(), ( val + "=%d" ).c_str(), &result ) == 1;
}

TBOOL CWBFontDescription::LoadBMFontText( uint8_t *Binary, int32_t BinarySize, uint8_t *img, int32_t xr, int32_t yr, std::vector<int>& enabledGlyphs )
{
  if ( !img || !Binary || BinarySize <= 0 || xr <= 0 || yr <= 0 ) return false;

  XRes = xr;
  YRes = yr;
  Image = new uint8_t[ xr*yr * 4 ];
  memcpy( Image, img, xr*yr * 4 );

  CStreamReaderMemory m;
  m.Open( Binary, BinarySize );

  if ( m.ReadByte() != 'i' || m.ReadByte() != 'n' || m.ReadByte() != 'f' || m.ReadByte() != 'o' )
  {
    LOG( LOG_ERROR, _T( "[gui] Error loading font data: bad fileformat or unsupported version" ) );
    return false;
  }

  std::string s;
  do
  {
    s = m.ReadLine();

    if ( s.find( "info" ) == 0 )
      continue;

    if ( s.find( "common" ) == 0 )
    {
      if ( !ReadInt( s, "lineHeight", LineHeight ) )
        return false;
      if ( !ReadInt( s, "base", Base ) )
        return false;
      continue;
    }

    if ( s.find( "page" ) == 0 )
    {
      continue;
    }

    if ( s.find( "char" ) == 0 )
    {
      if ( s.find( "chars" ) == 0 )
        continue;

      WBSYMBOLINPUT r;
      int32_t v;

      if ( !ReadInt( s, "id", v ) )
        return false;
      r.Char = v;

      if ( !ReadInt( s, "xadvance", v ) )
        return false;
      r.Advance = v;

      if ( !ReadInt( s, "xoffset", v ) )
        return false;
      r.Offset.x = v;

      if ( !ReadInt( s, "yoffset", v ) )
        return false;
      r.Offset.y = v;

      int x, y, width, height;

      if ( !ReadInt( s, "x", x ) )
        return false;
      if ( !ReadInt( s, "y", y ) )
        return false;
      if ( !ReadInt( s, "width", width ) )
        return false;
      if ( !ReadInt( s, "height", height ) )
        return false;

      r.UV = CRect( x, y, x + width, y + height );

      if (enabledGlyphs.empty() ||
          std::find(enabledGlyphs.begin(), enabledGlyphs.end(), r.Char) !=
              enabledGlyphs.end()) {
        if (r.Char >= 0 && r.Char <= 0xffff) {
          Alphabet.push_back(r);
        }
      }

      continue;
    }

    if ( s.find( "kerning" ) == 0 )
    {
      if ( s.find( "kernings" ) == 0 )
        continue;

      WBKERNINGDATA d;
      int32_t v;

      if ( !ReadInt( s, "first", v ) )
        return false;
      d.First = v;

      if ( !ReadInt( s, "second", v ) )
        return false;
      d.Second = v;

      if ( !ReadInt( s, "amount", v ) )
        return false;
      d.Amount = v;

      KerningData.push_back(d);
      continue;
    }

  } while ( s.size() > 0 );

  return true;
}

CWBFont::CWBFont( CAtlas *atlas )
{
  Atlas = atlas;
  LineHeight = 0;
  Base = 0;
  Offset_X_Char = 0;
}

CWBFont::~CWBFont()
{
  for ( int32_t x = 0; x < AlphabetSize; x++ )
    if ( Alphabet[ x ].Char == x )
      Atlas->DeleteImage( Alphabet[ x ].Handle );
  SAFEDELETEA( Alphabet );
}

void CWBFont::AddSymbol( uint16_t Char, WBATLASHANDLE Handle, CSize &Size, const CPoint &Offset, int32_t Advance, CRect contentRect )
{
  if ( Char >= AlphabetSize )
    return;

  WBSYMBOL s;
  s.Char = Char;
  s.Handle = Handle;
  s.OffsetX = Offset.x;
  s.OffsetY = Offset.y;
  s.Advance = Advance;
  s.SizeX = Size.x;
  s.SizeY = Size.y;
  s.calculatedContentRect = contentRect;

  Alphabet[ Char ] = s;
  //Alphabet.Add(Char, s);

  if ( Char == 'x' )
  {
    Offset_X_Char = Offset.y + contentRect.y1;
    Height_X_Char = contentRect.x2 - contentRect.x1;
  }
}

uint32_t ReadUTF8Char(char const *&Text) {
  uint32_t Char = *Text;
  Text++;

  if ( (Char & 0x80) ) // decode utf-8
  {
    if ( (Char & 0xe0) == 0xc0 )
    {
      Char = Char & ( ( 1 << 5 ) - 1 );
      for ( int z = 0; z < 1; z++ )
      {
        if ( *( Text ) )
        {
          Char = ( Char << 6 ) + ( ( *( Text ) ) & 0x3f );
          Text++;
        }
      }
    }
    else
    if ( (Char & 0xf0) == 0xe0 )
    {
      Char = Char & ( ( 1 << 4 ) - 1 );
      for ( int z = 0; z < 2; z++ )
      {
        if ( *( Text ) )
        {
          Char = ( Char << 6 ) + ( ( *( Text ) ) & 0x3f );
          Text++;
        }
      }
    }
    else
    if ( (Char & 0xf8) == 0xf0 )
    {
      Char = Char & ( ( 1 << 3 ) - 1 );
      for ( int z = 0; z < 3; z++ )
      {
        if ( *( Text ) )
        {
          Char = ( Char << 6 ) + ( ( *( Text ) ) & 0x3f );
          Text++;
        }
      }
    }
    else
    if ( (Char & 0xfc) == 0xf8 )
    {
      Char = Char & ( ( 1 << 2 ) - 1 );
      for ( int z = 0; z < 4; z++ )
      {
        if ( *( Text ) )
        {
          Char = ( Char << 6 ) + ( ( *( Text ) ) & 0x3f );
          Text++;
        }
      }
    }
    else
    if ( (Char & 0xfe) == 0xfc )
    {
      Char = Char & ( ( 1 << 1 ) - 1 );
      for ( int z = 0; z < 5; z++ )
      {
        if ( *( Text ) )
        {
          Char = ( Char << 6 ) + ( ( *( Text ) ) & 0x3f );
          Text++;
        }
      }
    }
  }

  return Char;
}

int32_t CWBFont::WriteChar(CWBDrawAPI *DrawApi, int Char, int32_t x, int32_t y, CColor Color)
{
  if ( Char >= AlphabetSize || Alphabet[ (uint16_t)Char ].Char != Char ) //missing character replaced by a simple rectangle
  {
    if ( Alphabet[ (uint16_t)MissingChar ].Char != MissingChar )
    {
      LOG( LOG_WARNING, _T( "[font] Used character %d and fallback character %d also missing from font." ), Char, MissingChar );
      return 0;
    }

    int32_t width = GetWidth( MissingChar );

    if ( Char != ' ' )
    {
      WBSYMBOL &mc = Alphabet[ (uint16_t)MissingChar ];
      DrawApi->DrawRectBorder( CRect( x + mc.OffsetX, y + mc.OffsetY, x + mc.OffsetX + mc.SizeX, y + mc.OffsetY + mc.SizeY ), Color );
    }
    return width;
  }

  WBSYMBOL &Symbol = Alphabet[ (uint16_t)Char ];
  int32_t width = Symbol.Advance;
  if ( !width ) return 0;
  //DrawApi->DrawRect(CRect(x + Symbol.OffsetX, y + Symbol.OffsetY, x + Symbol.OffsetX + Symbol.SizeX, y + Symbol.OffsetY + Symbol.SizeY), 0x80808080);
  DrawApi->DrawAtlasElement( Symbol.Handle, x + Symbol.OffsetX, y + Symbol.OffsetY, Color );
  return width;
}

int32_t CWBFont::Write( CWBDrawAPI *DrawApi, std::string_view String, int32_t x, int32_t y, CColor Color, WBTEXTTRANSFORM Transform, TBOOL DoKerning )
{
  if ( String.empty() ) return 0;
  std::string t(String); // make sure we have a \0 at the correct place.
  auto Text = t.c_str();
  int32_t xp = x;
  int32_t yp = y;

  while ( *Text )
  {
    // Increments Text
    uint16_t Char = ApplyTextTransformUtf8( t.c_str(), Text, Transform );

    if ( Char == '\n' )
    {
      xp = x;
      yp += GetLineHeight();
      continue;
    }

    xp += WriteChar( DrawApi, Char, xp, yp, Color );

    if ( DoKerning && *Text && Kerning.NumItems() )
    {
      auto next = Text;
      uint16_t NextChar = ApplyTextTransformUtf8( t.c_str(), next, Transform );
      CWBKerningPair k = CWBKerningPair( Char, NextChar );
      if ( Kerning.HasKey( k ) )
        xp += Kerning[ k ];
    }
  }

  return xp - x;
}

int32_t CWBFont::WriteChar(CWBDrawAPI *DrawApi, int Char, CPoint &p, CColor Color)
{
  return WriteChar( DrawApi, Char, p.x, p.y, Color );
}

int32_t CWBFont::Write(CWBDrawAPI *DrawApi, std::string_view String, CPoint &p,
                       CColor Color, WBTEXTTRANSFORM Transform,
                       TBOOL DoKerning) {
  return Write( DrawApi, String, p.x, p.y, Color, Transform, DoKerning );
}

int32_t CWBFont::GetWidth( uint16_t Char, TBOOL Advance )
{
  if ( Char >= AlphabetSize || Alphabet[ (uint16_t)Char ].Char != Char ) //missing character
  {
    if ( Alphabet[ (uint16_t)MissingChar ].Char != MissingChar )
      return 0;
    return Alphabet[ (uint16_t)MissingChar ].Advance;
  }
  return Advance ? Alphabet[ (uint16_t)Char ].Advance : ( Alphabet[ (uint16_t)Char ].OffsetX + Alphabet[ (uint16_t)Char ].calculatedContentRect.x2 );
}

int32_t CWBFont::GetWidth(std::string_view String, TBOOL AdvanceLastChar,
                          WBTEXTTRANSFORM Transform, TBOOL DoKerning,
                          TBOOL firstCharHack) {
  if (String.empty()) return 0;
  std::string t(String);  // make sure we have a \0 at the correct place.
  auto Text = t.c_str();
  int32_t xp = 0;
  int32_t maxXp = 0;

  bool firstChar = true;

  while ( *Text )
  {
    uint16_t Char = ApplyTextTransformUtf8( t.c_str(), Text, Transform );

    if ( Char == '\n' )
    {
      maxXp = max( xp, maxXp );
      xp = 0;
      continue;
    }

    if ( firstCharHack && firstChar && !AdvanceLastChar )
    {
      if ( Char >= AlphabetSize || Alphabet[ (uint16_t)Char ].Char != Char )
        xp -= Alphabet[ (uint16_t)MissingChar ].calculatedContentRect.x1;
      else
        xp -= Alphabet[ (uint16_t)Char ].calculatedContentRect.x1;
    }

    if ( AdvanceLastChar )
      xp += GetWidth( Char );
    else
    {
      if ( *Text )
        xp += GetWidth( Char );
      else
        xp += GetWidth( Char, false );
    }

    if ( DoKerning && *Text && Kerning.NumItems() )
    {
      auto next = Text;
      uint16_t NextChar = ApplyTextTransformUtf8( t.c_str(), next, Transform );
      CWBKerningPair k = CWBKerningPair( Char, NextChar );
      if ( Kerning.HasKey( k ) )
        xp += Kerning[ k ];
    }
  }

  return max( maxXp, xp );
}

void CWBFont::AddKerningPair( uint16_t First, uint16_t Second, int16_t Amount )
{
  Kerning[ CWBKerningPair( First, Second ) ] = Amount;
}

TBOOL CWBFont::Initialize( CWBFontDescription *Description, TCHAR mc )
{
  MissingChar = mc;
  if ( !Description ) return false;
  if ( !Atlas ) return false;
  if ( !Description->Image ) return false;

  AlphabetSize = 0;
  for ( const auto& abc:Description->Alphabet )
    AlphabetSize = max( AlphabetSize, abc.Char ) + 1;

  Alphabet = new WBSYMBOL[ AlphabetSize ];
  memset( Alphabet, 0, AlphabetSize * sizeof( WBSYMBOL ) );
  for ( int32_t x = 0; x < AlphabetSize; x++ )
    Alphabet[ x ].Char = (int16_t)( x - 1 );

  for ( const auto& abc: Description->Alphabet )
    if ( abc.UV.Area() > 0 )
    {
      WBATLASHANDLE h = Atlas->AddImage( Description->Image, Description->XRes, Description->YRes, abc.UV );

      if ( !h )
      {
        LOG( LOG_ERROR, _T( "[gui] Atlas Error while creating font!" ) );
        return false;
      };

      CRect content = CRect( abc.UV.x2, abc.UV.y2, abc.UV.x1, abc.UV.y1 );
      bool hadContent = false;

      for ( int j = abc.UV.y1; j < abc.UV.y2; j++ )
        for ( int i = abc.UV.x1; i < abc.UV.x2; i++ )
        {
          uint8_t* c = Description->Image + ( j*Description->XRes + i ) * 4;

          if ( c[ 3 ] > 10 )
          {
            hadContent = true;
            content.x1 = min( content.x1, i );
            content.x2 = max( content.x2, i );
            content.y1 = min( content.y1, j );
            content.y2 = max( content.y2, j );
          }
        }

      if ( hadContent )
        content -= abc.UV.TopLeft();
      else
        content = CRect( 0, 0, abc.UV.Width(), abc.UV.Height() );

      AddSymbol( abc.Char, h, abc.UV.Size(), abc.Offset, abc.Advance, content );
    }

  for ( const auto& kd:Description->KerningData )
  {
    AddKerningPair( kd.First, kd.Second, kd.Amount );
  }

  LineHeight = Description->LineHeight;
  Base = Description->Base;

  return true;
}

int32_t CWBFont::GetLineHeight()
{
  return LineHeight;
}

int32_t CWBFont::GetBase()
{
  return Base;
}

int32_t CWBFont::GetOffsetX( TCHAR Char )
{
  if ( Char >= AlphabetSize || Alphabet[ (uint16_t)Char ].Char != Char ) Char = MissingChar;
  if ( Char >= AlphabetSize || Alphabet[ (uint16_t)Char ].Char != Char ) return 0;

  return Alphabet[ (uint16_t)Char ].OffsetX;
}

int32_t CWBFont::GetOffsetY( TCHAR Char )
{
  if ( Char >= AlphabetSize || Alphabet[ (uint16_t)Char ].Char != Char ) Char = MissingChar;
  if ( Char >= AlphabetSize || Alphabet[ (uint16_t)Char ].Char != Char ) return 0;

  return Alphabet[ (uint16_t)Char ].OffsetY;
}

int32_t CWBFont::GetCenterWidth(int32_t x1, int32_t x2, std::string_view Text,
                                WBTEXTTRANSFORM Transform) {
  return ( x1 + x2 - GetWidth( Text, false, Transform, true ) ) / 2;// -GetOffsetX(Char);
}

int32_t CWBFont::GetCenterHeight( int32_t y1, int32_t y2 )
{
  return y1 + ( y2 - y1 ) / 2 - GetMedian();
}

CPoint CWBFont::GetCenter(std::string_view Text, CRect Rect,
                          WBTEXTTRANSFORM Transform) {
  return CPoint( GetCenterWidth( Rect.x1, Rect.x2, Text, Transform ), GetCenterHeight( Rect.y1, Rect.y2 ) );
}

int32_t CWBFont::GetMedian()
{
  return Offset_X_Char + Height_X_Char / 2;
}

void CWBFont::ConvertToUppercase()
{
  for ( int32_t x = 0; x < AlphabetSize; x++ )
    if ( Alphabet[ x ].Char == x )
    {
      WBSYMBOL &c = Alphabet[ x ];
      if ( c.Char != towupper( c.Char ) )
      {
        if ( Alphabet[ towupper( c.Char ) ].Char == towupper( c.Char ) )
        {
          WBSYMBOL &C = Alphabet[ towupper( c.Char ) ];

          c.Advance = C.Advance;
          Atlas->DeleteImage( c.Handle );
          c.Handle = C.Handle;
          c.OffsetX = C.OffsetX;
          c.OffsetY = C.OffsetY;
          c.SizeX = C.SizeX;
          c.SizeY = C.SizeY;
        }
      }
    }
}

INLINE char CWBFont::ApplyTextTransform( const char *Text, const char *CurrPos, WBTEXTTRANSFORM Transform )
{
  if ( Transform <= 0 ) return *CurrPos;
  switch ( Transform )
  {
  case WBTT_NONE:
    return *CurrPos;
    break;
  case WBTT_CAPITALIZE:
    if ( Text == CurrPos || ( CurrPos > Text && _istspace( *( CurrPos - 1 ) ) ) ) return _totupper( *CurrPos );
    return _totlower( *CurrPos );
    break;
  case WBTT_UPPERCASE:
    return _totupper( *CurrPos );
    break;
  case WBTT_LOWERCASE:
    return _totlower( *CurrPos );
    break;
  default:
    break;
  }
  return 0;
}

// Also increments CurrPos.
INLINE uint16_t CWBFont::ApplyTextTransformUtf8(const char *Text,
                                                char const *&CurrPos,
                                                WBTEXTTRANSFORM Transform) {
  uint32_t decoded = ReadUTF8Char( CurrPos );

  if ( Transform <= 0 ) 
    return decoded;

  switch ( Transform )
  {
  case WBTT_NONE:
    return decoded;
    break;
  case WBTT_CAPITALIZE:
    if ( Text == CurrPos || ( CurrPos > Text && _istspace( *( CurrPos - 1 ) ) ) ) return _totupper( decoded );
    return _totlower( decoded );
    break;
  case WBTT_UPPERCASE:
    return _totupper( decoded );
    break;
  case WBTT_LOWERCASE:
    return _totlower( decoded );
    break;
  default:
    break;
  }
  return 0;
}

int32_t CWBFont::GetHeight( uint16_t Char )
{
  return GetLineHeight();
}

int32_t CWBFont::GetHeight(std::string_view String) {
  int32_t lineCount = 1;
  for (const auto& c: String ) {
    if ( c == '\n' )
      lineCount++;
  }

  return LineHeight * lineCount;
}

CPoint CWBFont::GetTextPosition(std::string_view String, CRect &Container,
                                WBTEXTALIGNMENTX XAlign,
                                WBTEXTALIGNMENTY YAlign,
                                WBTEXTTRANSFORM Transform /*= WBTT_NONE*/,
                                TBOOL DoKerning /*= true*/) {
  CPoint p = Container.TopLeft();

  int32_t Width = GetWidth( String, false, Transform, DoKerning );
  int32_t Height = GetLineHeight();

  if ( XAlign == WBTA_CENTERX ) p.x = GetCenterWidth( Container.x1, Container.x2, String, Transform );
  if ( XAlign == WBTA_RIGHT ) p.x = Container.x2 - Width;
  if ( YAlign == WBTA_CENTERY ) p.y = GetCenterHeight( Container.y1, Container.y2 );
  if ( YAlign == WBTA_BOTTOM ) p.y = Container.y2 - Height;

  return p;
}

