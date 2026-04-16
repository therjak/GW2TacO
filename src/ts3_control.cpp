module;
#include <algorithm>
#include <format>

module taco.ts3_control;

import taco.language;
import taco.overlay_config;
import taco.ts3connection;
import whiteboard;

using math::Point;
using math::Rect;

void Ts3Control::OnDraw(gui::CWBDrawAPI* API) {
  gui::CWBFont* f = GetFont(GetState());
  int32_t size = f->GetLineHeight();

  if (!teamSpeakConnection.authenticated_) {
    // If we got no ts running, just quit. Do not annoy with useless screen
    // clutter.
    return;
    if (HasConfigString("TS3APIKey")) {
      f->Write(API, DICT("ts3authfail1"), Point(0, 0), CColor{0xffffffff});
      f->Write(API, DICT("ts3authfail2"), Point(0, size), CColor{0xffffffff});
      f->Write(API, DICT("ts3authfail3"), Point(0, size * 2),
               CColor{0xffffffff});
      f->Write(API, DICT("ts3authfail4"), Point(0, size * 3),
               CColor{0xfffffff});
    } else {
      f->Write(API, DICT("ts3nokeyset1"), Point(0, 0), CColor{0xffffffff});
      f->Write(API, DICT("ts3nokeyset2"), Point(0, size), CColor{0xffffffff});
      f->Write(API, DICT("ts3nokeyset3"), Point(0, size * 2),
               CColor{0xffffffff});
    }
  }

  gui::WBSKINELEMENTID playeroff = App->GetSkin()->GetElementID("ts3playeroff");
  gui::WBSKINELEMENTID playeron = App->GetSkin()->GetElementID("ts3playeron");
  gui::WBSKINELEMENTID outputoff =
      App->GetSkin()->GetElementID("ts3outputmuted");
  gui::WBSKINELEMENTID inputoff = App->GetSkin()->GetElementID("ts3inputmuted");

  bool LeftAlign = true;
  Rect r = ClientToScreen(GetClientRect());
  LeftAlign = r.x1 < App->GetXRes() / 2 && r.x2 < App->GetXRes() / 2;

  Rect displayrect = GetClientRect();

  for (int32_t cnt = 0; cnt < 2; cnt++) {
    int32_t ypos = 0;
    for (auto& x : teamSpeakConnection.handlers_) {
      TS3Connection::TS3Schandler& handler = x.second;
      if (handler.connected &&
          handler.clients.find(handler.my_client_id) != handler.clients.end()) {
        Point p = f->GetTextPosition(
            handler.name, GetClientRect() - Rect(0, ypos, 0, 0),
            LeftAlign ? gui::WBTEXTALIGNMENTX::WBTA_LEFT
                      : gui::WBTEXTALIGNMENTX::WBTA_RIGHT,
            gui::WBTEXTALIGNMENTY::WBTA_TOP, gui::WBTEXTTRANSFORM::WBTT_NONE,
            true);
        if (cnt) f->Write(API, handler.name, p);
        ypos += f->GetLineHeight();

        int32_t mychannelid = handler.clients[handler.my_client_id].channel_id;

        if (handler.channels.find(mychannelid) != handler.channels.end()) {
          int32_t participants = 0;
          for (auto& y : handler.clients) {
            const TS3Connection::TS3Client& cl = y.second;
            if (cl.channel_id == mychannelid) participants++;
          }

          auto channel_text = std::format(
              "{:s} ({:d})", handler.channels[mychannelid].name, participants);

          Point p = f->GetTextPosition(
              channel_text, GetClientRect() - Rect(size / 2, ypos, 0, 0),
              LeftAlign ? gui::WBTEXTALIGNMENTX::WBTA_LEFT
                        : gui::WBTEXTALIGNMENTX::WBTA_RIGHT,
              gui::WBTEXTALIGNMENTY::WBTA_TOP, gui::WBTEXTTRANSFORM::WBTT_NONE,
              true);
          if (cnt) f->Write(API, channel_text, p);
          ypos += f->GetLineHeight();
        }

        std::vector<TS3Connection::TS3Client*> clients;
        for (auto& y : handler.clients) {
          clients.push_back(&y.second);
        }

        std::sort(clients.begin(), clients.end(),
                  [](const TS3Connection::TS3Client* a,
                     const TS3Connection::TS3Client* b) {
                    return b->last_talk_time < a->last_talk_time;
                  });

        for (const auto cl : clients) {
          if ((ypos + f->GetLineHeight()) > displayrect.y2) break;

          if (cl->channel_id == mychannelid) {
            gui::WBSKINELEMENTID id = playeroff;
            if (cl->input_muted) id = inputoff;
            if (cl->output_muted) id = outputoff;
            if (cl->talk_status) id = playeron;

            App->GetSkin()->RenderElement(
                API, id,
                LeftAlign ? Rect(size / 2, ypos, size / 2 + size - 1,
                                  ypos + size - 1)
                          : Rect(GetClientRect().Width() - size / 2 - size + 1,
                                  ypos, GetClientRect().Width() - size / 2,
                                  ypos + size - 1));

            Point p = f->GetTextPosition(
                cl->name, GetClientRect() - Rect(2 * size, ypos, 2 * size, 0),
                LeftAlign ? gui::WBTEXTALIGNMENTX::WBTA_LEFT
                          : gui::WBTEXTALIGNMENTX::WBTA_RIGHT,
                gui::WBTEXTALIGNMENTY::WBTA_TOP,
                gui::WBTEXTTRANSFORM::WBTT_NONE, true);

            if (cnt) f->Write(API, cl->name, p);

            ypos += f->GetLineHeight();
          }
        }
      }
      ypos += size / 2;
    }

    if (!cnt) {
      DrawBackgroundItem(API, CSSProperties.DisplayDescriptor,
                         Rect(0, 0, GetClientRect().Width(), ypos),
                         GetState());
    }
  }

  DrawBorder(API);
}

Ts3Control::Ts3Control() : CWBGuiType() {}

Ts3Control::~Ts3Control() = default;

gui::CWBItem* Ts3Control::Factory(gui::CWBItem* Root, CXMLNode& node,
                                  Rect& Pos) {
  return Ts3Control::Create(Root, Pos);
}

bool Ts3Control::IsMouseTransparent(const Point& ClientSpacePoint,
                                    gui::WBMESSAGE MessageType) {
  return true;
}
