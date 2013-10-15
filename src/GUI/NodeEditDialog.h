/********************************************************************************
 * A dialog for modifying the cfg of a single node.
 ********************************************************************************/

#ifndef NODE_EDIT_DIALOG_H
#define NODE_EDIT_DIALOG_H

#include <string>
#include <vector>

#include <QDialog>
#include <QVariant>
#include <QAction>
#include <QApplication>
#include <QButtonGroup>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QWidget>
#include <QSlider>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QScrollArea>
#include <QSignalMapper>

using namespace std;

class GLWidget;
class CfgModel;

class NodeEditSlider : public QWidget {

  Q_OBJECT

  public:
    NodeEditSlider(QWidget* _parent, string _label);

    QSlider* GetSlider() { return m_slider; }
    QLabel* GetDOFName() { return m_dofName; }
    QLabel* GetDOFValue() { return m_dofValue; }

  private slots:
    void UpdateDOFLabel(int _newVal);

  private:
    virtual bool eventFilter(QObject* _target, QEvent* _event); //ignore mouse wheel on sliders

    QHBoxLayout* m_layout;
    QSlider* m_slider;
    QLabel* m_dofName;
    QLabel* m_dofValue;
};

class NodeEditDialog : public QDialog {

  Q_OBJECT

  public:
    NodeEditDialog(QWidget* _parent, CfgModel* _node, GLWidget* _scene);
    ~NodeEditDialog();
    void SetUpWidgets();
    void SetCurrentNode(CfgModel* _node);
    void InitSliderValues(const vector<double>& _vals);
    void SetNodeLabel(string _nodeID, int _ccID);

  private slots:
    void UpdateDOF(int _id); //Update value of DOF associated with m_sliders[_id]

  private:
    QLabel* m_nodeLabel;
    QPushButton* m_setButton;
    QGroupBox* m_scrollAreaBox;
    QVBoxLayout* m_overallLayout;
    QVBoxLayout* m_scrollAreaBoxLayout;
    QScrollArea* m_scrollArea;
    QSignalMapper* m_sliderMapper;
    vector<NodeEditSlider*> m_sliders; //destruction??

    GLWidget* m_gLScene;
    CfgModel* m_currentNode;
};

#endif
