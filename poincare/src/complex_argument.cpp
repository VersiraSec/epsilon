#include <poincare/complex_argument.h>
#include <poincare/complex_cartesian.h>
#include <poincare/constant.h>
#include <poincare/layout_helper.h>
#include <poincare/rational.h>
#include <poincare/serialization_helper.h>
#include <poincare/simplification_helper.h>
#include <poincare/undefined.h>
extern "C" {
#include <assert.h>
}
#include <cmath>

namespace Poincare {

constexpr Expression::FunctionHelper ComplexArgument::s_functionHelper;

int ComplexArgumentNode::numberOfChildren() const { return ComplexArgument::s_functionHelper.numberOfChildren(); }

Layout ComplexArgumentNode::createLayout(Preferences::PrintFloatMode floatDisplayMode, int numberOfSignificantDigits) const {
  return LayoutHelper::Prefix(ComplexArgument(this), floatDisplayMode, numberOfSignificantDigits, ComplexArgument::s_functionHelper.name());
}

int ComplexArgumentNode::serialize(char * buffer, int bufferSize, Preferences::PrintFloatMode floatDisplayMode, int numberOfSignificantDigits) const {
  return SerializationHelper::Prefix(this, buffer, bufferSize, floatDisplayMode, numberOfSignificantDigits, ComplexArgument::s_functionHelper.name());
}

Expression ComplexArgumentNode::shallowReduce(ReductionContext reductionContext) {
  return ComplexArgument(this).shallowReduce(reductionContext);
}

template<typename T>
Complex<T> ComplexArgumentNode::computeOnComplex(const std::complex<T> c, Preferences::ComplexFormat, Preferences::AngleUnit angleUnit) {
  return Complex<T>::Builder(std::arg(c));
}

Expression ComplexArgument::shallowReduce(ExpressionNode::ReductionContext reductionContext) {
  {
    Expression e = SimplificationHelper::defaultShallowReduce(*this);
    if (!e.isUninitialized()) {
      return e;
    }
  }
  Expression c = childAtIndex(0);
  if (c.type() == ExpressionNode::Type::Matrix) {
    return mapOnMatrixFirstChild(reductionContext);
  }
  if (c.deepIsMatrix(reductionContext.context())) {
    return *this;
  }

  if (c.type() == ExpressionNode::Type::ComplexCartesian) {
    ComplexCartesian complexChild = static_cast<ComplexCartesian &>(c);
    Expression childArg = complexChild.argument(reductionContext);
    replaceWithInPlace(childArg);
    return childArg.shallowReduce(reductionContext);
  }
  Context * context = reductionContext.context();
  Expression res;
  if (c.nullStatus(context) == ExpressionNode::NullStatus::Null) {
    res = Undefined::Builder();
  } else if (c.sign(context) == ExpressionNode::Sign::Positive) {
    res = Rational::Builder(0);
  } else if (c.sign(context) == ExpressionNode::Sign::Negative) {
    res = Constant::Builder(UCodePointGreekSmallLetterPi);
  } else {
    double approximation = c.approximateToScalar<double>(context, reductionContext.complexFormat(), reductionContext.angleUnit(), true);
    if (approximation < 0.0) {
      res = Constant::Builder(UCodePointGreekSmallLetterPi);
    } else if (approximation > 0.0) {
      res = Rational::Builder(0);
    } else if (approximation == 0.0) {
      res = Undefined::Builder();
    } // else, approximation is NaN
  }
  if (res.isUninitialized()) {
    return *this;
  } else {
    replaceWithInPlace(res);
    return res;
  }
}

}
