#pragma once
#include "Bedrock/WhiteBoard/whiteboard.h"
#include <objbase.h>

enum class POIBehavior : int32_t
{
  AlwaysVisible,
  ReappearOnMapChange,
  ReappearOnDailyReset,
  OnlyVisibleBeforeActivation,
  ReappearAfterTimer,
  ReappearOnMapReset,
  OncePerInstance,
  DailyPerChar,
  OncePerInstancePerChar,
  WvWObjective,
};

struct MarkerTypeData
{
  struct 
  {
    TBOOL needsExportToUserData : 1;
    TBOOL iconFileSaved : 1;
    TBOOL sizeSaved : 1;
    TBOOL alphaSaved : 1;
    TBOOL fadeNearSaved : 1;
    TBOOL fadeFarSaved : 1;
    TBOOL heightSaved : 1;
    TBOOL behaviorSaved : 1;
    TBOOL resetLengthSaved : 1;
    TBOOL autoTriggerSaved : 1;
    TBOOL hasCountdownSaved : 1;
    TBOOL triggerRangeSaved : 1;
    TBOOL minSizeSaved : 1;
    TBOOL maxSizeSaved : 1;
    TBOOL colorSaved : 1;
    TBOOL trailDataSaved : 1;
    TBOOL animSpeedSaved : 1;
    TBOOL textureSaved : 1;
    TBOOL trailScaleSaved : 1;
    TBOOL toggleCategorySaved : 1;
    TBOOL achievementIdSaved : 1;
    TBOOL achievementBitSaved : 1;
    TBOOL autoTrigger : 1;
    TBOOL hasCountdown : 1;
    TBOOL miniMapVisible : 1;
    TBOOL bigMapVisible : 1;
    TBOOL inGameVisible : 1;
    TBOOL miniMapVisibleSaved : 1;
    TBOOL bigMapVisibleSaved : 1;
    TBOOL inGameVisibleSaved : 1;
    TBOOL scaleWithZoom : 1;
    TBOOL scaleWithZoomSaved : 1;
    TBOOL miniMapSizeSaved : 1;
    TBOOL miniMapFadeOutLevelSaved : 1;
    TBOOL keepOnMapEdge : 1;
    TBOOL keepOnMapEdgeSaved : 1;
    TBOOL infoSaved : 1;
    TBOOL infoRangeSaved : 1;
  } bits;

  MarkerTypeData();

  TF32 size = 1.0;
  TF32 alpha = 1.0f;
  TF32 fadeNear = -1;
  TF32 fadeFar = -1;
  TF32 height = 1.5f;
  TF32 triggerRange = 2.0f;
  TF32 animSpeed = 1;
  TF32 trailScale = 1;
  int32_t miniMapSize = 20;
  TF32 miniMapFadeOutLevel = 100.0f;
  TF32 infoRange = 2.0f;

  POIBehavior behavior = POIBehavior::AlwaysVisible;
  CColor color = CColor( 0xffffffff );

  int16_t resetLength = 0;
  int16_t minSize = 5;
  int16_t maxSize = 2048;

  int16_t iconFile = -1;
  int16_t trailData = -1;
  int16_t texture = -1;
  int16_t toggleCategory = -1;
  int16_t achievementId = -1;
  int16_t achievementBit = -1;
  int16_t info = -1;

  void Read( CXMLNode &n, TBOOL StoreSaveState );
  void Write( CXMLNode *n );
};

class GW2TacticalCategory;

struct Achievement
{
  bool done = false;
  CArray<int32_t> bits;
};

struct POI
{
  MarkerTypeData typeData;
  WBATLASHANDLE icon = 0;

  CVector4 cameraSpacePosition;

  CVector3 position;
  int32_t mapID;
  TU8 wvwObjectiveID;
  CString Type;

  time_t lastUpdateTime = 0;
  TBOOL External = false;
  TBOOL routeMember = false;

  int16_t zipFile;
  int16_t iconFile;

  GUID guid;

  GW2TacticalCategory *category = nullptr;
  void SetCategory( CWBApplication *App, GW2TacticalCategory *t );

  bool IsVisible( const tm& ptm, const time_t& currtime, bool achievementsFetched, CDictionary<int32_t, Achievement> &achievements, LIGHTWEIGHT_CRITICALSECTION &dataWriteCritSec );
};

struct POIActivationDataKey
{
  GUID guid;
  int uniqueData = 0;

  POIActivationDataKey() = default;

  //POIActivationDataKey( GUID g )
  //  : guid( g )
  //  , instanceID( 0 )
  //{

  //}

  POIActivationDataKey( GUID g, int inst )
    : guid( g )
    , uniqueData( inst )
  {

  }

  bool operator== ( const POIActivationDataKey& d )
  {
    return guid == d.guid && uniqueData == d.uniqueData;
  }

};

struct POIActivationData
{
  GUID poiguid;
  int uniqueData = 0;
  time_t lastUpdateTime = 0;
};

struct POIRoute
{
  CString name;
  TBOOL backwards = true;
  CArray<GUID> route;
  TBOOL external = false;
  TBOOL hasResetPos = false;
  CVector3 resetPos;
  float resetRad = 0;
  int MapID = 0;

  int32_t activeItem = -1;
};

TU32 DictionaryHash( const GUID &i );
TU32 DictionaryHash( const POIActivationDataKey &i );

extern CDictionaryEnumerable<GUID, POI> POIs;
extern CDictionaryEnumerable<POIActivationDataKey, POIActivationData> ActivationData;
extern CArray<POIRoute> Routes;
extern CDictionaryEnumerable<CString, POI> wvwPOIs;

class GW2TacticalDisplay : public CWBItem
{
  TBOOL TacticalIconsOnEdge;
  TF32 asp;
  CMatrix4x4 cam;
  CMatrix4x4 persp;
  CRect drawrect;

  void FetchAchievements();
  void InsertPOI( POI& poi );
  void DrawPOI( CWBDrawAPI *API, const tm& ptm, const time_t& currtime, POI& poi, bool drawDistance, CString& infoText );
  void DrawPOIMinimap( CWBDrawAPI *API, const CRect& miniRect, CVector2& pos, const tm& ptm, const time_t& currtime, POI& poi, float alpha, float zoomLevel );
  virtual void OnDraw( CWBDrawAPI *API );
  CVector3 ProjectTacticalPos( CVector3 pos, TF32 fov, TF32 asp );
  CArray<POI*> mapPOIs;
  CArray<POI*> minimapPOIs;
  bool drawWvWNames;

  bool beingFetched = false;
  bool achievementsFetched = false;
  int32_t lastFetchTime = 0;
  std::thread fetchThread;
  LIGHTWEIGHT_CRITICALSECTION dataWriteCritSec;

  CDictionary<int32_t, Achievement> achievements;

public:

  GW2TacticalDisplay( CWBItem *Parent, CRect Position );
  virtual ~GW2TacticalDisplay();

  static CWBItem *Factory( CWBItem *Root, CXMLNode &node, CRect &Pos );
  WB_DECLARE_GUIITEM( _T( "gw2tactical" ), CWBItem );

  virtual TBOOL IsMouseTransparent( CPoint &ClientSpacePoint, WBMESSAGE MessageType );
  void RemoveUserMarkersFromMap();

};

class GW2TacticalCategory
{
  CString cachedTypeName;

public:
  CString name;
  CString displayName;

  int16_t zipFile;

  MarkerTypeData data;
  TBOOL KeepSaveState = false;
  TBOOL IsOnlySeparator = false;
  GW2TacticalCategory *Parent = nullptr;
  CArray<GW2TacticalCategory*> children;

  CString GetFullTypeName();

  TBOOL IsDisplayed = true;
  TBOOL IsVisible();

  virtual ~GW2TacticalCategory()
  {
    children.FreeArray();
  }
};

void AddPOI( CWBApplication *App );
void DeletePOI();
void UpdatePOI();
void ImportPOIS( CWBApplication *App );
void ExportPOIS();
void ImportPOIActivationData();
void ExportPOIActivationData();

void OpenTypeContextMenu( CWBContextMenu *ctx, CArray<GW2TacticalCategory*> &CategoryList, TBOOL AddVisibilityMarkers = false, int32_t BaseID = 0, TBOOL closeOnClick = false );
void OpenTypeContextMenu( CWBContextItem *ctx, CArray<GW2TacticalCategory*> &CategoryList, TBOOL AddVisibilityMarkers = false, int32_t BaseID = 0, TBOOL closeOnClick = false );

float WorldToGameCoords( float world );
float GameToWorldCoords( float game );
void FindClosestRouteMarkers( TBOOL force );

int32_t GetTime();
int32_t AddStringToMap( const CString& string );
CString& GetStringFromMap( int32_t idx );
