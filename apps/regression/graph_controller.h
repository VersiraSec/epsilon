#ifndef REGRESSION_GRAPH_CONTROLLER_H
#define REGRESSION_GRAPH_CONTROLLER_H

#include "banner_view.h"
#include "store.h"
#include "graph_options_controller.h"
#include "graph_view.h"
#include "../shared/interactive_curve_view_controller.h"
#include "../shared/curve_view_cursor.h"
#include "../shared/cursor_view.h"
#include "../shared/round_cursor_view.h"

namespace Regression {

class GraphController : public Shared::InteractiveCurveViewController {

public:
  GraphController(Escher::Responder * parentResponder, Escher::InputEventHandlerDelegate * inputEventHandlerDelegate, Escher::ButtonRowController * header, Store * store, Shared::CurveViewCursor * cursor, uint32_t * rangeVersion, int * selectedDotIndex, int * selectedSeriesIndex);
  bool isEmpty() const override;
  I18n::Message emptyMessage() override;
  void viewWillAppear() override;
  void selectRegressionCurve() { *m_selectedDotIndex = -1; }
  int selectedSeriesIndex() const { return *m_selectedSeriesIndex; }

  // moveCursorHorizontally and Vertically are public to be used in tests
  bool moveCursorHorizontally(int direction, int scrollSpeed = 1) override;
  bool moveCursorVertically(int direction) override;

private:
  constexpr static int k_maxLegendLength = 16;

  Poincare::Context * globalContext();

  // SimpleInteractiveCurveViewController
  float cursorBottomMarginRatio() override { return cursorBottomMarginRatioForBannerHeight(m_bannerView.minimalSizeForOptimalDisplay().height()); }
  void reloadBannerView() override;
  Shared::InteractiveCurveViewRange * interactiveCurveViewRange() override;
  Shared::CurveView * curveView() override;
  bool openMenu() override;

  // InteractiveCurveViewController
  void initCursorParameters() override;
  bool cursorMatchesModel() override;
  uint32_t rangeVersion() override;
  int selectedCurveIndex() const override { return *m_selectedSeriesIndex; }
  bool closestCurveIndexIsSuitable(int newIndex, int currentIndex) const override;
  Poincare::Coordinate2D<double> xyValues(int curveIndex, double x, Poincare::Context * context) const override;
  double yValue(int curveIndex, double x, Poincare::Context * context) const;
  bool suitableYValue(double y) const override;
  int numberOfCurves() const override;

  void setRoundCrossCursorView();
  Shared::CursorView m_crossCursorView;
  Shared::RoundCursorView m_roundCursorView;
  BannerView m_bannerView;
  GraphView m_view;
  Store * m_store;
  GraphOptionsController m_graphOptionsController;
  /* The selectedDotIndex is -1 when no dot is selected, m_numberOfPairs when
   * the mean dot is selected and the dot index otherwise */
  int * m_selectedDotIndex;
  int * m_selectedSeriesIndex;
  Model::Type m_modelType[Store::k_numberOfSeries];
};

}


#endif
