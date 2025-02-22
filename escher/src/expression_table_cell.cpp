#include <escher/expression_table_cell.h>
#include <escher/container.h>
#include <escher/palette.h>
#include <assert.h>

namespace Escher {

ExpressionTableCell::ExpressionTableCell(Responder * parentResponder) :
  Responder(parentResponder),
  TableCell(),
  m_labelExpressionView(this, 0, 0, 0.0f, 0.5f, KDColorBlack, KDColorWhite)
{
}

void ExpressionTableCell::setHighlighted(bool highlight) {
  TableCell::setHighlighted(highlight);
  KDColor backgroundColor = highlight? Palette::Select : KDColorWhite;
  m_labelExpressionView.setBackgroundColor(backgroundColor);
}

void ExpressionTableCell::setLayout(Poincare::Layout layout) {
  m_labelExpressionView.setLayout(layout);
  if (!layout.isUninitialized()) {
    layoutSubviews();
  }
}

void ExpressionTableCell::didBecomeFirstResponder() {
  Container::activeApp()->setFirstResponder(&m_labelExpressionView);
}

}
