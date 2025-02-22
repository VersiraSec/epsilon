#ifndef GRAPH_LIST_TYPE_PARAMATER_CONTROLLER_H
#define GRAPH_LIST_TYPE_PARAMATER_CONTROLLER_H

#include <escher/view_controller.h>
#include <escher/selectable_list_view_controller.h>
#include <escher/selectable_table_view.h>
#include <escher/message_table_cell_with_expression.h>
#include <escher/stack_view_controller.h>
#include <ion/storage.h>

namespace Graph {

class TypeParameterController : public Escher::SelectableListViewController {
public:
  TypeParameterController(Escher::Responder * parentResponder);

  void didBecomeFirstResponder() override;
  bool handleEvent(Ion::Events::Event event) override;

  // ViewController
  const char * title() override;
  void viewWillAppear() override;
  TELEMETRY_ID("TypeParameter");

  // MemoizedListViewDataSource
  int numberOfRows() const override { return k_numberOfTypes; }
  KDCoordinate nonMemoizedRowHeight(int j) override;
  void willDisplayCellForIndex(Escher::HighlightCell * cell, int index) override;
  Escher::MessageTableCellWithExpression * reusableCell(int index, int type) override;
  int reusableCellCount(int type) override { return k_numberOfTypes; }
  int typeAtIndex(int index) override { return 0; }

  void setRecord(Ion::Storage::Record record) { m_record = record; }
private:
  constexpr static int k_numberOfTypes = 3;
  Escher::StackViewController * stackController() const;
  Escher::MessageTableCellWithExpression m_cells[k_numberOfTypes];
  Ion::Storage::Record m_record;
};

}

#endif
