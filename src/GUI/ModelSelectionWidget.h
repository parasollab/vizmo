#ifndef MODEL_SELECTION_WIDGET_H_
#define MODEL_SELECTION_WIDGET_H_

// TO DO: rethink the way the configuraiotns are updated
// i think we can just call the updateConfiger(int) and make it control all motions
// TO DO: Set up the slider values initially. When loaded, signal slider
// and update slider values.

using namespace std;

#include <QtGui>

class Model;
class GLWidget;

class ModelSelectionWidget : public QTreeWidget {

  Q_OBJECT

  public:
    struct ListViewItem : public QTreeWidgetItem {
      ListViewItem(QTreeWidget* _parent) : QTreeWidgetItem(_parent),
          m_model(NULL) {}
      ListViewItem(QTreeWidgetItem* _parent) : QTreeWidgetItem(_parent),
          m_model(NULL) {}
      Model* m_model;
    };
    ModelSelectionWidget(GLWidget* _glWidget, QWidget* _parent = NULL);

    void CallResetLists();

  signals:
    void ResetSignal();
    void UpdateTextWidget();
    void ResetListsFromMPSig();

  public slots:
    void ResetLists();
    void Select();
    void SelectionChanged();

  private:
    void FillTree(vector<Model*>& _objs);
    ListViewItem* CreateItem(ListViewItem* _p, Model* _model);
    void ClearLists();
    int m_maxNoModels;
    vector<ListViewItem*> m_items;
    GLWidget* m_glWidget;
};

#endif
