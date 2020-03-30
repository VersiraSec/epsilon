#include "plot_view.h"

namespace Matplotlib {

void PlotView::drawRect(KDContext * ctx, KDRect rect) const {
  ctx->fillRect(rect, KDColorWhite);

  if (m_store->gridRequested()) {
    drawGrid(ctx, rect);
  }

  // Draw labels below all figures because they're drawn on a white rectangle.
  // TODO: we could blend them in the background by adding a parameter to drawLabelsAndGraduations.
  if (m_store->axesRequested()) {
    drawAxes(ctx, rect);
    drawLabelsAndGraduations(ctx, rect, Axis::Vertical, true);
    drawLabelsAndGraduations(ctx, rect, Axis::Horizontal, true);
  }

  for (PlotStore::Dot dot : m_store->dots()) {
    traceDot(ctx, rect, dot);
  }

  for (PlotStore::Label label : m_store->labels()) {
    traceLabel(ctx, rect, label);
  }

  for (PlotStore::Segment segment : m_store->segments()) {
    traceSegment(ctx, rect, segment);
  }

  for (PlotStore::Rect rectangle : m_store->rects()) {
    traceRect(ctx, rect, rectangle);
  }
}

void PlotView::traceDot(KDContext * ctx, KDRect r, PlotStore::Dot dot) const {
  drawDot(ctx, r, dot.x(), dot.y(), dot.color());
}

void PlotView::traceSegment(KDContext * ctx, KDRect r, PlotStore::Segment segment) const {
  drawSegment(
    ctx, r,
    segment.xStart(), segment.yStart(),
    segment.xEnd(), segment.yEnd(),
    segment.color()
  );
  if (segment.isArrow()) {
    float dx = segment.xEnd() - segment.xStart();
    float dy = segment.yEnd() - segment.yStart();
    drawArrow(ctx, r, segment.xEnd(), segment.yEnd(), dx, dy, segment.color());
  }
}

static inline KDCoordinate maxKDCoordinate(KDCoordinate x, KDCoordinate y) { return x > y ? x : y; }
void PlotView::traceRect(KDContext * ctx, KDRect r, PlotStore::Rect rect) const {
  KDRect pixelRect(
    std::round(floatToPixel(Axis::Horizontal, rect.x())),
    std::round(floatToPixel(Axis::Vertical, rect.y())),
    maxKDCoordinate(std::round(rect.width() / pixelWidth()), 1), // Rectangle should at least be visible
    std::round(rect.height() / pixelHeight())
  );
  ctx->fillRect(pixelRect, rect.color());
}

void PlotView::traceLabel(KDContext * ctx, KDRect r, PlotStore::Label label) const {
  drawLabel(ctx, r,
    label.x(), label.y(), label.string(),
    KDColorBlack,
    RelativePosition::None,
    RelativePosition::None
  );
}


}
