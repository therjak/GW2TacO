module;
#include <memory>

export module whiteboard:root;

import :css_item;
import :gui_item;

export namespace gui {

class Application;

class Root : public GuiType<"root", Widget> {
 public:
  Root();
  static inline std::unique_ptr<Root> Create(const math::Rect& Pos) {
    auto p = std::make_unique<Root>();
    p->Initialize(nullptr, Pos);
    return p;
  }
  ~Root() override;

  void SetApplication(Application* Application);

 private:
  bool MessageProc(const Message& message) override;
  void OnDraw(DrawAPI* API) override;
};

}  // namespace gui
