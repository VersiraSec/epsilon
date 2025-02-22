#include "histogram_controller.h"
#include "../shared/poincare_helpers.h"
#include "app.h"
#include <poincare/ieee754.h>
#include <poincare/preferences.h>
#include <algorithm>
#include <cmath>
#include <assert.h>
#include <float.h>

using namespace Poincare;
using namespace Shared;
using namespace Escher;

namespace Statistics {

HistogramController::HistogramController(Responder * parentResponder, InputEventHandlerDelegate * inputEventHandlerDelegate, ButtonRowController * header, Store * store, uint32_t * storeVersion, uint32_t * barVersion, uint32_t * rangeVersion, int * selectedBarIndex, int * selectedSeriesIndex) :
  MultipleDataViewController(parentResponder, store, selectedBarIndex, selectedSeriesIndex),
  ButtonRowDelegate(header, nullptr),
  m_view(this, store),
  m_storeVersion(storeVersion),
  m_barVersion(barVersion),
  m_rangeVersion(rangeVersion),
  m_histogramParameterController(nullptr, inputEventHandlerDelegate, store),
  m_parameterButton(this, I18n::Message::HistogramSet, Invocation([](void * context, void * sender) {
    HistogramController * histogramController = static_cast<HistogramController * >(context);
    histogramController->stackController()->push(histogramController->histogramParameterController());
    return true;
  }, this), KDFont::SmallFont)
{
}

void HistogramController::setCurrentDrawnSeries(int series) {
  initYRangeParameters(series);
  /* The histogram's CurveView range has been updated along the Vertical axis.
   * To call drawLabelsAndGraduations (in HistogramView::drawRect()), the
   * CurveView must be reloaded so that labels and their values match the new
   * range.
   * In this situation, we update CurveView's Vertical axis, and draw horizontal
   * labels, which are independent. To avoid having to call CurveView::reload(),
   * axis could be taken into account when checking if labels are up to date,
   * instead of using rangeChecksum(), which mixes all axis. */
  m_view.dataViewAtIndex(series)->CurveView::reload();
}

StackViewController * HistogramController::stackController() {
  StackViewController * stack = (StackViewController *)(parentResponder()->parentResponder()->parentResponder());
  return stack;
}

Button * HistogramController::buttonAtIndex(int index, ButtonRowController::Position position) const {
  assert(index == 0);
  return const_cast<Button *>(&m_parameterButton);
}

const char * HistogramController::title() {
  return I18n::translate(I18n::Message::HistogramTab);
}

bool HistogramController::handleEvent(Ion::Events::Event event) {
  if (header()->selectedButton() == 0) {
    if (event == Ion::Events::Down) {
      header()->setSelectedButton(-1);
      multipleDataView()->setDisplayBanner(true);
      multipleDataView()->selectDataView(selectedSeriesIndex());
      highlightSelection();
      reloadBannerView();
      return true;
    }
    if (event == Ion::Events::Up) {
      Container::activeApp()->setFirstResponder(tabController());
      return true;
    }
    return false;
  }
  if (selectedSeriesIndex() == 0 && event == Ion::Events::Up) {
    multipleDataView()->deselectDataView(selectedSeriesIndex());
    multipleDataView()->setDisplayBanner(false);
    header()->setSelectedButton(0);
    return true;
  }
  if (event == Ion::Events::OK || event == Ion::Events::EXE) {
    stackController()->push(histogramParameterController());
    return true;
  }
  return MultipleDataViewController::handleEvent(event);
}

void HistogramController::viewWillAppear() {
  if (header()->selectedButton() >= 0) {
    header()->setSelectedButton(-1);
  }
  MultipleDataViewController::viewWillAppear();

  multipleDataView()->setDisplayBanner(true);
  multipleDataView()->selectDataView(selectedSeriesIndex());
  highlightSelection();

  uint32_t storeChecksum = m_store->storeChecksum();
  bool initedRangeParameters = false;
  if (*m_storeVersion != storeChecksum) {
    *m_storeVersion = storeChecksum;
    initBarParameters();
    initRangeParameters();
    initedRangeParameters = true;
  }
  uint32_t barChecksum = m_store->barChecksum();
  if (*m_barVersion != barChecksum) {
    *m_barVersion = barChecksum;
    if (!initedRangeParameters) {
      initRangeParameters();
    }
  }
  uint32_t rangeChecksum = m_store->rangeChecksum();
  if (*m_rangeVersion != rangeChecksum) {
    *m_rangeVersion = rangeChecksum;
    initBarSelection();
    reloadBannerView();
    multipleDataView()->reload();
  }
}

void HistogramController::didEnterResponderChain(Responder * firstResponder) {
  assert(selectedSeriesIndex() >= 0);
  if (!multipleDataView()->dataViewAtIndex(selectedSeriesIndex())->isMainViewSelected()) {
    header()->setSelectedButton(0);
  }
}

void HistogramController::willExitResponderChain(Responder * nextFirstResponder) {
  if (nextFirstResponder == tabController()) {
    assert(tabController() != nullptr);
    if (header()->selectedButton() == 0) {
      header()->setSelectedButton(-1);
      return;
    }
    assert(selectedSeriesIndex() >= 0);
  }
  MultipleDataViewController::willExitResponderChain(nextFirstResponder);
}

void HistogramController::highlightSelection() {
  HistogramView * selectedHistogramView = static_cast<HistogramView *>(m_view.dataViewAtIndex(selectedSeriesIndex()));
  selectedHistogramView->setHighlight(m_store->startOfBarAtIndex(selectedSeriesIndex(), *m_selectedBarIndex), m_store->endOfBarAtIndex(selectedSeriesIndex(), *m_selectedBarIndex));
  // if the selectedBar was outside of range, we need to scroll
  if (m_store->scrollToSelectedBarIndex(selectedSeriesIndex(), *m_selectedBarIndex)) {
    multipleDataView()->reload();
  }
}

Responder * HistogramController::tabController() const {
  return (parentResponder()->parentResponder()->parentResponder()->parentResponder());
}

void HistogramController::reloadBannerView() {
  if (selectedSeriesIndex() < 0) {
    return;
  }
  int precision = Preferences::sharedPreferences()->numberOfSignificantDigits();
  constexpr size_t bufferSize = k_maxNumberOfCharacters + 2 * PrintFloat::charSizeForFloatsWithPrecision(Poincare::PrintFloat::k_numberOfStoredSignificantDigits);
  char buffer[bufferSize] = "";
  int numberOfChar = 0;

  // Add Interval Data
  const char * legend = " [";
  /* The word Interval is just a bit too long to display two numbers with a sign and maximal precision after it. */
  int intervalPrecision = std::min(precision, Poincare::PrintFloat::k_numberOfStoredSignificantDigits - 2);
  int legendLength = strlen(legend);
  strlcpy(buffer, legend, bufferSize);
  numberOfChar += legendLength;

  // Add lower bound
  if (selectedSeriesIndex() >= 0) {
    double lowerBound = m_store->startOfBarAtIndex(selectedSeriesIndex(), *m_selectedBarIndex);
    numberOfChar += PoincareHelpers::ConvertFloatToText<double>(lowerBound, buffer+numberOfChar, bufferSize-numberOfChar, intervalPrecision);
  }

  numberOfChar+= UTF8Decoder::CodePointToChars(';', buffer + numberOfChar, bufferSize - numberOfChar - 1);

  // Add upper bound
  if (selectedSeriesIndex() >= 0) {
    double upperBound = m_store->endOfBarAtIndex(selectedSeriesIndex(), *m_selectedBarIndex);
    numberOfChar += PoincareHelpers::ConvertFloatToText<double>(upperBound, buffer+numberOfChar, bufferSize-numberOfChar, intervalPrecision);
  }
  numberOfChar+= UTF8Decoder::CodePointToChars('[', buffer + numberOfChar, bufferSize - numberOfChar - 1);

  buffer[numberOfChar++] = '\0';
  m_view.bannerView()->intervalView()->setText(buffer);

  // Add Size Data
  buffer[0] = ' ';
  numberOfChar = 1;
  double size = 0;
  if (selectedSeriesIndex() >= 0) {
    size = m_store->heightOfBarAtIndex(selectedSeriesIndex(), *m_selectedBarIndex);
    numberOfChar += PoincareHelpers::ConvertFloatToText<double>(size, buffer+numberOfChar, bufferSize-numberOfChar, precision);
  }
  buffer[numberOfChar++] = '\0';
  m_view.bannerView()->sizeView()->setText(buffer);

  // Add Frequency Data
  buffer[0] = ' ';
  numberOfChar = 1;
  if (selectedSeriesIndex() >= 0) {
    double frequency = size/m_store->sumOfOccurrences(selectedSeriesIndex());
    numberOfChar += PoincareHelpers::ConvertFloatToText<double>(frequency, buffer+numberOfChar, bufferSize - numberOfChar, precision);
  }
  buffer[numberOfChar++] = '\0';
  m_view.bannerView()->frequencyView()->setText(buffer);

  m_view.bannerView()->reload();
}

bool HistogramController::moveSelectionHorizontally(int deltaIndex) {
  int newSelectedBarIndex = *m_selectedBarIndex;
  do {
    newSelectedBarIndex+=deltaIndex;
  } while (m_store->heightOfBarAtIndex(selectedSeriesIndex(), newSelectedBarIndex) == 0
      && newSelectedBarIndex >= 0
      && newSelectedBarIndex < m_store->numberOfBars(selectedSeriesIndex()));

  if (newSelectedBarIndex >= 0
      && newSelectedBarIndex < m_store->numberOfBars(selectedSeriesIndex())
      && *m_selectedBarIndex != newSelectedBarIndex)
  {
    *m_selectedBarIndex = newSelectedBarIndex;
    m_view.dataViewAtIndex(selectedSeriesIndex())->setHighlight(m_store->startOfBarAtIndex(selectedSeriesIndex(), *m_selectedBarIndex), m_store->endOfBarAtIndex(selectedSeriesIndex(), *m_selectedBarIndex));
    /* Reload the view even if it did not scroll because the banner height
     * might have changed. */
    reloadBannerView();
    multipleDataView()->reload();
    return true;
  }
  return false;
}

void HistogramController::preinitXRangeParameters(double * xMin) {
  /* Compute m_store's min and max values, hold them temporarily in the
   * CurveViewRange, for later use by initRangeParameters and
   * initBarParameters. Indeed, initRangeParameters will anyway alter the
   * CurveViewRange. The CurveViewRange setter methods take care of the case
   * where minValue >= maxValue. Moreover they compute the xGridUnit, which is
   * used by initBarParameters. */
  double minValue = DBL_MAX;
  double maxValue = -DBL_MAX;
  for (int i = 0; i < Store::k_numberOfSeries; i ++) {
    if (!m_store->seriesIsEmpty(i)) {
      minValue = std::min<double>(minValue, m_store->minValue(i));
      maxValue = std::max<double>(maxValue, m_store->maxValue(i));
    }
  }
  assert(xMin != nullptr);
  *xMin = minValue;
  m_store->setXMin(minValue);
  m_store->setXMax(maxValue);
}

void HistogramController::initRangeParameters() {
  assert(selectedSeriesIndex() >= 0 && m_store->sumOfOccurrences(selectedSeriesIndex()) > 0);
  double barWidth = m_store->barWidth();
  double xMin;
  preinitXRangeParameters(&xMin);
  double xMax = m_store->xMax() + barWidth;
  /* if a bar is represented by less than one pixel, we cap xMax */
  if ((xMax - xMin)/barWidth > k_maxNumberOfBarsPerWindow) {
    xMax = xMin + k_maxNumberOfBarsPerWindow*barWidth;
  }
  m_store->setXMin(xMin - Store::k_displayLeftMarginRatio*(xMax-xMin));
  m_store->setXMax(xMax + Store::k_displayRightMarginRatio*(xMax-xMin));

  initYRangeParameters(selectedSeriesIndex());
}

void HistogramController::initYRangeParameters(int series) {
  assert(series >= 0 && m_store->sumOfOccurrences(series) > 0);
  float yMax = -FLT_MAX;
  for (int index = 0; index < m_store->numberOfBars(series); index++) {
    float size = m_store->heightOfBarAtIndex(series, index);
    if (size > yMax) {
      yMax = size;
    }
  }
  yMax = yMax/m_store->sumOfOccurrences(series);
  yMax = yMax < 0 ? 1 : yMax;
  m_store->setYMax(yMax*(1.0f+Store::k_displayTopMarginRatio));

  /* Compute YMin:
   *    ratioFloatPixel*(0-yMin) = k_bottomMargin
   *    ratioFloatPixel*(yMax-yMin) = viewHeight
   *
   *    -ratioFloatPixel*yMin = k_bottomMargin
   *    ratioFloatPixel*yMax-ratioFloatPixel*yMin = viewHeight
   *
   *    ratioFloatPixel = (viewHeight - k_bottomMargin)/yMax
   *    yMin = -k_bottomMargin/ratioFloatPixel = yMax*k_bottomMargin/(k_bottomMargin - viewHeight)
   * */

  m_store->setYMin(m_store->yMax()*(float)Store::k_bottomMargin/((float)Store::k_bottomMargin - m_view.dataViewAtIndex(series)->bounds().height()));
}

void HistogramController::initBarParameters() {
  assert(selectedSeriesIndex() >= 0 && m_store->sumOfOccurrences(selectedSeriesIndex()) > 0);
  double xMin;
  preinitXRangeParameters(&xMin);
  m_store->setFirstDrawnBarAbscissa(xMin);
  double barWidth = m_store->xGridUnit();
  if (barWidth <= 0.0) {
    barWidth = 1.0;
  } else {
    // Truncate the bar width, as we convert from float to double
    const double precision = 7; // TODO factorize? This is an experimental value, the same as in Expression;;Epsilon<float>()
    const double logBarWidth = IEEE754<double>::exponentBase10(barWidth);
    barWidth = ((int)(barWidth * std::pow(10.0, precision - logBarWidth))) * std::pow(10.0, -precision + logBarWidth);
  }
  m_store->setBarWidth(barWidth);
}

void HistogramController::initBarSelection() {
  assert(selectedSeriesIndex() >= 0 && m_store->sumOfOccurrences(selectedSeriesIndex()) > 0);
  *m_selectedBarIndex = 0;
  while ((m_store->heightOfBarAtIndex(selectedSeriesIndex(), *m_selectedBarIndex) == 0 ||
      m_store->startOfBarAtIndex(selectedSeriesIndex(), *m_selectedBarIndex) < m_store->firstDrawnBarAbscissa()) && *m_selectedBarIndex < m_store->numberOfBars(selectedSeriesIndex())) {
    *m_selectedBarIndex = *m_selectedBarIndex+1;
  }
  if (*m_selectedBarIndex >= m_store->numberOfBars(selectedSeriesIndex())) {
    /* No bar is after m_firstDrawnBarAbscissa, so we select the first bar */
    *m_selectedBarIndex = 0;
    while (m_store->heightOfBarAtIndex(selectedSeriesIndex(), *m_selectedBarIndex) == 0 && *m_selectedBarIndex < m_store->numberOfBars(selectedSeriesIndex())) {
      *m_selectedBarIndex = *m_selectedBarIndex+1;
    }
  }
  m_store->scrollToSelectedBarIndex(selectedSeriesIndex(), *m_selectedBarIndex);
}

}
