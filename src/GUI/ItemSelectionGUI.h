// TO DO: rethink the way the configuraiotns are updated
// i think we can just call the updateConfiger(int) and make it control all motions
// TO DO: Set up the slider values initially. When loaded, signal slider
// and update slider values.

#include "MainWin.h"

#include <vector>
#include <string>
using namespace std;

#include <QTreeWidget>
#include <QTreeWidgetItem>

namespace plum {
  class GLModel;
  class PlumObject;
}
using namespace plum;

class VizmoListViewItem : public QTreeWidgetItem {

  public:
    VizmoListViewItem(QTreeWidget* _parent):QTreeWidgetItem(_parent){m_model=NULL;}
    VizmoListViewItem(QTreeWidgetItem* _parent):QTreeWidgetItem(_parent){m_model=NULL;}
    GLModel* m_model;
};

class VizmoItemSelectionGUI : public QTreeWidget{

  Q_OBJECT

  public:
    VizmoItemSelectionGUI(QWidget* _parent = NULL);
    void ResetLists();
    void FillTree(vector<PlumObject*>& _objs);
    VizmoListViewItem* CreateItem(VizmoListViewItem* _p, GLModel* _model);

  public slots:
    void Select();

  signals:
    void itemSelected();
    void CallUpdate();
    void UpdateTextGUI();

  private slots:
    void SelectionChanged();

  private:
    void ClearLists();
    int m_maxNoModels;
    vector<VizmoListViewItem*> m_items;
};




