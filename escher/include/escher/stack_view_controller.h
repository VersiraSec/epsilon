#ifndef ESCHER_STACK_VIEW_CONTROLLER_H
#define ESCHER_STACK_VIEW_CONTROLLER_H

#include <escher/view_controller.h>
#include <escher/stack_view.h>
#include <escher/palette.h>
#include <escher/solid_color_view.h>

namespace Escher {

constexpr uint8_t kMaxNumberOfStacks = 5;

class StackViewController : public ViewController {
public:
  StackViewController(Responder * parentResponder, ViewController * rootViewController,
    KDColor textColor = KDColorWhite, KDColor backgroundColor = Palette::PopUpTitleBackground, KDColor separatorColor = Palette::PurpleDark);

  /* Push creates a new StackView and adds it */
  void push(ViewController * vc, KDColor textColor = Palette::SubTab, KDColor backgroundColor = KDColorWhite, KDColor separatorColor = Palette::GrayBright);
  void pop();
  void popUntilDepth(int depth, bool shouldSetupTopViewController);

  int depth() const { return m_numberOfChildren; }
  View * view() override { return &m_view; }
  ViewController * topViewController();
  const char * title() override;
  bool handleEvent(Ion::Events::Event event) override;
  void didBecomeFirstResponder() override;
  void initView() override;
  void viewWillAppear() override;
  void viewDidDisappear() override;
  void setupHeadersBorderOverlaping(bool headersOverlapHeaders = true, bool headersOverlapContent = false, KDColor headersContentBorderColor = Palette::GrayBright) {
    m_view.setupHeadersBorderOverlaping(headersOverlapHeaders, headersOverlapContent, headersContentBorderColor);
  }
  static constexpr uint8_t k_maxNumberOfChildren = kMaxNumberOfStacks;
private:
  class Frame {
  public:
    Frame(ViewController * viewController = nullptr, KDColor textColor = Palette::SubTab, KDColor backgroundColor = KDColorWhite, KDColor separatorColor = Palette::GrayBright) :
      m_viewController(viewController),
      m_textColor(textColor),
      m_backgroundColor(backgroundColor),
      m_separatorColor(separatorColor) {}
    ViewController * viewController() { return m_viewController; }
    KDColor textColor() { return m_textColor; }
    KDColor backgroundColor() { return m_backgroundColor; }
    KDColor separatorColor() { return m_separatorColor; }
  private:
    ViewController * m_viewController;
    KDColor m_textColor;
    KDColor m_backgroundColor;
    KDColor m_separatorColor;
  };
  class ControllerView : public View {
  public:
    ControllerView();
    void shouldDisplayStackHeaders(bool shouldDisplay);
    int8_t numberOfStacks() const { return m_numberOfStacks; }
    void setContentView(View * view);
    void setupHeadersBorderOverlaping(bool headersOverlapHeaders, bool headersOverlapContent, KDColor headersContentBorderColor);
    void pushStack(Frame frame);
    void popStack();
  protected:
#if ESCHER_VIEW_LOGGING
  const char * className() const override;
#endif
  private:
    int numberOfSubviews() const override;
    View * subviewAtIndex(int index) override;
    void layoutSubviews(bool force = false) override;
    bool borderShouldOverlapContent() const;

    StackView m_stackViews[kMaxNumberOfStacks];
    SolidColorView m_borderView;
    View * m_contentView;
    int8_t m_numberOfStacks;
    bool m_displayStackHeaders;
    bool m_headersOverlapHeaders;
    bool m_headersOverlapContent;
  };
  ControllerView m_view;
  void pushModel(Frame frame);
  void setupActiveViewController();
  Frame m_childrenFrame[k_maxNumberOfChildren];
  uint8_t m_numberOfChildren;
  bool m_isVisible;
};

}
#endif
