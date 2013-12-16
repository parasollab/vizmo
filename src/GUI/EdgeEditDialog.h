/********************************************************************************
 * A dialog for modifying an edge. Initial functionality: Select an intermediate
 *  cfg from list and move it with the standard NodeEditDialog. Can also add an
 *  intermediate immediately after selected one in list or remove selected.
 ********************************************************************************/

#ifndef EDGE_EDIT_DIALOG_H
#define EDGE_EDIT_DIALOG_H

#include <string>
#include <vector>

#include <QtGui>

using namespace std;

class GLWidget;
class CfgModel;
class EdgeModel;

struct CfgListItem : public QListWidgetItem {

    CfgListItem(QListWidget* _parent, CfgModel* _cfg) : QListWidgetItem(_parent), m_cfg(_cfg) {}
    CfgModel* m_cfg;
};

class CfgListWidget : public QListWidget {

  Q_OBJECT

  public:
    CfgListWidget(QWidget* _parent = NULL);
    vector<CfgListItem*>& GetListItems() { return m_items; }

  signals:
    void CallUpdateGL();

  private slots:
    void SelectInMap();

  private:
    vector<CfgListItem*> m_items;
};

class EdgeEditDialog : public QDialog {

  Q_OBJECT

  public:
    EdgeEditDialog(QWidget* _parent, EdgeModel* _edge, GLWidget* _scene);
    ~EdgeEditDialog();

    void SetUpWidgets();
    void SetCurrentEdge(EdgeModel* _edge);
    void ClearIntermediates();
    void ResetIntermediates();

  private slots:
    void EditIntermediate();
    void AddIntermediate();
    void RemoveIntermediate();

  private:
    QHBoxLayout* m_buttonLayout;
    QVBoxLayout* m_overallLayout;
    QLabel* m_edgeLabel;
    CfgListWidget* m_intermediatesList;
    QPushButton* m_editIntermediateButton;
    QPushButton* m_addIntermediateButton;
    QPushButton* m_removeIntermediateButton;
    QPushButton* m_doneButton;

    GLWidget* m_glScene;
    EdgeModel* m_currentEdge;
};

#endif
