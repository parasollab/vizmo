// TO DO: rethink the way the configuraiotns are updated
// i think we can just call the updateConfiger(int) and make it control all motions
// TO DO: Set up the slider values initially. When loaded, signal slider
// and update slider values.

#include <vector>
#include <string>
using namespace std;

#include <QTreeWidget>
#include <QTreeWidgetItem>

class GLModel;

class ModelSelectionWidget : public QTreeWidget {

  Q_OBJECT

  public:
    struct ListViewItem : public QTreeWidgetItem {
      ListViewItem(QTreeWidget* _parent) : QTreeWidgetItem(_parent), m_model(NULL) {}
      ListViewItem(QTreeWidgetItem* _parent) : QTreeWidgetItem(_parent), m_model(NULL) {}
      GLModel* m_model;
    };

    ModelSelectionWidget(QWidget* _parent = NULL);
    void ResetLists();
    void FillTree(vector<GLModel*>& _objs);
    ListViewItem* CreateItem(ListViewItem* _p, GLModel* _model);

  public slots:
    void Select();

  signals:
    void CallUpdate();
    void UpdateTextGUI();

  private slots:
    void SelectionChanged();

  private:
    void ClearLists();
    int m_maxNoModels;
    vector<ListViewItem*> m_items;
};
