#include "src/locational_timer.h"

#include <format>
#include <vector>

#include "src/base/rectangle.h"
#include "src/base/sphere.h"
#include "src/base/vector.h"
#include "src/mumble_link.h"
#include "src/overlay_config.h"

using math::CPoint;
using math::CRect;

std::vector<LocationalTimer> LocationalTimers;

void ImportLocationalTimers() {
  CXMLDocument d;
  if (!d.LoadFromFile("locationaltimers.xml")) return;

  if (!d.GetDocumentNode().GetChildCount("timers")) return;
  CXMLNode root = d.GetDocumentNode().GetChild("timers");

  for (int32_t x = 0; x < root.GetChildCount("areatriggeredtimer"); x++) {
    LocationalTimer t;
    t.ImportData(root.GetChild("areatriggeredtimer", x));
    LocationalTimers.push_back(t);
  }
}

LocationalTimer::LocationalTimer() = default;

LocationalTimer::~LocationalTimer() = default;

void LocationalTimer::Update() {
  if (mumbleLink.mapID != MapID) {
    IsRunning = false;
    return;
  }

  if (!IsRunning) {
    if (EnterSphere.Contains(mumbleLink.charPosition)) {
      IsRunning = true;
      StartTime = GetTime();
    }
  }

  if (IsRunning) {
    if ((GetTime() - StartTime) / 1000.0f > TimerLength) IsRunning = false;
    if (!ExitSphere.Contains(mumbleLink.charPosition)) IsRunning = false;
    if ((ResetPoint - mumbleLink.charPosition).Length() < 0.1)
      IsRunning = false;
  }
}

void LocationalTimer::ImportData(const CXMLNode& node) {
  if (node.HasAttribute("mapid")) node.GetAttributeAsInteger("mapid", &MapID);
  if (node.HasAttribute("length"))
    node.GetAttributeAsInteger("length", &TimerLength);
  if (node.HasAttribute("startdelay"))
    node.GetAttributeAsInteger("startdelay", &StartDelay);

  if (node.HasAttribute("enterspherex"))
    node.GetAttributeAsFloat("enterspherex", &EnterSphere.Position.x);
  if (node.HasAttribute("enterspherey"))
    node.GetAttributeAsFloat("enterspherey", &EnterSphere.Position.y);
  if (node.HasAttribute("enterspherez"))
    node.GetAttributeAsFloat("enterspherez", &EnterSphere.Position.z);
  if (node.HasAttribute("entersphererad"))
    node.GetAttributeAsFloat("entersphererad", &EnterSphere.Radius);

  if (node.HasAttribute("exitspherex"))
    node.GetAttributeAsFloat("exitspherex", &ExitSphere.Position.x);
  if (node.HasAttribute("exitspherey"))
    node.GetAttributeAsFloat("exitspherey", &ExitSphere.Position.y);
  if (node.HasAttribute("exitspherez"))
    node.GetAttributeAsFloat("exitspherez", &ExitSphere.Position.z);
  if (node.HasAttribute("exitsphererad"))
    node.GetAttributeAsFloat("exitsphererad", &ExitSphere.Radius);

  if (node.HasAttribute("resetpointx"))
    node.GetAttributeAsFloat("resetpointx", &ResetPoint.x);
  if (node.HasAttribute("resetpointy"))
    node.GetAttributeAsFloat("resetpointy", &ResetPoint.y);
  if (node.HasAttribute("resetpointz"))
    node.GetAttributeAsFloat("resetpointz", &ResetPoint.z);

  for (int32_t x = 0; x < node.GetChildCount("timeevent"); x++) {
    CXMLNode te = node.GetChild("timeevent", x);
    TimerEvent tmr;
    if (te.HasAttribute("text")) tmr.Text = te.GetAttributeAsString("text");
    if (te.HasAttribute("timestamp"))
      te.GetAttributeAsInteger("timestamp", &tmr.Time);
    if (te.HasAttribute("countdown"))
      te.GetAttributeAsInteger("countdown", &tmr.CountdownLength);
    if (te.HasAttribute("onscreentime"))
      te.GetAttributeAsInteger("onscreentime", &tmr.OnScreenLength);
    Events.push_back(tmr);
  }
}

void TimerDisplay::OnDraw(CWBDrawAPI* API) {
  if (!HasConfigValue("LocationalTimersVisible"))
    SetConfigValue("LocationalTimersVisible", 1);
  if (!GetConfigValue("LocationalTimersVisible")) return;

  int32_t tme = GetTime();
  CWBFont* f = GetFont(GetState());

  int32_t ypos = math::Lerp(GetClientRect().y1, GetClientRect().y2, 0.25f);

  for (auto& t : LocationalTimers) {
    t.Update();
    if (!t.IsRunning) continue;

    float timepos = (tme - t.StartTime) / 1000.0f - t.StartDelay;

    for (auto& e : t.Events) {
      if (!(timepos > e.Time - e.CountdownLength &&
            timepos < e.Time + e.OnScreenLength))
        continue;

      auto s = e.Text;
      if (timepos < e.Time && timepos > e.Time - e.CountdownLength)
        s += std::format(" in {:d}", static_cast<int32_t>(e.Time - timepos));

      CPoint pos = f->GetTextPosition(
          s, CRect(GetClientRect().x1, ypos, GetClientRect().x2, ypos),
          WBTEXTALIGNMENTX::WBTA_CENTERX, WBTEXTALIGNMENTY::WBTA_CENTERY,
          WBTEXTTRANSFORM::WBTT_NONE, true);
      ypos += f->GetLineHeight();
      f->Write(API, s, pos);
    }
  }
}

bool TimerDisplay::IsMouseTransparent(const CPoint& ClientSpacePoint,
                                      WBMESSAGE MessageType) {
  return true;
}

TimerDisplay::TimerDisplay(CWBItem* Parent, CRect Position)
    : CWBItem(Parent, Position) {}

TimerDisplay::~TimerDisplay() = default;

CWBItem* TimerDisplay::Factory(CWBItem* Root, const CXMLNode& node,
                               CRect& Pos) {
  return TimerDisplay::Create(Root, Pos).get();
}
