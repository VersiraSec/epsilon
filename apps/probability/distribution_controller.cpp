#include "distribution_controller.h"
#include <assert.h>
#include <new>
#include "app.h"
#include "distribution/binomial_distribution.h"
#include "distribution/chi_squared_distribution.h"
#include "distribution/exponential_distribution.h"
#include "distribution/fisher_distribution.h"
#include "distribution/geometric_distribution.h"
#include "distribution/normal_distribution.h"
#include "distribution/poisson_distribution.h"
#include "distribution/student_distribution.h"
#include "distribution/uniform_distribution.h"
#include "images/binomial_icon.h"
#include "images/chi_squared_icon.h"
#include "images/exponential_icon.h"
#include "images/fisher_icon.h"
#include "images/geometric_icon.h"
#include "images/normal_icon.h"
#include "images/poisson_icon.h"
#include "images/student_icon.h"
#include "images/uniform_icon.h"
#include "images/focused_binomial_icon.h"
#include "images/focused_chi_squared_icon.h"
#include "images/focused_exponential_icon.h"
#include "images/focused_fisher_icon.h"
#include "images/focused_geometric_icon.h"
#include "images/focused_normal_icon.h"
#include "images/focused_poisson_icon.h"
#include "images/focused_student_icon.h"
#include "images/focused_uniform_icon.h"

using namespace Escher;

namespace Probability {

View * DistributionController::ContentView::subviewAtIndex(int index) {
  assert(index >= 0 && index < numberOfSubviews());
  switch (index) {
  case 0:
    return &m_titleView;
  case 1:
    return m_selectableTableView;
  default:
    return &m_borderView;
  }
}

void DistributionController::ContentView::layoutSubviews(bool force) {
  KDCoordinate titleHeight = KDFont::SmallFont->glyphSize().height() + k_titleMargin;
  m_titleView.setFrame(KDRect(0, 0, bounds().width(), titleHeight), force);
  m_selectableTableView->setFrame(KDRect(0, titleHeight, bounds().width(),  bounds().height() - titleHeight), force);
  m_borderView.setFrame(KDRect(m_selectableTableView->leftMargin(), titleHeight + m_selectableTableView->topMargin(), bounds().width() - m_selectableTableView->leftMargin() - m_selectableTableView->rightMargin(), Metric::CellSeparatorThickness), force);
}

DistributionController::DistributionController(Responder * parentResponder, Distribution * distribution, ParametersController * parametersController) :
  SelectableListViewController(parentResponder),
  m_contentView(&m_selectableTableView),
  m_distribution(distribution),
  m_parametersController(parametersController)
{
  assert(m_distribution != nullptr);
  m_selectableTableView.setTopMargin(0);
  // Fit m_selectableTableView scroll to content size
  m_selectableTableView.decorator()->setVerticalMargins(0, Metric::CommonBottomMargin);
}

void Probability::DistributionController::viewWillAppear() {
  ViewController::viewWillAppear();
  selectRow((int)m_distribution->type());
}

void Probability::DistributionController::didBecomeFirstResponder() {
  App::app()->snapshot()->setActivePage(App::Snapshot::Page::Distribution);
  if (selectedRow() == -1) {
    selectCellAtLocation(0, 0);
  } else {
    selectCellAtLocation(selectedColumn(), selectedRow());
  }
  Container::activeApp()->setFirstResponder(&m_selectableTableView);
}

bool Probability::DistributionController::handleEvent(Ion::Events::Event event) {
  if (event == Ion::Events::OK || event == Ion::Events::EXE || event == Ion::Events::Right) {
    StackViewController * stack = (StackViewController *)parentResponder();
    setDistributionAccordingToIndex(selectedRow());
    stack->push(m_parametersController, KDColorWhite, Palette::PurpleBright, Palette::PurpleBright);
    return true;
  }
  return false;
}

KDCoordinate Probability::DistributionController::nonMemoizedRowHeight(int j) {
  Cell tempCell;
  return heightForCellAtIndex(&tempCell, j);
}

HighlightCell * Probability::DistributionController::reusableCell(int index, int type) {
  assert(index >= 0);
  assert(index < reusableCellCount(type));
  return &m_cells[index];
}

constexpr I18n::Message sMessages[] = {
  I18n::Message::Binomial,
  I18n::Message::Uniforme,
  I18n::Message::Exponential,
  I18n::Message::Normal,
  I18n::Message::ChiSquared,
  I18n::Message::Student,
  I18n::Message::Geometric,
  I18n::Message::Poisson,
  I18n::Message::Fisher
};

void Probability::DistributionController::willDisplayCellForIndex(HighlightCell * cell, int index) {
  Cell * myCell = static_cast<Cell *>(cell);
  myCell->setLabel(sMessages[index]);
  const Image * images[k_totalNumberOfModels] = {
    ImageStore::BinomialIcon,
    ImageStore::UniformIcon,
    ImageStore::ExponentialIcon,
    ImageStore::NormalIcon,
    ImageStore::ChiSquaredIcon,
    ImageStore::StudentIcon,
    ImageStore::GeometricIcon,
    ImageStore::PoissonIcon,
    ImageStore::FisherIcon
  };
  const Image * focusedImages[k_totalNumberOfModels] = {
    ImageStore::FocusedBinomialIcon,
    ImageStore::FocusedUniformIcon,
    ImageStore::FocusedExponentialIcon,
    ImageStore::FocusedNormalIcon,
    ImageStore::FocusedChiSquaredIcon,
    ImageStore::FocusedStudentIcon,
    ImageStore::FocusedGeometricIcon,
    ImageStore::FocusedPoissonIcon,
    ImageStore::FocusedFisherIcon
  };
  myCell->setImage(images[index], focusedImages[index]);
  myCell->reloadCell();
}

void Probability::DistributionController::setDistributionAccordingToIndex(int index) {
  if ((int)m_distribution->type() == index) {
    return;
  }
  m_distribution->~Distribution();
  switch (index) {
    case 0:
      new(m_distribution) BinomialDistribution();
      break;
    case 1:
      new(m_distribution) UniformDistribution();
      break;
    case 2:
      new(m_distribution) ExponentialDistribution();
      break;
    case 3:
      new(m_distribution) NormalDistribution();
      break;
    case 4:
      new(m_distribution) ChiSquaredDistribution();
      break;
    case 5:
      new(m_distribution) StudentDistribution();
      break;
    case 6:
      new(m_distribution) GeometricDistribution();
      break;
    case 7:
      new(m_distribution) PoissonDistribution();
      break;
    case 8:
      new(m_distribution) FisherDistribution();
      break;
    default:
     return;
  }
  m_parametersController->reinitCalculation();
}

}
