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
#include <QListWidget>
#include <QDialog>

#include "Models/TempObjsModel.h"

using namespace std;

class CfgModel;
class EdgeModel;
class GLWidget;
class MainWindow;
class NodeEditDialog;

struct CfgListItem : public QListWidgetItem {

    CfgListItem(QListWidget* _parent, CfgModel* _cfg) : QListWidgetItem(_parent), m_cfg(_cfg) {}
    CfgModel* m_cfg;
};

class CfgListWidget : public QListWidget {

  Q_OBJECT

  public:
    typedef vector<CfgListItem*>::iterator IIT;

    CfgListWidget(QWidget* _parent = NULL);
    vector<CfgListItem*>& GetListItems() { return m_items; }

  private slots:
    void SelectInMap();

  private:
    vector<CfgListItem*> m_items;
};

class EdgeEditDialog : public QDialog {

  Q_OBJECT

  public:
    typedef vector<CfgListItem*>::iterator IIT;

    EdgeEditDialog(MainWindow* _mainWindow, EdgeModel* _originalEdge);
    ~EdgeEditDialog();

    void ClearIntermediates();
    void ResetIntermediates();

  private slots:
    void EditIntermediate();
    void AddIntermediate();
    void RemoveIntermediate();
    void FinalizeEdgeEdit(int _accepted);

  private:
    CfgListWidget* m_intermediatesList;
    MainWindow* m_mainWindow;
    EdgeModel* m_originalEdge;
    EdgeModel* m_tempEdge;
    GLWidget* m_glScene;
    TempObjsModel m_tempObjs;
    QPointer<NodeEditDialog> m_nodeEditDialog;
};

#endif
