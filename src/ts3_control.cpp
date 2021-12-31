#include "src/ts3_control.h"

#include <algorithm>

#include "src/base/string_format.h"
#include "src/language.h"
#include "src/overlay_config.h"
#include "src/ts3_connection.h"
#include "src/white_board/application.h"

using math::CPoint;
using math::CRect;

void TS3Control::OnDraw(CWBDrawAPI* API) {
  CWBFont* f = GetFont(GetState());
  int32_t size = f->GetLineHeight();

  if (!teamSpeakConnection.authenticated) {
    if (HasConfigString("TS3APIKey")) {
      f->Write(API, DICT("ts3authfail1"), CPoint(0, 0), CColor{0xffffffff});
      f->Write(API, DICT("ts3authfail2"), CPoint(0, size), CColor{0xffffffff});
      f->Write(API, DICT("ts3authfail3"), CPoint(0, size * 2),
               CColor{0xffffffff});
      f->Write(API, DICT("ts3authfail4"), CPoint(0, size * 3),
               CColor{0xfffffff});
    } else {
      f->Write(API, DICT("ts3nokeyset1"), CPoint(0, 0), CColor{0xffffffff});
      f->Write(API, DICT("ts3nokeyset2"), CPoint(0, size), CColor{0xffffffff});
      f->Write(API, DICT("ts3nokeyset3"), CPoint(0, size * 2),
               CColor{0xffffffff});
    }
  }

  WBSKINELEMENTID playeroff = App->GetSkin()->GetElementID("ts3playeroff");
  WBSKINELEMENTID playeron = App->GetSkin()->GetElementID("ts3playeron");
  WBSKINELEMENTID outputoff = App->GetSkin()->GetElementID("ts3outputmuted");
  WBSKINELEMENTID inputoff = App->GetSkin()->GetElementID("ts3inputmuted");

  bool LeftAlign = true;
  CRect r = ClientToScreen(GetClientRect());
  LeftAlign = r.x1 < App->GetXRes() / 2 && r.x2 < App->GetXRes() / 2;

  CRect displayrect = GetClientRect();

  for (int32_t cnt = 0; cnt < 2; cnt++) {
    int32_t ypos = 0;
    for (auto& x : teamSpeakConnection.handlers) {
      TS3Connection::TS3Schandler& handler = x.second;
      if (handler.Connected &&
          handler.Clients.find(handler.myclientid) != handler.Clients.end()) {
        CPoint p = f->GetTextPosition(
            handler.name, GetClientRect() - CRect(0, ypos, 0, 0),
            LeftAlign ? WBTEXTALIGNMENTX::WBTA_LEFT
                      : WBTEXTALIGNMENTX::WBTA_RIGHT,
            WBTEXTALIGNMENTY::WBTA_TOP, WBTEXTTRANSFORM::WBTT_NONE, true);
        if (cnt) f->Write(API, handler.name, p);
        ypos += f->GetLineHeight();

        int32_t mychannelid = handler.Clients[handler.myclientid].channelid;

        if (handler.Channels.find(mychannelid) != handler.Channels.end()) {
          int32_t participants = 0;
          for (auto& y : handler.Clients) {
            const TS3Connection::TS3Client& cl = y.second;
            if (cl.channelid == mychannelid) participants++;
          }

          auto channelText = FormatString(
              "%s (%d)", handler.Channels[mychannelid].name.c_str(),
              participants);

          CPoint p = f->GetTextPosition(
              channelText, GetClientRect() - CRect(size / 2, ypos, 0, 0),
              LeftAlign ? WBTEXTALIGNMENTX::WBTA_LEFT
                        : WBTEXTALIGNMENTX::WBTA_RIGHT,
              WBTEXTALIGNMENTY::WBTA_TOP, WBTEXTTRANSFORM::WBTT_NONE, true);
          if (cnt) f->Write(API, channelText, p);
          ypos += f->GetLineHeight();
        }

        std::vector<TS3Connection::TS3Client*> clients;
        for (auto& y : handler.Clients) {
          clients.push_back(&y.second);
        }

        std::sort(clients.begin(), clients.end(),
                  [](const TS3Connection::TS3Client* a,
                     const TS3Connection::TS3Client* b) {
                    return b->lastTalkTime < a->lastTalkTime;
                  });

        for (const auto cl : clients) {
          if ((ypos + f->GetLineHeight()) > displayrect.y2) break;

          if (cl->channelid == mychannelid) {
            WBSKINELEMENTID id = playeroff;
            if (cl->inputmuted) id = inputoff;
            if (cl->outputmuted) id = outputoff;
            if (cl->talkStatus) id = playeron;

            App->GetSkin()->RenderElement(
                API, id,
                LeftAlign ? CRect(size / 2, ypos, size / 2 + size - 1,
                                  ypos + size - 1)
                          : CRect(GetClientRect().Width() - size / 2 - size + 1,
                                  ypos, GetClientRect().Width() - size / 2,
                                  ypos + size - 1));

            CPoint p = f->GetTextPosition(
                cl->name, GetClientRect() - CRect(2 * size, ypos, 2 * size, 0),
                LeftAlign ? WBTEXTALIGNMENTX::WBTA_LEFT
                          : WBTEXTALIGNMENTX::WBTA_RIGHT,
                WBTEXTALIGNMENTY::WBTA_TOP, WBTEXTTRANSFORM::WBTT_NONE, true);

            if (cnt) f->Write(API, cl->name, p);

            ypos += f->GetLineHeight();
          }
        }
      }
      ypos += size / 2;
    }

    if (!cnt)
      DrawBackgroundItem(API, CSSProperties.DisplayDescriptor,
                         CRect(0, 0, GetClientRect().Width(), ypos),
                         GetState());
  }

  DrawBorder(API);
}

TS3Control::TS3Control(CWBItem* Parent, CRect Position)
    : CWBItem(Parent, Position) {}

TS3Control::~TS3Control() = default;

CWBItem* TS3Control::Factory(CWBItem* Root, CXMLNode& node, CRect& Pos) {
  return TS3Control::Create(Root, Pos).get();
}

bool TS3Control::IsMouseTransparent(const CPoint& ClientSpacePoint,
                                    WBMESSAGE MessageType) {
  return true;
}
