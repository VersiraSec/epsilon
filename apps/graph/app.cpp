#include "app.h"
#include "../apps_container.h"
#include "graph_icon.h"
#include <apps/i18n.h>

using namespace Poincare;
using namespace Shared;
using namespace Escher;

namespace Graph {

I18n::Message App::Descriptor::name() const {
  return I18n::Message::FunctionApp;
}

I18n::Message App::Descriptor::upperName() const {
  return I18n::Message::FunctionAppCapital;
}

const Image * App::Descriptor::icon() const {
  return ImageStore::GraphIcon;
}

App::Snapshot::Snapshot() :
  Shared::FunctionApp::Snapshot::Snapshot(),
  m_functionStore(),
  m_graphRange()
{
}

App * App::Snapshot::unpack(Container * container) {
  return new (container->currentAppBuffer()) App(this);
}

void App::Snapshot::reset() {
  Shared::FunctionApp::Snapshot::reset();
  for (int i = 0; i < Shared::ContinuousFunction::k_numberOfPlotTypes; i++) {
    m_interval[i].reset();
  }
}

static constexpr App::Descriptor sDescriptor;

const App::Descriptor * App::Snapshot::descriptor() const {
  return &sDescriptor;
}

void App::Snapshot::tidy() {
  m_functionStore.tidy();
  m_graphRange.setDelegate(nullptr);
}

App::App(Snapshot * snapshot) :
  FunctionApp(snapshot, &m_inputViewController),
  m_listController(&m_listFooter, &m_listHeader, &m_listFooter, this),
  m_listFooter(&m_listHeader, &m_listController, &m_listController, ButtonRowController::Position::Bottom, ButtonRowController::Style::EmbossedGray),
  m_listHeader(&m_listStackViewController, &m_listFooter, &m_listController),
  m_listStackViewController(&m_tabViewController, &m_listHeader),
  m_graphController(&m_graphAlternateEmptyViewController, this, snapshot->graphRange(), snapshot->cursor(), snapshot->indexFunctionSelectedByCursor(), snapshot->rangeVersion(), &m_graphHeader),
  m_graphAlternateEmptyViewController(&m_graphHeader, &m_graphController, &m_graphController),
  m_graphHeader(&m_graphStackViewController, &m_graphAlternateEmptyViewController, &m_graphController),
  m_graphStackViewController(&m_tabViewController, &m_graphHeader),
  m_valuesController(&m_valuesAlternateEmptyViewController, this, &m_valuesHeader),
  m_valuesAlternateEmptyViewController(&m_valuesHeader, &m_valuesController, &m_valuesController),
  m_valuesHeader(&m_valuesStackViewController, &m_valuesAlternateEmptyViewController, &m_valuesController),
  m_valuesStackViewController(&m_tabViewController, &m_valuesHeader),
  m_tabViewController(&m_inputViewController, snapshot, &m_listStackViewController, &m_graphStackViewController, &m_valuesStackViewController),
  m_inputViewController(&m_modalViewController, &m_tabViewController, this, this, this)
{
}

CodePoint App::XNT() {
  int selectedFunctionIndex = m_listController.selectedRow();
  if (selectedFunctionIndex >= 0) {
    assert(selectedFunctionIndex < functionStore()->numberOfModels());
    Ion::Storage::Record record = functionStore()->recordAtIndex(selectedFunctionIndex);
    return functionStore()->modelForRecord(record)->symbol();
  }
  return 'x';
}

NestedMenuController * App::variableBoxForInputEventHandler(InputEventHandler * textInput) {
  MathVariableBoxController * varBox = AppsContainer::sharedAppsContainer()->variableBoxController();
  varBox->setSender(textInput);
  varBox->lockDeleteEvent(MathVariableBoxController::Page::Function);
  return varBox;
}

}
