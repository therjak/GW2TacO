#include "TrailLogger.h"
#include "OverlayConfig.h"
#include <commdlg.h>

#define TRAILFILEVERSION 0

extern float globalOpacity;
extern float minimapOpacity;


CDictionaryEnumerable<GUID, GW2Trail*> trails;

extern CWBApplication *App;
CStreamWriterFile* TrailLog = nullptr;

int32_t lastMap = -1;
CVector3 lastPos = CVector3( 0, 0, 0 );

float WorldToGameCoords( float world );
float GameToWorldCoords( float game );
float GetMapFade();

void GlobalDoTrailLogging( int32_t mapID, CVector3 charPos )
{
  GW2TrailDisplay* trails = (GW2TrailDisplay*)App->GetRoot()->FindChildByID( _T( "trail" ), _T( "gw2Trails" ) );
  if ( trails )
    trails->DoTrailLogging( mapID, charPos );
}

void GW2TrailDisplay::DrawProxy( CWBDrawAPI *API, bool miniMaprender )
{
  int32_t fadeoutBubble = GetConfigValue( "FadeoutBubble" );

  drawrect = GetClientRect();

  cam.SetLookAtLH( mumbleLink.camPosition, mumbleLink.camPosition + mumbleLink.camDir, CVector3( 0, 1, 0 ) );
  persp.SetPerspectiveFovLH( mumbleLink.fov, drawrect.Width() / (float)drawrect.Height(), 0.01f, 150.0f );
  asp = drawrect.Width() / (float)drawrect.Height();

  //CMatrix4x4 m = cam*persp;

  API->FlushDrawBuffer();

  App->GetDevice()->SetVertexShader( vxShader );
  App->GetDevice()->SetPixelShader( pxShader );
  App->GetDevice()->SetVertexFormat( vertexFormat );
  trailSampler->Apply( CORESMP_PS0 );
  trailDepthStencil->Apply();

  if ( !HasConfigValue( "ShowMinimapTrails" ) )
    SetConfigValue( "ShowMinimapTrails", 1 );
  int showMinimapTrails = GetConfigValue( "ShowMinimapTrails" );

  if ( !HasConfigValue( "ShowBigmapTrails" ) )
    SetConfigValue( "ShowBigmapTrails", 1 );
  int showBigmapTrails = GetConfigValue( "ShowBigmapTrails" );

  if ( !HasConfigValue( "ShowInGameTrails" ) )
    SetConfigValue( "ShowInGameTrails", 1 );
  int showIngameTrails = GetConfigValue( "ShowInGameTrails" );

  float one = 1;
  float data[ 8 ];

  if (!miniMaprender && showIngameTrails>0)
  for ( int x = 0; x < 2; x++ )
  {
    if ( x == 0 )
      trailRasterizer2->Apply();
    else
      trailRasterizer1->Apply();

    CLightweightCriticalSection cs( &critsec );

    for ( int32_t y = 0; y < trails.NumItems(); y++ )
    {
      auto& trail = *trails.GetByIndex( y );
      if ( !trail.typeData.bits.inGameVisible && showIngameTrails != 2 )
        continue;

      auto& str = GetStringFromMap( trail.typeData.texture );
      CCoreTexture* texture = nullptr;

      if ( str.Length() )
        texture = GetTexture( str, trail.zipFile, trail.category ? GetStringFromMap(trail.category->zipFile) : "");
      else
        texture = trailTexture;

      float width = GameToWorldCoords( 20 );

      trail.SetupAndDraw( constBuffer, texture, cam, persp, one, x == 0, fadeoutBubble, data, GetMapFade() * globalOpacity, width, width, 1.0f );
    }

    if ( editedTrail )
      if ( editedTrail->map == mumbleLink.mapID )
      {
        data[ 0 ] = GetTime() / 1000.0f;

        App->GetDevice()->SetTexture( CORESMP_PS0, trailTexture );

        constBuffer->Reset();
        constBuffer->AddData( cam, 16 * 4 );
        constBuffer->AddData( persp, 16 * 4 );
        constBuffer->AddData( &mumbleLink.charPosition, 12 );
        constBuffer->AddData( &one, 4 );
        constBuffer->AddData( data, 16 );
        //color

        data[ 0 ] = 0.2f;
        data[ 1 ] = 0.7f;
        data[ 2 ] = 1.0f;
        data[ 3 ] = 0.8f;

        if ( x == 0 )
        {
          data[ 0 ] *= 0.5f;
          data[ 1 ] *= 0.5f;
          data[ 2 ] *= 0.5f;
        }

        constBuffer->AddData( data, 16 );
        data[ 0 ] = 1000;
        data[ 1 ] = 1200;
        data[ 2 ] = float( fadeoutBubble );
        data[ 3 ] = GameToWorldCoords( 20 );
        data[ 4 ] = GameToWorldCoords( 20 );
        data[ 5 ] = 1.0f;
        constBuffer->AddData( data, 32 );

        constBuffer->Upload();
        App->GetDevice()->SetShaderConstants( 0, 1, &constBuffer );

        editedTrail->Draw();
      }
  }

  // draw minimap
  if ( miniMaprender )
  {
    trailRasterizer3->Apply();
    CRect miniRect = GetMinimapRectangle();
    CRect clientRect = GetClientRect();

    float mapFade = GetMapFade();

    if ( mapFade > 0 && showMinimapTrails > 0 )
    {
      CMatrix4x4 camera = mumbleLink.miniMap.BuildTransformationMatrix( miniRect, false );
      //camera *= CMatrix4x4().Scaling( CVector3( 2.0f / clientRect.Width(), -2.0f / clientRect.Height(), 0.0f ) );
      //camera *= CMatrix4x4().Translation( CVector3( -1.0f, -1.0f, 0.5 ) );

      camera *= CMatrix4x4().Translation( -CVector3( float( miniRect.x1 ), float( miniRect.y1 ), 0 ) );
      camera *= CMatrix4x4().Scaling( CVector3( clientRect.Width() / float( miniRect.Width() ), clientRect.Height() / float( miniRect.Height() ), 0 ) );
      camera *= CMatrix4x4().Scaling( CVector3( 2.0f / clientRect.Width(), -2.0f / clientRect.Height(), 0.0f ) );
      camera *= CMatrix4x4().Translation( CVector3( -1.0f, 1.0f, 0.5 ) );
      CMatrix4x4 perspective;
      perspective.SetIdentity();

      API->SetRenderView( miniRect );

      for ( int32_t y = 0; y < trails.NumItems(); y++ )
      {
        auto& trail = *trails.GetByIndex( y );
        if ( !trail.typeData.bits.miniMapVisible && showMinimapTrails != 2 )
          continue;

        float trailWidth = trail.typeData.miniMapSize*0.5f;
        if ( trail.typeData.bits.scaleWithZoom )
          trailWidth /= mumbleLink.miniMap.mapScale;

        auto& str = GetStringFromMap( trail.typeData.texture );
        CCoreTexture* texture = nullptr;

        if ( str.Length() )
          texture = GetTexture( str, trail.zipFile, trail.category ? GetStringFromMap(trail.category->zipFile) : "");
        else
          texture = trailTexture;

        float alpha = 1.0f - max( 0.0f, min( 1.0f, ( mumbleLink.miniMap.mapScale - trail.typeData.miniMapFadeOutLevel ) / 2.0f ) );

        trail.SetupAndDraw( constBuffer, texture, camera, perspective, one, false, 0, data, mapFade * alpha * minimapOpacity, 1.0f, GameToWorldCoords( 20 ) * 0.1f, trailWidth );
      }
    }

    if ( mumbleLink.isMapOpen && mapFade < 1.0 && showBigmapTrails > 0 )
    {
      miniRect = GetClientRect();
      CMatrix4x4 camera = mumbleLink.bigMap.BuildTransformationMatrix( miniRect, true );

      camera *= CMatrix4x4().Scaling( CVector3( 2.0f / clientRect.Width(), -2.0f / clientRect.Height(), 0.0f ) );
      camera *= CMatrix4x4().Translation( CVector3( -1.0f, 1.0f, 0.5 ) );
      //camera *= CMatrix4x4().Scaling( CVector3( 10, 10, 1 ) );
      CMatrix4x4 perspective;
      perspective.SetIdentity();

      API->SetRenderView( miniRect );

      for ( int32_t y = 0; y < trails.NumItems(); y++ )
      {
        auto& trail = *trails.GetByIndex( y );
        if ( !trail.typeData.bits.bigMapVisible && showBigmapTrails != 2 )
          continue;

        float trailWidth = trail.typeData.miniMapSize*0.5f;
        if ( trail.typeData.bits.scaleWithZoom )
          trailWidth /= mumbleLink.miniMap.mapScale;

        auto& str = GetStringFromMap( trail.typeData.texture );
        CCoreTexture* texture = nullptr;

        if ( str.Length() )
          texture = GetTexture( str, trail.zipFile, trail.category ? GetStringFromMap(trail.category->zipFile) : "");
        else
          texture = trailTexture;

        float alpha = 1.0f - max( 0.0f, min( 1.0f, ( mumbleLink.bigMap.mapScale - trail.typeData.miniMapFadeOutLevel ) / 2.0f ) );
        trail.SetupAndDraw( constBuffer, texture, camera, perspective, one, false, 0, data, ( 1.0f - mapFade ) * alpha * minimapOpacity, 1.0f, GameToWorldCoords( 20 ) * 0.1f, trailWidth );
      }
    }

    API->SetRenderView( App->GetRoot()->GetWindowRect() );
  }

  API->SetUIRenderState();
}

void GW2TrailDisplay::OnDraw( CWBDrawAPI *API )
{
  if ( !HasConfigValue( "TrailLayerVisible" ) )
    SetConfigValue( "TrailLayerVisible", 1 );

  if ( !GetConfigValue( "TrailLayerVisible" ) )
    return;

  if ( !HasConfigValue( "FadeoutBubble" ) )
    SetConfigValue( "FadeoutBubble", 1 );

  if ( !HasConfigValue( "TacticalLayerVisible" ) )
    SetConfigValue( "TacticalLayerVisible", 1 );

  if ( !GetConfigValue( "TacticalLayerVisible" ) )
    return;

  if ( !mumbleLink.IsValid() )
    return;

  DrawProxy( API, false );

  if ( GetConfigValue( "LogTrails" ) )
  {
    CWBFont *f = GetFont( GetState() );
    int32_t ypos = Lerp( GetClientRect().y1, GetClientRect().y2, 0.25f );

    CString s = "TacO is logging your trail.";

    CPoint pos = f->GetTextPosition( s, CRect( GetClientRect().x1, ypos, GetClientRect().x2, ypos ), WBTA_CENTERX, WBTA_CENTERY, WBTT_NONE, true );
    ypos += f->GetLineHeight();
    f->Write( API, s, pos, 0xffff0000 );
  }
}

void GW2TrailDisplay::DoTrailLogging( int32_t mapID, CVector3 charPos )
{
  CLightweightCriticalSection cs( &critsec );

  if ( !trailBeingRecorded )
    return;

  if ( trailRecordPaused )
    return;

  if ( mapID != lastMap )
    ClearEditedTrail();

  if ( !editedTrail )
  {
    editedTrail = new GW2Trail();
    editedTrail->Reset( mapID );
  }

  float dist = WorldToGameCoords( ( lastPos - charPos ).Length() );
  if ( dist < 30 )
    return;

  lastMap = mapID;
  editedTrail->positions.push_back(charPos);
  lastPos = charPos;

  editedTrail->Update();
}

void GW2TrailDisplay::ClearEditedTrail()
{
  SAFEDELETE( editedTrail );
}

#define MINIZ_HEADER_FILE_ONLY
#include "Bedrock/UtilLib/miniz.c"

mz_zip_archive* OpenZipFile( const CString& zipFile );

CCoreTexture2D* GW2TrailDisplay::GetTexture( const CString& fname, const CString& zipFile, const CString& categoryZip )
{
  CString s = ( zipFile.Length() ? ( zipFile + "\\" ) : "" ) + fname;
  s.ToLower();

  if ( textureCache.HasKey( s ) )
  {
    if ( textureCache[ s ] )
      return textureCache[ s ];
    return trailTexture;
  }

  if (zipFile.Length() || categoryZip.Length())
  {
    // we didn't find an entry from within the zip file, try to load it

    for (int x = 0; x < 2; x++)
    {
      if (!zipFile.Length() && x == 0)
        continue;

      if (!categoryZip.Length() && x == 1)
        continue;

      mz_zip_archive* zip = x == 0 ? OpenZipFile(zipFile) : OpenZipFile(categoryZip);

      if (zip)
      {
        int idx = mz_zip_reader_locate_file(zip, fname.GetPointer(), nullptr, 0);
        if (idx >= 0 && !mz_zip_reader_is_file_a_directory(zip, idx))
        {
          mz_zip_archive_file_stat stat;
          if (mz_zip_reader_file_stat(zip, idx, &stat) && stat.m_uncomp_size > 0)
          {
            uint8_t* data = new uint8_t[(int32_t)stat.m_uncomp_size];

            if (mz_zip_reader_extract_to_mem(zip, idx, data, (int32_t)stat.m_uncomp_size, 0))
            {
              CCoreTexture2D* tex = App->GetDevice()->CreateTexture2D(data, (int32_t)stat.m_uncomp_size);
              if (tex)
              {
                textureCache[s] = tex;
                delete[] data;
                return tex;
              }
              else
                LOG_ERR("[GW2TacO] Failed to decompress image %s from archive %s", fname.GetPointer(), x == 0 ? zipFile.GetPointer() : categoryZip.GetPointer());
            }
            delete[] data;
          }
        }
      }
    }

    // zipfile load failed, fall back to regular load and add it as an alias
    auto texture = GetTexture( fname, "", "" );
    textureCache[ s ] = texture;
    return texture;
  }

  CStreamReaderMemory f;
  if ( !f.Open( s.GetPointer() ) && !f.Open( ( CString( "POIs\\" ) + s ).GetPointer() ) )
  {
    textureCache[ s ] = nullptr;
    LOG_ERR("[GW2TacO] Failed to open image %s", s.GetPointer());
    return trailTexture;
  }

  auto texture = App->GetDevice()->CreateTexture2D(f.GetData(), int32_t(f.GetLength()));
  textureCache[s] = texture;
  if (!texture)
    LOG_ERR("[GW2TacO] Failed to decompress image %s", s.GetPointer());
  return textureCache[ s ];
}

GW2TrailDisplay::GW2TrailDisplay( CWBItem *Parent, CRect Position ) : CWBItem( Parent, Position )
{
  constBuffer = App->GetDevice()->CreateConstantBuffer();

  CStreamReaderMemory tex;
  if ( tex.Open( "Data\\trail.png" ) )
  {
    trailTexture = App->GetDevice()->CreateTexture2D( tex.GetData(), int32_t( tex.GetLength() ) );
    if (!trailTexture)
      LOG_ERR("[GW2TacO] Failed to decompress trail texture image!");
  }
  else
    LOG_ERR( "[GW2TacO] Failed to open trail texture!" );
  
  App->GetDevice()->SetShaderConstants( 0, 1, &constBuffer );
  trailSampler = App->GetDevice()->CreateSamplerState();
  trailSampler->SetAddressU( CORETEXADDRESS_WRAP );
  trailSampler->SetAddressV( CORETEXADDRESS_WRAP );
  trailSampler->SetFilter( COREFILTER_ANISOTROPIC );
  trailSampler->Update();

  trailRasterizer1 = App->GetDevice()->CreateRasterizerState();
  trailRasterizer1->SetCullMode( CORECULL_CCW );
  trailRasterizer1->Update();

  trailRasterizer2 = App->GetDevice()->CreateRasterizerState();
  trailRasterizer2->SetCullMode( CORECULL_CW );
  trailRasterizer2->Update();

  trailRasterizer3 = App->GetDevice()->CreateRasterizerState();
  trailRasterizer3->SetCullMode( CORECULL_NONE );
  trailRasterizer3->Update();

  trailDepthStencil = App->GetDevice()->CreateDepthStencilState();
  trailDepthStencil->SetDepthEnable( false );
  trailDepthStencil->Update();

  LPCSTR code =
    "Texture2D GuiTexture:register(t0);"
    "SamplerState Sampler:register(s0);"
    "cbuffer resdata : register(b0)"
    "{							   "
    "		float4x4 camera;	   "
    "		float4x4 persp;   "
    "   float4   charpos;"
    "   float4   data;"
    "   float4   color;"
    "   float2   nearFarFades;"
    "   float    fadeOutBubble;"
    "   float    width;"
    "   float    uvScale;"
    "   float    width2d;"
    "}"
    "float rayspheredepth(float3 o, float3 d)"
    "{"
    "  if (fadeOutBubble==0) return 1;"
    "  float4 c = mul(camera,charpos + float4(0,1,0,0));"
    "  c/=c.w;"
    "  float r=2.00;"
    "  float3 oc=o-c.xyz;"
    "  float discriminant = dot(oc, d) * dot(oc, d) - ( dot( oc, oc ) - r * r ); "
    "  if (discriminant < 0) return 1;"
    "  float t1 = -dot(d, oc) - sqrt(discriminant);"
    "  float t2 = -dot(d, oc) + sqrt(discriminant);"
    "  return saturate(1-abs(t2-t1)/r/2.0);"
    "}"
    "struct VSIN { float4 Position : POSITIONT; float2 UV : TEXCOORD0; float4 Pos2 : TEXCOORD1; float4 Color : COLOR0; };"
    "struct VSOUT { float4 Position : SV_POSITION; float2 UV : TEXCOORD0; float4 Color : COLOR0; float4 p : TEXCOORD1; };"
    "VSOUT vsmain(VSIN x) { VSOUT k; k.p=k.Position=mul(camera,(x.Position-x.Pos2)*width+x.Pos2); k.p/=k.p.w; float4 p2 = mul(camera,x.Pos2); p2/=p2.w; k.Position=mul(persp,(k.Position-p2)*width2d+p2); k.UV=float2(x.UV.x,x.UV.y*uvScale); k.Color=x.Color; k.p/=k.p.w; return k; }"
    "float4 psmain(VSOUT x) : SV_TARGET0 {  float farFade = saturate(1.0-(x.p.z-nearFarFades.x)/(nearFarFades.y-nearFarFades.x)); float a=rayspheredepth(0,normalize(x.p.xyz)); a*=farFade; return x.Color*GuiTexture.Sample(Sampler,x.UV + float2(0,data.x))*color*float4(1,1,1,a); }";

  vxShader = App->GetDevice()->CreateVertexShader( code, (int32_t)strlen( code ), "vsmain", "vs_4_0" );
  pxShader = App->GetDevice()->CreatePixelShader( code, (int32_t)strlen( code ), "psmain", "ps_4_0" );

  COREVERTEXATTRIBUTE TrailVertexFormat[] =
  {
    COREVXATTR_POSITIONT4,
    COREVXATTR_TEXCOORD2,
    COREVXATTR_TEXCOORD4,
    COREVXATTR_COLOR4,

    COREVXATTR_STOP,
  };

  COREVERTEXATTRIBUTE *vx = TrailVertexFormat;
  CArray<COREVERTEXATTRIBUTE> Att;
  while ( *vx != COREVXATTR_STOP ) Att += *vx++;
    
  vertexFormat = App->GetDevice()->CreateVertexFormat( Att, vxShader );
  if ( !vertexFormat )
  {
    LOG( LOG_ERROR, _T( "[GW2TacO]  Error creating Trail Vertex Format" ) );
  }


  //GW2Trail* trail = new GW2Trail();
  //trail->Build( App->GetDevice(), *(int32_t*)f.GetData(), (float*)( f.GetData() + 4 ), ( f.GetLength() - 4 ) / 12 );

  //trails += trail;
}
GW2TrailDisplay::~GW2TrailDisplay()
{
  textureCache.FreeAllA();

  //SAFEDELETE( vertexFormat ); // this crashes too...
  SAFEDELETE( pxShader );
  SAFEDELETE( vxShader );
  SAFEDELETE( constBuffer );
  SAFEDELETE( trailTexture );
  //SAFEDELETE( trailSampler ); // this crashed every time for some guy...
  SAFEDELETE( trailRasterizer1 );
  SAFEDELETE( trailRasterizer2 );
  SAFEDELETE( trailRasterizer3 );
  SAFEDELETE( trailDepthStencil );
}

CWBItem *GW2TrailDisplay::Factory( CWBItem *Root, CXMLNode &node, CRect &Pos )
{
  return new GW2TrailDisplay( Root, Pos );
}

TBOOL GW2TrailDisplay::IsMouseTransparent( CPoint &ClientSpacePoint, WBMESSAGE MessageType )
{
  return true;
}

void GW2TrailDisplay::StartStopTrailRecording( TBOOL start )
{
  trailBeingRecorded = start;
  if ( !trailBeingRecorded )
    ClearEditedTrail();
}

void GW2TrailDisplay::PauseTrail( TBOOL pause, TBOOL newSection )
{
  trailRecordPaused = pause;

  CWBButton* btn = App->GetRoot()->FindChildByID< CWBButton >( "pausetrail" );
  if ( btn )
  {
    btn->Push( pause );
    btn->SetText( btn->IsPushed() ? "Resume Recording" : "Pause Recording" );
  }

  btn = App->GetRoot()->FindChildByID< CWBButton >( "startnewsection" );
  if ( btn )
    btn->Hide( !pause );

  if ( !pause && newSection && editedTrail )
    editedTrail->positions.emplace_back( CVector3( 0, 0, 0 ) );
}

void GW2TrailDisplay::DeleteLastTrailSegment()
{
  if ( !editedTrail )
    return;

  if ( !editedTrail->positions.empty() )
    editedTrail->positions.pop_back();

  editedTrail->Update();
}

void GW2TrailDisplay::DeleteTrailSegment()
{

}

extern bool disableHooks;


void GW2TrailDisplay::ExportTrail()
{
  if ( !editedTrail )
    return;

  disableHooks = true;

  TCHAR dir[ 1024 ];
  if ( !GetCurrentDirectory( 1024, dir ) )
    memset( dir, 0, sizeof( TCHAR ) * 1024 );
  char Filestring[ 256 ];

  OPENFILENAME opf;
  opf.hwndOwner = (HWND)App->GetHandle();
  opf.lpstrFilter = "GW2 Taco Trail Files\0*.trl\0\0";
  opf.lpstrCustomFilter = 0;
  opf.nMaxCustFilter = 0L;
  opf.nFilterIndex = 1L;
  opf.lpstrFile = Filestring;
  opf.lpstrFile[ 0 ] = '\0';
  opf.nMaxFile = 256;
  opf.lpstrFileTitle = 0;
  opf.nMaxFileTitle = 50;
  opf.lpstrInitialDir = "Data";
  opf.lpstrTitle = "Save Trail";
  opf.nFileOffset = 0;
  opf.nFileExtension = 0;
  opf.lpstrDefExt = "trl";
  opf.lpfnHook = NULL;
  opf.lCustData = 0;
  opf.Flags = ( OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_NONETWORKBUTTON ) & ~OFN_ALLOWMULTISELECT;
  opf.lStructSize = sizeof( OPENFILENAME );

  opf.hInstance = GetModuleHandle( 0 );
  opf.pvReserved = NULL;
  opf.dwReserved = 0;
  opf.FlagsEx = 0;

  opf.lpstrInitialDir = dir;

  if ( GetSaveFileName( &opf ) )
  {
    editedTrail->SaveToFile( CString( opf.lpstrFile ) );
  }
  else
  {
    DWORD error = CommDlgExtendedError();
  }

  SetCurrentDirectory( dir );

  disableHooks = false;
}

void GW2TrailDisplay::ImportTrail()
{
  disableHooks = true;

  TCHAR dir[ 1024 ];
  if ( !GetCurrentDirectory( 1024, dir ) )
    memset( dir, 0, sizeof( TCHAR ) * 1024 );
  char Filestring[ 256 ];

  OPENFILENAME opf;
  opf.hwndOwner = (HWND)App->GetHandle();
  opf.lpstrFilter = "GW2 Taco Trail Files\0*.trl\0\0";
  opf.lpstrCustomFilter = 0;
  opf.nMaxCustFilter = 0L;
  opf.nFilterIndex = 1L;
  opf.lpstrFile = Filestring;
  opf.lpstrFile[ 0 ] = '\0';
  opf.nMaxFile = 256;
  opf.lpstrFileTitle = 0;
  opf.nMaxFileTitle = 50;
  opf.lpstrInitialDir = "Data";
  opf.lpstrTitle = "Import Trail";
  opf.nFileOffset = 0;
  opf.nFileExtension = 0;
  opf.lpstrDefExt = "trl";
  opf.lpfnHook = NULL;
  opf.lCustData = 0;
  opf.Flags = ( OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_NONETWORKBUTTON ) & ~OFN_ALLOWMULTISELECT;
  opf.lStructSize = sizeof( OPENFILENAME );

  opf.hInstance = GetModuleHandle( 0 );
  opf.pvReserved = NULL;
  opf.dwReserved = 0;
  opf.FlagsEx = 0;

  if ( GetOpenFileName( &opf ) )
  {
    CStreamReaderMemory file;
    if ( file.Open( opf.lpstrFile ) )
    {
      StartStopTrailRecording( true );
      ClearEditedTrail();
      PauseTrail( true );

      CWBButton* btn = App->GetRoot()->FindChildByID< CWBButton >( "starttrail" );
      if ( btn )
        btn->Push( true );
      btn = App->GetRoot()->FindChildByID< CWBButton >( "pausetrail" );
      if ( btn )
        btn->Push( true );

      if ( !editedTrail )
      {
        editedTrail = new GW2Trail();
        editedTrail->Import( file, true );
      }

      lastMap = mumbleLink.mapID;
      lastPos = mumbleLink.charPosition;
    }
  }

  SetCurrentDirectory( dir );

  disableHooks = false;
}

void GW2Trail::Reset( int32_t _mapID /*= 0 */ )
{
  map = _mapID;
  positions.clear();
}

TBOOL GW2Trail::SaveToFile( const CString& fname )
{
  if ( positions.empty() )
    return false;

  CStreamWriterFile TrailLog;
  if ( !TrailLog.Open( fname.GetPointer() ) )
    return false;

  TrailLog.WriteDWord( TRAILFILEVERSION );

  TrailLog.WriteDWord( map );
  TrailLog.Write( &positions[0], sizeof( CVector3 )*positions.size() );

  return true;
}

GW2Trail::~GW2Trail()
{
  SAFEDELETE( trailMesh );
  SAFEDELETE( idxBuf );
}

void GW2Trail::Build( CCoreDevice* d, int32_t mapID, float* points, int pointCount )
{
  dev = d;
  map = mapID;

  SAFEDELETE( trailMesh );
  SAFEDELETE( idxBuf );

  if ( pointCount <= 1 )
    return;

  GW2TrailVertex* vertices = new GW2TrailVertex[ pointCount * 2 ];
  memset( vertices, 0, sizeof( GW2TrailVertex )*pointCount * 2 );
  int vertexCount = 0;
  int32_t* indices = new int32_t[ ( pointCount - 1 ) * 6 ];

  float width = GameToWorldCoords( 20 );

  CVector3 lastPos = CVector3( points );
  CVector3 lastOrt = CVector3( 0, 0, 0 );

  float uvStretch = 0;

  int cnt = 0;
  int icnt = 0;

  float twist = 1;

  for ( int32_t x = 0; x < pointCount; x++ )
  {
    CVector3 pos = CVector3( points + x * 3 );

    if ( pos == CVector3( 0, 0, 0 ) )
    {
      if ( x + 1 >= pointCount )
        break;
      pos = lastPos = CVector3( points + ( x + 1 ) * 3 );
      twist = 1;
      lastOrt = CVector3( 0, 0, 0 );
    }

    CVector3 nextPos = CVector3( points + min( pointCount - 1, x + 1 ) * 3 );

    if ( nextPos == CVector3( 0, 0, 0 ) )
      nextPos = pos;

    uvStretch += ( pos - lastPos ).Length() * typeData.trailScale * 2;

    CVector3 dir = nextPos - lastPos;
    dir.y = 0;
    float dirLen = dir.Length();
    dir /= dirLen;
    CVector3 ort = CVector3::Cross( dir, CVector3( 0, 1, 0 ) ).Normalized();

    if ( lastOrt != CVector3( 0, 0, 0 ) && CVector3::Dot( ort, lastOrt ) < 0 )
      twist *= -1;

    CVector3 p1 = pos + ort * twist;
    CVector3 p2 = pos - ort * twist;

    vertices[ cnt * 2 ].Pos = CVector4( p1.x, p1.y, p1.z, 1 );
    vertices[ cnt * 2 + 1 ].Pos = CVector4( p2.x, p2.y, p2.z, 1 );
    vertices[ cnt * 2 ].CenterPos = CVector4( pos.x, pos.y, pos.z, 1 );
    vertices[ cnt * 2 + 1 ].CenterPos = CVector4( pos.x, pos.y, pos.z, 1 );
    vertices[ cnt * 2 ].Color = 0xffffffff;
    vertices[ cnt * 2 + 1 ].Color = 0xffffffff;
    vertices[ cnt * 2 ].UV = CVector2( 0, -uvStretch );
    vertices[ cnt * 2 + 1 ].UV = CVector2( 1, -uvStretch );

    if ( x < pointCount - 1 )
    {
      indices[ icnt++ ] = x * 2;
      if ( twist > 0 )
      {
        indices[ icnt++ ] = x * 2 + 2;
        indices[ icnt++ ] = x * 2 + 1;
      }
      else
      {
        indices[ icnt++ ] = x * 2 + 1;
        indices[ icnt++ ] = x * 2 + 2;
      }

      indices[ icnt++ ] = x * 2 + 2;
      if ( twist > 0 )
      {
        indices[ icnt++ ] = x * 2 + 3;
        indices[ icnt++ ] = x * 2 + 1;
      }
      else
      {
        indices[ icnt++ ] = x * 2 + 1;
        indices[ icnt++ ] = x * 2 + 3;
      }
    }

    cnt++;
    lastPos = pos;

    lastOrt = ort;
    vertexCount += 2;
  }

  trailMesh = dev->CreateVertexBuffer( (uint8_t*)vertices, vertexCount * sizeof( GW2TrailVertex ) );
  length = pointCount * 2;
  idxBuf = dev->CreateIndexBuffer( ( pointCount - 1 ) * 6, 4 );

  int32_t* idxData;

  if ( idxBuf->Lock( (void**)&idxData ) )
  {
    memcpy( idxData, indices, sizeof( int32_t ) * 6 * ( pointCount - 1 ) );
    //int cnt = 0;
    //for ( int32_t x = 0; x < pointCount - 1; x++ )
    //{
    //  idxData[ cnt++ ] = x * 2;
    //  idxData[ cnt++ ] = x * 2 + 2;
    //  idxData[ cnt++ ] = x * 2 + 1;
    //  idxData[ cnt++ ] = x * 2 + 2;
    //  idxData[ cnt++ ] = x * 2 + 3;
    //  idxData[ cnt++ ] = x * 2 + 1;
    //}
    idxBuf->UnLock();

  }

  SAFEDELETEA( indices );
  SAFEDELETEA( vertices );
}

void GW2Trail::Draw()
{
  if ( !trailMesh || !idxBuf )
    return;

  dev->SetVertexBuffer( trailMesh, 0 );
  dev->SetIndexBuffer( idxBuf );
  dev->DrawIndexedTriangles( length - 2, length );
}

void GW2Trail::Update()
{
  if ( !App->GetDevice() )
    return;

  Build( App->GetDevice(), map, (float*)&positions[0], positions.size() );
}

void GW2Trail::SetupAndDraw( CCoreConstantBuffer* constBuffer, CCoreTexture* texture, CMatrix4x4& cam, CMatrix4x4& persp, float& one, bool scaleData, int32_t fadeoutBubble, float* data, float fadeAlpha, float width, float uvScale, float width2d )
{
  if ( category && !category->IsVisible() )
    return;

  if ( map != mumbleLink.mapID )
    return;

  App->GetDevice()->SetTexture( CORESMP_PS0, texture );

  data[ 0 ] = GetTime() / 1000.0f;

  data[ 0 ] *= typeData.animSpeed;

  constBuffer->Reset();
  constBuffer->AddData( cam, 16 * 4 );
  constBuffer->AddData( persp, 16 * 4 );
  constBuffer->AddData( &mumbleLink.charPosition, 12 );
  constBuffer->AddData( &one, 4 );
  constBuffer->AddData( data, 16 );
  //color

  data[ 0 ] = typeData.color.R() / 255.0f;
  data[ 1 ] = typeData.color.G() / 255.0f;
  data[ 2 ] = typeData.color.B() / 255.0f;
  data[ 3 ] = typeData.alpha * fadeAlpha;

  if ( scaleData )
  {
    data[ 0 ] *= 0.5;
    data[ 1 ] *= 0.5;
    data[ 2 ] *= 0.5;
  }

  constBuffer->AddData( data, 16 );

  data[ 0 ] = GameToWorldCoords( typeData.fadeNear );
  data[ 1 ] = GameToWorldCoords( typeData.fadeFar );
  data[ 2 ] = float( fadeoutBubble );
  data[ 3 ] = width;
  data[ 4 ] = uvScale;
  data[ 5 ] = width2d;

  constBuffer->AddData( data, 32 );

  constBuffer->Upload();
  App->GetDevice()->SetShaderConstants( 0, 1, &constBuffer );

  Draw();
}

void GW2Trail::SetCategory( CWBApplication *App, GW2TacticalCategory *t )
{
  category = t;
  typeData = t->data;
  Type = t->GetFullTypeName();
}

TBOOL GW2Trail::Import( CStreamReaderMemory& f, TBOOL keepPoints )
{
  if ( keepPoints )
  {
    positions.clear();
    for ( int32_t x = 0; x < ( f.GetLength() - 8 ) / 12; x++ )
      positions.emplace_back( CVector3( &( (float*)( f.GetData() + 8 ) )[ x * 3 ] ) );
  }

  Build( App->GetDevice(), *(int32_t*)( f.GetData() + 4 ), (float*)( f.GetData() + 8 ), int32_t( ( f.GetLength() - 8 ) / 12 ) );
  return true;
}

TBOOL GW2Trail::Import( CString& fileName, const CString& zipFile, TBOOL keepPoints /*= false */ )
{
  if ( zipFile.Length() )
  {
    mz_zip_archive* zip = OpenZipFile( zipFile );

    if ( zip )
    {
      int idx = mz_zip_reader_locate_file( zip, fileName.GetPointer(), nullptr, 0 );
      if ( idx >= 0 && !mz_zip_reader_is_file_a_directory( zip, idx ) )
      {
        mz_zip_archive_file_stat stat;
        if ( mz_zip_reader_file_stat( zip, idx, &stat ) && stat.m_uncomp_size > 0 )
        {
          auto data = std::make_unique<uint8_t[]>((int32_t)stat.m_uncomp_size);

          if ( mz_zip_reader_extract_to_mem( zip, idx, data.get(), (int32_t)stat.m_uncomp_size, 0 ) )
          {
            CStreamReaderMemory f;
            if ( f.Open( data.get(), int32_t( stat.m_uncomp_size ) ) )
            {
              if ( Import( f, keepPoints ) )
                return true;
            }
          }
        }
      }
    }
  }

  CStreamReaderMemory f;
  if ( !f.Open( fileName.GetPointer() ) && !f.Open( ( CString( "POIs\\" ) + fileName ).GetPointer() ) )
  {
    LOG_ERR( "[GW2TacO] Failed to open trail data file %s", fileName.GetPointer() );
    return false;
  }

  return Import( f, keepPoints );
}
