#include <poincare_layouts.h>
#include "helper.h"

using namespace Poincare;

QUIZ_CASE(poincare_layout_constructors) {
  EmptyLayout e0 = EmptyLayout::Builder();
  AbsoluteValueLayout e1 = AbsoluteValueLayout::Builder(e0);
  CodePointLayout e2 = CodePointLayout::Builder('a');
  BinomialCoefficientLayout e3 = BinomialCoefficientLayout::Builder(e1, e2);
  CeilingLayout e4 = CeilingLayout::Builder(e3);
  RightParenthesisLayout e5 = RightParenthesisLayout::Builder();
  RightSquareBracketLayout e6 = RightSquareBracketLayout::Builder();
  CondensedSumLayout e7 = CondensedSumLayout::Builder(e4, e5, e6);
  ConjugateLayout e8 = ConjugateLayout::Builder(e7);
  LeftParenthesisLayout e10 = LeftParenthesisLayout::Builder();
  FloorLayout e11 = FloorLayout::Builder(e10);
  FractionLayout e12 = FractionLayout::Builder(e8, e11);
  HorizontalLayout e13 = HorizontalLayout::Builder();
  LeftSquareBracketLayout e14 = LeftSquareBracketLayout::Builder();
  IntegralLayout e15 = IntegralLayout::Builder(e11, e12, e13, e14);
  NthRootLayout e16 = NthRootLayout::Builder(e15);
  MatrixLayout e17 = MatrixLayout::Builder();
  EmptyLayout e18 = EmptyLayout::Builder();
  EmptyLayout e19 = EmptyLayout::Builder();
  EmptyLayout e20 = EmptyLayout::Builder();
  ProductLayout e21 = ProductLayout::Builder(e17, e18, e19, e20);
  EmptyLayout e22 = EmptyLayout::Builder();
  EmptyLayout e23 = EmptyLayout::Builder();
  EmptyLayout e24 = EmptyLayout::Builder();
  SumLayout e25 = SumLayout::Builder(e21, e22, e23, e24);
  VerticalOffsetLayout e26 = VerticalOffsetLayout::Builder(e25, VerticalOffsetLayoutNode::Position::Superscript);
  CodePointLayout e27 = CodePointLayout::Builder('t');
  DerivativeLayout e28 = DerivativeLayout::Builder(e15, e27, e26);
}

QUIZ_CASE(poincare_layout_comparison) {
  Layout e0 = CodePointLayout::Builder('a');
  Layout e1 = CodePointLayout::Builder('a');
  Layout e2 = CodePointLayout::Builder('b');
  quiz_assert(e0.isIdenticalTo(e1));
  quiz_assert(!e0.isIdenticalTo(e2));

  Layout e3 = EmptyLayout::Builder();
  Layout e4 = EmptyLayout::Builder();
  quiz_assert(e3.isIdenticalTo(e4));
  quiz_assert(!e3.isIdenticalTo(e0));

  Layout e5 = NthRootLayout::Builder(e0);
  Layout e6 = NthRootLayout::Builder(e1);
  Layout e7 = NthRootLayout::Builder(e2);
  quiz_assert(e5.isIdenticalTo(e6));
  quiz_assert(!e5.isIdenticalTo(e7));
  quiz_assert(!e5.isIdenticalTo(e0));

  Layout e8 = VerticalOffsetLayout::Builder(e5, VerticalOffsetLayoutNode::Position::Superscript);
  Layout e9 = VerticalOffsetLayout::Builder(e6, VerticalOffsetLayoutNode::Position::Superscript);
  Layout e10 = VerticalOffsetLayout::Builder(NthRootLayout::Builder(CodePointLayout::Builder('a')), VerticalOffsetLayoutNode::Position::Subscript);
  quiz_assert(e8.isIdenticalTo(e9));
  quiz_assert(!e8.isIdenticalTo(e10));
  quiz_assert(!e8.isIdenticalTo(e0));

  Layout e11 = SumLayout::Builder(e0, e3, e6, e2);
  Layout e12 = SumLayout::Builder(CodePointLayout::Builder('a'), EmptyLayout::Builder(), NthRootLayout::Builder(CodePointLayout::Builder('a')), CodePointLayout::Builder('b'));
  Layout e13 = ProductLayout::Builder(CodePointLayout::Builder('a'), EmptyLayout::Builder(), NthRootLayout::Builder(CodePointLayout::Builder('a')), CodePointLayout::Builder('b'));
  quiz_assert(e11.isIdenticalTo(e12));
  quiz_assert(!e11.isIdenticalTo(e13));
}

QUIZ_CASE(poincare_layout_fraction_create) {

  /*                         12
   * 12|34+5 -> "Divide" -> --- + 5
   *                        |34
   * */
  Layout layout = LayoutHelper::String("1234+5", 6);
  LayoutCursor cursor(layout.childAtIndex(2), LayoutCursor::Position::Left);
  cursor.addFractionLayoutAndCollapseSiblings();
  assert_layout_serialize_to(layout, "\u0012\u001212\u0013/\u001234\u0013\u0013+5");
  quiz_assert(cursor.isEquivalentTo(LayoutCursor(layout.childAtIndex(0).childAtIndex(1), LayoutCursor::Position::Left)));

  /*
   *  1                      1   3
   * --- 3|4 -> "Divide" -> --- ---
   *  2                      2   4
   * */
  Layout l1 = HorizontalLayout::Builder(
      FractionLayout::Builder(
        HorizontalLayout::Builder(CodePointLayout::Builder('1')),
        HorizontalLayout::Builder(CodePointLayout::Builder('2'))),
      CodePointLayout::Builder('3'),
      CodePointLayout::Builder('4'));
  LayoutCursor c1(l1.childAtIndex(2), LayoutCursor::Position::Left);
  c1.addFractionLayoutAndCollapseSiblings();
  assert_layout_serialize_to(l1, "\u0012\u00121\u0013/\u00122\u0013\u0013\u0012\u00123\u0013/\u00124\u0013\u0013");

  /*
   *                                sin(x)cos(x)
   * sin(x)cos(x)|2 -> "Divide" -> --------------
   *                                     2
   * */

  Layout l2 = LayoutHelper::String("sin(x)cos(x)2", 13);
  LayoutCursor c2(l2.childAtIndex(12), LayoutCursor::Position::Left);
  c2.addFractionLayoutAndCollapseSiblings();
  assert_layout_serialize_to(l2, "\u0012\u0012sin(x)cos(x)\u0013/\u00122\u0013\u0013");
}

QUIZ_CASE(poincare_layout_power) {
  /*
   *                      2|
   * 12| -> "Square" -> 12 |
   *
   * */
  Layout l1 = LayoutHelper::String("12", 2);
  LayoutCursor c1(l1.childAtIndex(1), LayoutCursor::Position::Right);
  c1.addEmptySquarePowerLayout();
  assert_layout_serialize_to(l1, "12^\u00122\u0013");

  /*                        2|
   *  2|                ( 2) |
   * 1 | -> "Square" -> (1 ) |
   *
   * */
  Layout l2 = HorizontalLayout::Builder(
      CodePointLayout::Builder('1'),
      VerticalOffsetLayout::Builder(CodePointLayout::Builder('2'), VerticalOffsetLayoutNode::Position::Superscript));
  LayoutCursor c2(l2.childAtIndex(1), LayoutCursor::Position::Right);
  c2.addEmptySquarePowerLayout();
  assert_layout_serialize_to(l2, "(1^\u00122\u0013)^\u00122\u0013");

  /*                             (    2|)
   * ( 2)|                       (( 2) |)
   * (1 )| -> "Left" "Square" -> ((1 ) |)
   * */
  Layout l3 = HorizontalLayout::Builder(
      LeftParenthesisLayout::Builder(),
      CodePointLayout::Builder('1'),
      VerticalOffsetLayout::Builder(CodePointLayout::Builder('2'), VerticalOffsetLayoutNode::Position::Superscript),
      RightParenthesisLayout::Builder());
  LayoutCursor c3(l3.childAtIndex(3), LayoutCursor::Position::Right);
  c3.moveLeft(nullptr);
  c3.addEmptySquarePowerLayout();
  assert_layout_serialize_to(l3, "((1^\u00122\u0013)^\u00122\u0013)");
}

QUIZ_CASE(poincare_layout_parentheses_size) {
  /*      3
   * (2+(---)6)1
   *      4
   * Assert that the first and last parentheses have the same size.
   */
  HorizontalLayout layout = HorizontalLayout::Builder();
  LeftParenthesisLayout leftPar = LeftParenthesisLayout::Builder();
  RightParenthesisLayout rightPar = RightParenthesisLayout::Builder();
  layout.addChildAtIndex(leftPar, 0, 0, nullptr);
  layout.addChildAtIndex(CodePointLayout::Builder('2'), 1, 1, nullptr);
  layout.addChildAtIndex(CodePointLayout::Builder('+'), 2, 2, nullptr);
  layout.addChildAtIndex(LeftParenthesisLayout::Builder(), 3, 3, nullptr);
  layout.addChildAtIndex(FractionLayout::Builder(
        CodePointLayout::Builder('3'),
        CodePointLayout::Builder('4')),
      4, 4, nullptr);
  layout.addChildAtIndex(RightParenthesisLayout::Builder(), 4, 4, nullptr);
  layout.addChildAtIndex(CodePointLayout::Builder('6'), 5, 5, nullptr);
  layout.addChildAtIndex(rightPar, 7, 7, nullptr);
  layout.addChildAtIndex(CodePointLayout::Builder('1'), 8, 8, nullptr);
  quiz_assert(leftPar.layoutSize().height() == rightPar.layoutSize().height());
}
