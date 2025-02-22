#include "continuous_function.h"
#include "poincare_helpers.h"
#include <apps/constant.h>
#include <poincare/derivative.h>
#include <poincare/integral.h>
#include <poincare/matrix.h>
#include <poincare/multiplication.h>
#include <poincare/rational.h>
#include <poincare/serialization_helper.h>
#include <poincare/trigonometry.h>
#include <escher/palette.h>
#include <ion/unicode/utf8_helper.h>
#include <ion/unicode/utf8_decoder.h>
#include <apps/i18n.h>
#include <float.h>
#include <cmath>
#include <algorithm>

using namespace Poincare;
using namespace Escher;

namespace Shared {

void ContinuousFunction::DefaultName(char buffer[], size_t bufferSize) {
  constexpr int k_maxNumberOfDefaultLetterNames = 4;
  constexpr char k_defaultLetterNames[k_maxNumberOfDefaultLetterNames] = {
    'f', 'g', 'h', 'p'
  };
  /* First default names are f, g, h, p and then f0, f1... ie, "f[number]",
   * for instance "f12", that does not exist yet in the storage. */
  size_t constantNameLength = 1; // 'f', no null-terminating char
  assert(bufferSize > constantNameLength+1);
  // Find the next available name
  int currentNumber = -k_maxNumberOfDefaultLetterNames;
  int currentNumberLength = 0;
  int availableBufferSize = bufferSize - constantNameLength;
  while (currentNumberLength < availableBufferSize) {
    // Choose letter
    buffer[0] = currentNumber < 0 ? k_defaultLetterNames[k_maxNumberOfDefaultLetterNames+currentNumber] : k_defaultLetterNames[0];
    // Choose number if required
    if (currentNumber >= 0) {
      currentNumberLength = Poincare::Integer(currentNumber).serialize(&buffer[1], availableBufferSize);
    } else {
      buffer[1] = 0;
    }
    if (GlobalContext::SymbolAbstractNameIsFree(buffer)) {
      // Name found
      break;
    }
    currentNumber++;
  }
  assert(currentNumberLength >= 0 && currentNumberLength < availableBufferSize);
}

ContinuousFunction ContinuousFunction::NewModel(Ion::Storage::Record::ErrorStatus * error, const char * baseName) {
  static int s_colorIndex = 0;
  // Create the record
  char nameBuffer[SymbolAbstract::k_maxNameSize];
  RecordDataBuffer data(Palette::nextDataColor(&s_colorIndex));
  if (baseName == nullptr) {
    DefaultName(nameBuffer, SymbolAbstract::k_maxNameSize);
    baseName = nameBuffer;
  }
  *error = Ion::Storage::sharedStorage()->createRecordWithExtension(baseName, Ion::Storage::funcExtension, &data, sizeof(data));

  // Return if error
  if (*error != Ion::Storage::Record::ErrorStatus::None) {
    return ContinuousFunction();
  }

  // Return the ContinuousFunction with the new record
  return ContinuousFunction(Ion::Storage::sharedStorage()->recordBaseNamedWithExtension(baseName, Ion::Storage::funcExtension));
}

int ContinuousFunction::derivativeNameWithArgument(char * buffer, size_t bufferSize) {
  // Fill buffer with f(x). Keep size for derivative sign.
  int derivativeSize = UTF8Decoder::CharSizeOfCodePoint('\'');
  int numberOfChars = nameWithArgument(buffer, bufferSize - derivativeSize);
  assert(numberOfChars + derivativeSize < (int)bufferSize);
  char * firstParenthesis = const_cast<char *>(UTF8Helper::CodePointSearch(buffer, '('));
  if (!UTF8Helper::CodePointIs(firstParenthesis, '(')) {
    return numberOfChars;
  }
  memmove(firstParenthesis + derivativeSize, firstParenthesis, numberOfChars - (firstParenthesis - buffer) + 1);
  UTF8Decoder::CodePointToChars('\'', firstParenthesis, derivativeSize);
  return numberOfChars + derivativeSize;
}

Poincare::Expression ContinuousFunction::expressionReduced(Poincare::Context * context) const {
  Poincare::Expression result = ExpressionModelHandle::expressionReduced(context);
  if (plotType() == PlotType::Parametric) {
    Expression trueExpression = result.type() == ExpressionNode::Type::Dependency ? result.childAtIndex(0) : result;
    if (trueExpression.type() != Poincare::ExpressionNode::Type::Matrix
     || static_cast<Poincare::Matrix&>(trueExpression).numberOfRows() != 2
     || static_cast<Poincare::Matrix&>(trueExpression).numberOfColumns() != 1)
    {
      return Poincare::Expression::Parse("[[undef][undef]]", nullptr);
    }
  }
  return result;
}

I18n::Message ContinuousFunction::parameterMessageName() const {
  return ParameterMessageForPlotType(plotType());
}

CodePoint ContinuousFunction::symbol() const {
  switch (plotType()) {
  case PlotType::Cartesian:
    return 'x';
  case PlotType::Polar:
    return UCodePointGreekSmallLetterTheta;
  default:
    assert(plotType() == PlotType::Parametric);
    return 't';
  }
}

ContinuousFunction::PlotType ContinuousFunction::plotType() const {
  return recordData()->plotType();
}

void ContinuousFunction::setPlotType(PlotType newPlotType, Poincare::Preferences::AngleUnit angleUnit, Context * context) {
  PlotType currentPlotType = plotType();
  if (newPlotType == currentPlotType) {
    return;
  }

  recordData()->setPlotType(newPlotType);

  setCache(nullptr);

  // Recompute the layouts
  m_model.tidy();

  // Recompute the definition domain
  double tMin = newPlotType == PlotType::Cartesian ? -INFINITY : 0.0;
  double tMax = newPlotType == PlotType::Cartesian ? INFINITY : 2.0*Trigonometry::PiInAngleUnit(angleUnit);
  setTMin(tMin);
  setTMax(tMax);

  /* Recompute the unknowns. For instance, if the function was f(x) = xθ, it is
   * stored as f(?) = ?θ. When switching to polar type, it should be stored as
   * f(?) = ?? */
  constexpr int previousTextContentMaxSize = Constant::MaxSerializedExpressionSize;
  char previousTextContent[previousTextContentMaxSize];
  m_model.text(this, previousTextContent, previousTextContentMaxSize, symbol());

  Ion::Storage::Record::ErrorStatus error = setContent(previousTextContent, context);
  // Handle parametric function switch
  if (currentPlotType == PlotType::Parametric && error == Ion::Storage::Record::ErrorStatus::None) {
    Expression e = expressionClone();
    if (e.isUninitialized()
        || e.type() != ExpressionNode::Type::Matrix
        || static_cast<Poincare::Matrix&>(e).numberOfRows() != 2
        || static_cast<Poincare::Matrix&>(e).numberOfColumns() != 1) {
      return;
    }
    // Change [x(t) y(t)] to y(t)
    Expression nextContent = e.childAtIndex(1);
    /* We need to detach it, otherwise nextContent will think it has a parent
     * when we retrieve it from the storage. */
    nextContent.detachFromParent();
    error = setExpressionContent(nextContent);
  } else if (newPlotType == PlotType::Parametric && error == Ion::Storage::Record::ErrorStatus::None) {
    Expression e = expressionClone();
    // Change y(t) to [t y(t)]
    Matrix newExpr = Matrix::Builder();
    newExpr.addChildAtIndexInPlace(Symbol::Builder(UCodePointUnknown), 0, 0);
    // if y(t) was not uninitialized, insert [t 2t] to set an example
    e = e.isUninitialized() ? Multiplication::Builder(Rational::Builder(2), Symbol::Builder(UCodePointUnknown)) : e;
    newExpr.addChildAtIndexInPlace(e, newExpr.numberOfChildren(), newExpr.numberOfChildren());
    newExpr.setDimensions(2, 1);
    error = setExpressionContent(newExpr);
  }
  if (error != Ion::Storage::Record::ErrorStatus::None) {
    // An error occurred, reset plot type to the initial one.
    recordData()->setPlotType(currentPlotType);
  }
}

I18n::Message ContinuousFunction::ParameterMessageForPlotType(PlotType plotType) {
  if (plotType == PlotType::Cartesian) {
    return I18n::Message::X;
  }
  if (plotType == PlotType::Polar) {
    return I18n::Message::Theta;
  }
  assert(plotType == PlotType::Parametric);
  return I18n::Message::T;
}

template <typename T>
Poincare::Coordinate2D<T> ContinuousFunction::privateEvaluateXYAtParameter(T t, Poincare::Context * context) const {
  Coordinate2D<T> x1x2 = templatedApproximateAtParameter(t, context);
  PlotType type = plotType();
  if (type == PlotType::Cartesian || type == PlotType::Parametric) {
    return x1x2;
  }
  assert(type == PlotType::Polar);
  T factor = (T)1.0;
  Preferences::AngleUnit angleUnit = Preferences::sharedPreferences()->angleUnit();
  if (angleUnit == Preferences::AngleUnit::Degree) {
    factor = (T) (M_PI/180.0);
  } else if (angleUnit == Preferences::AngleUnit::Gradian) {
    factor = (T) (M_PI/200.0);
  } else {
    assert(angleUnit == Preferences::AngleUnit::Radian);
  }
  const float angle = x1x2.x1()*factor;
  return Coordinate2D<T>(x1x2.x2() * std::cos(angle), x1x2.x2() * std::sin(angle));
}

bool ContinuousFunction::displayDerivative() const {
  return recordData()->displayDerivative();
}

void ContinuousFunction::setDisplayDerivative(bool display) {
  return recordData()->setDisplayDerivative(display);
}

int ContinuousFunction::printValue(double cursorT, double cursorX, double cursorY, char * buffer, int bufferSize, int precision, Poincare::Context * context) {
  PlotType type = plotType();
  if (type == PlotType::Cartesian) {
    return Function::printValue(cursorT, cursorX, cursorY, buffer, bufferSize, precision, context);
  }
  if (type == PlotType::Polar) {
    return PoincareHelpers::ConvertFloatToText<double>(evaluate2DAtParameter(cursorT, context).x2(), buffer, bufferSize, precision);
  }
  assert(type == PlotType::Parametric);
  int result = 0;
  result += UTF8Decoder::CodePointToChars('(', buffer+result, bufferSize-result);
  result += PoincareHelpers::ConvertFloatToText<double>(cursorX, buffer+result, bufferSize-result, precision);
  result += UTF8Decoder::CodePointToChars(';', buffer+result, bufferSize-result);
  result += PoincareHelpers::ConvertFloatToText<double>(cursorY, buffer+result, bufferSize-result, precision);
  result += UTF8Decoder::CodePointToChars(')', buffer+result, bufferSize-result);
  return result;
}

double ContinuousFunction::approximateDerivative(double x, Poincare::Context * context) const {
  assert(plotType() == PlotType::Cartesian);
  if (x < tMin() || x > tMax()) {
    return NAN;
  }
  constexpr int bufferSize = CodePoint::MaxCodePointCharLength + 1;
  char unknown[bufferSize];
  Poincare::SerializationHelper::CodePoint(unknown, bufferSize, UCodePointUnknown);
  // Derivative is simplified once and for all
  return PoincareHelpers::ApproximateWithValueForSymbol(expressionDerivateReduced(context), unknown, x, context);
}

float ContinuousFunction::tMin() const {
  return recordData()->tMin();
}

float ContinuousFunction::tMax() const {
  return recordData()->tMax();
}

void ContinuousFunction::setTMin(float tMin) {
  recordData()->setTMin(tMin);
  setCache(nullptr);
}

void ContinuousFunction::setTMax(float tMax) {
  recordData()->setTMax(tMax);
  setCache(nullptr);
}

void ContinuousFunction::xRangeForDisplay(float xMinLimit, float xMaxLimit, float * xMin, float * xMax, float * yMinIntrinsic, float * yMaxIntrinsic, Poincare::Context * context) const {
  if (plotType() != PlotType::Cartesian) {
    assert(std::isfinite(tMin()) && std::isfinite(tMax()) && std::isfinite(rangeStep()) && rangeStep() > 0);
    protectedFullRangeForDisplay(tMin(), tMax(), rangeStep(), xMin, xMax, context, true);
    *yMinIntrinsic = FLT_MAX;
    *yMaxIntrinsic = -FLT_MAX;
    return;
  }

  *xMin = NAN;
  *xMax = NAN;
  *yMinIntrinsic = NAN;
  *yMaxIntrinsic = NAN;

  if (basedOnCostlyAlgorithms(context)) {
    /* The function makes use of some costly algorithms, such as integration or
     * sequences, and cannot be computed in a timely manner. */
    return;
  }

  Zoom::ValueAtAbscissa evaluation = [](float x, Context * context, const void * auxiliary) {
    /* When evaluating sin(x)/x close to zero using the standard sine function,
     * one can detect small variations, while the cardinal sine is supposed to
     * be locally monotonous. To smooth out such variations, we round the
     * result of the evaluations. As we are not interested in precise results
     * but only in ordering, this approximation is sufficient. */
    constexpr float precision = 1e-5;
    return precision * std::round(static_cast<const Function *>(auxiliary)->evaluateXYAtParameter(x, context).x2() / precision);
  };
  Zoom::InterestingRangesForDisplay(evaluation, xMin, xMax, yMinIntrinsic, yMaxIntrinsic, std::max(tMin(), xMinLimit), std::min(tMax(), xMaxLimit), context, this);
}

void ContinuousFunction::yRangeForDisplay(float xMin, float xMax, float yMinForced, float yMaxForced, float ratio, float * yMin, float * yMax, Poincare::Context * context, bool optimizeRange) const {
  if (plotType() != PlotType::Cartesian) {
    assert(std::isfinite(tMin()) && std::isfinite(tMax()) && std::isfinite(rangeStep()) && rangeStep() > 0);
    protectedFullRangeForDisplay(tMin(), tMax(), rangeStep(), yMin, yMax, context, false);
    return;
  }

  *yMin = NAN;
  *yMax = NAN;

  if (basedOnCostlyAlgorithms(context)) {
    /* The function makes use of some costly algorithms, such as integration or
     * sequences, and cannot be computed in a timely manner. */
    return;
  }

  if (!optimizeRange) {
    protectedFullRangeForDisplay(xMin, xMax, (xMax - xMin) / (Ion::Display::Width / 4), yMin, yMax, context, false);
    return;
  }

  Zoom::ValueAtAbscissa evaluation = [](float x, Context * context, const void * auxiliary) {
    return static_cast<const Function *>(auxiliary)->evaluateXYAtParameter(x, context).x2();
  };

  if (yMaxForced - yMinForced <= ratio * (xMax - xMin)) {
    Zoom::RangeWithRatioForDisplay(evaluation, ratio, xMin, xMax, yMinForced, yMaxForced, yMin, yMax, context, this);
    if (*yMin < *yMax) {
      return;
    }
  }

  *yMin = NAN;
  *yMax = NAN;

  Zoom::RefinedYRangeForDisplay(evaluation, xMin, xMax, yMin, yMax, context, this);
}

void ContinuousFunction::Model::tidy() const {
  m_expressionDerivate = Expression();
  ExpressionModel::tidy();
}

void * ContinuousFunction::Model::expressionAddress(const Ion::Storage::Record * record) const {
  return (char *)record->value().buffer+sizeof(RecordDataBuffer);
}

size_t ContinuousFunction::Model::expressionSize(const Ion::Storage::Record * record) const {
  return record->value().size-sizeof(RecordDataBuffer);
}

Expression ContinuousFunction::Model::expressionDerivateReduced(const Ion::Storage::Record * record, Poincare::Context * context) const {
  if (m_expressionDerivate.isUninitialized()) {
    m_expressionDerivate = Poincare::Derivative::Builder(expressionReduced(record, context).clone(), Symbol::Builder(UCodePointUnknown), Symbol::Builder(UCodePointUnknown));
    /* On complex functions, this step can take a significant time.
     * A workaround could be to identify big functions to skip simplification at
     * the cost of possible inaccurate evaluations (such as diff(abs(x),x,0) not
     * being undefined). */
    PoincareHelpers::Simplify(&m_expressionDerivate, context, ExpressionNode::ReductionTarget::SystemForApproximation);
    // simplify might return an uninitialized Expression if interrupted
    if (m_expressionDerivate.isUninitialized()) {
      m_expressionDerivate = Poincare::Derivative::Builder(expressionReduced(record, context).clone(), Symbol::Builder(UCodePointUnknown), Symbol::Builder(UCodePointUnknown));
    }
  }
  return m_expressionDerivate;
}

ContinuousFunction::RecordDataBuffer * ContinuousFunction::recordData() const {
  assert(!isNull());
  Ion::Storage::Record::Data d = value();
  return reinterpret_cast<RecordDataBuffer *>(const_cast<void *>(d.buffer));
}

template<typename T>
Coordinate2D<T> ContinuousFunction::templatedApproximateAtParameter(T t, Poincare::Context * context) const {
  if (t < tMin() || t > tMax()) {
    return Coordinate2D<T>(plotType() == PlotType::Cartesian ? t : NAN, NAN);
  }
  constexpr int bufferSize = CodePoint::MaxCodePointCharLength + 1;
  char unknown[bufferSize];
  Poincare::SerializationHelper::CodePoint(unknown, bufferSize, UCodePointUnknown);
  PlotType type = plotType();
  Expression e = expressionReduced(context);
  if (type != PlotType::Parametric) {
    assert(type == PlotType::Cartesian || type == PlotType::Polar);
    return Coordinate2D<T>(t, PoincareHelpers::ApproximateWithValueForSymbol(e, unknown, t, context));
  }
  if (e.type() == ExpressionNode::Type::Dependency) {
    e = e.childAtIndex(0);
  }
  assert(e.type() == ExpressionNode::Type::Matrix);
  assert(static_cast<Poincare::Matrix&>(e).numberOfRows() == 2);
  assert(static_cast<Poincare::Matrix&>(e).numberOfColumns() == 1);
  return Coordinate2D<T>(
      PoincareHelpers::ApproximateWithValueForSymbol(e.childAtIndex(0), unknown, t, context),
      PoincareHelpers::ApproximateWithValueForSymbol(e.childAtIndex(1), unknown, t, context));
}

Coordinate2D<double> ContinuousFunction::nextMinimumFrom(double start, double max, Context * context, double relativePrecision, double minimalStep, double maximalStep) const {
  return nextPointOfInterestFrom(start, max, context, [](Expression e, char * symbol, double start, double max, Context * context, double relativePrecision, double minimalStep, double maximalStep) { return PoincareHelpers::NextMinimum(e, symbol, start, max, context, relativePrecision, minimalStep, maximalStep); }, relativePrecision, minimalStep, maximalStep);
}

Coordinate2D<double> ContinuousFunction::nextMaximumFrom(double start, double max, Context * context, double relativePrecision, double minimalStep, double maximalStep) const {
  return nextPointOfInterestFrom(start, max, context, [](Expression e, char * symbol, double start, double max, Context * context, double relativePrecision, double minimalStep, double maximalStep) { return PoincareHelpers::NextMaximum(e, symbol, start, max, context, relativePrecision, minimalStep, maximalStep); }, relativePrecision, minimalStep, maximalStep);
}

Coordinate2D<double> ContinuousFunction::nextRootFrom(double start, double max, Context * context, double relativePrecision, double minimalStep, double maximalStep) const {
  return nextPointOfInterestFrom(start, max, context, [](Expression e, char * symbol, double start, double max, Context * context, double relativePrecision, double minimalStep, double maximalStep) { return Coordinate2D<double>(PoincareHelpers::NextRoot(e, symbol, start, max, context, relativePrecision, minimalStep, maximalStep), 0.0); }, relativePrecision, minimalStep, maximalStep);
}

Coordinate2D<double> ContinuousFunction::nextIntersectionFrom(double start, double max, Poincare::Context * context, Poincare::Expression e, double relativePrecision, double minimalStep, double maximalStep, double eDomainMin, double eDomainMax) const {
  assert(plotType() == PlotType::Cartesian);
  constexpr int bufferSize = CodePoint::MaxCodePointCharLength + 1;
  char unknownX[bufferSize];
  SerializationHelper::CodePoint(unknownX, bufferSize, UCodePointUnknown);
  double tmin = std::max<double>(tMin(), eDomainMin), tmax = std::min<double>(tMax(), eDomainMax);
  start = start < tmin ? tmin : start > tmax ? tmax : start;
  max = max < tmin ? tmin : max > tmax ? tmax : max;
  if (start == max) {
    return NAN;
  }
  return PoincareHelpers::NextIntersection(expressionReduced(context), unknownX, start, max, context, e, relativePrecision, minimalStep, maximalStep);
}

Coordinate2D<double> ContinuousFunction::nextPointOfInterestFrom(double start, double max, Context * context, ComputePointOfInterest compute, double relativePrecision, double minimalStep, double maximalStep) const {
  assert(plotType() == PlotType::Cartesian);
  constexpr int bufferSize = CodePoint::MaxCodePointCharLength + 1;
  char unknownX[bufferSize];
  SerializationHelper::CodePoint(unknownX, bufferSize, UCodePointUnknown);
  double tmin = tMin(), tmax = tMax();
  start = start < tmin ? tmin : start > tmax ? tmax : start;
  max = max < tmin ? tmin : max > tmax ? tmax : max;
  if (start == max) {
    return NAN;
  }
  return compute(expressionReduced(context), unknownX, start, max, context, relativePrecision, minimalStep, maximalStep);
}

Poincare::Expression ContinuousFunction::sumBetweenBounds(double start, double end, Poincare::Context * context) const {
  assert(plotType() == PlotType::Cartesian);
  start = std::max<double>(start, tMin());
  end = std::min<double>(end, tMax());
  return Poincare::Integral::Builder(expressionReduced(context).clone(), Poincare::Symbol::Builder(UCodePointUnknown), Poincare::Float<double>::Builder(start), Poincare::Float<double>::Builder(end)); // Integral takes ownership of args
  /* TODO: when we approximate integral, we might want to simplify the integral
   * here. However, we might want to do it once for all x (to avoid lagging in
   * the derivative table. */
}

Ion::Storage::Record::ErrorStatus ContinuousFunction::setContent(const char * c, Poincare::Context * context) {
  setCache(nullptr);
  return ExpressionModelHandle::setContent(c, context);
}

bool ContinuousFunction::basedOnCostlyAlgorithms(Context * context) const {
  return expressionReduced(context).hasExpression([](const Expression e, const void * context) {
      return e.type() == ExpressionNode::Type::Sequence
          || e.type() == ExpressionNode::Type::Integral
          || e.type() == ExpressionNode::Type::Derivative;
      }, nullptr);
}

template Coordinate2D<float> ContinuousFunction::templatedApproximateAtParameter<float>(float, Poincare::Context *) const;
template Coordinate2D<double> ContinuousFunction::templatedApproximateAtParameter<double>(double, Poincare::Context *) const;

template Poincare::Coordinate2D<float> ContinuousFunction::privateEvaluateXYAtParameter<float>(float, Poincare::Context *) const;
template Poincare::Coordinate2D<double> ContinuousFunction::privateEvaluateXYAtParameter<double>(double, Poincare::Context *) const;

}
