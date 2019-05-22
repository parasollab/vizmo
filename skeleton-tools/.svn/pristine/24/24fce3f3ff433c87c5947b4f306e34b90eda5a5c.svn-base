#ifndef NODE_EDIT_DIALOG_H_
#define NODE_EDIT_DIALOG_H_

#include <string>
#include <vector>

#include <QtGui>

#include "Models/MapModel.h"
#include "Models/CfgModel.h"
#include "Models/EdgeModel.h"
#include "Models/TempObjsModel.h"

using namespace std;

class GLWidget;
class MainWindow;


class NodeEditValidator : public QDoubleValidator {

  public:

    NodeEditValidator(double _min, double _max, int _decimals, QWidget* _parent)
        : QDoubleValidator(_min, _max, _decimals, _parent), m_min(_min),
        m_max(_max) { }

    virtual QValidator::State validate(QString& _s, int& _i) const;

  private:

    double m_min;
    double m_max;
};


class NodeEditSlider : public QWidget {

  Q_OBJECT

  public:

    NodeEditSlider(QWidget* _parent, string _label);

    QSlider* GetSlider() { return m_slider; }
    QLineEdit* GetDOFValue() { return m_dofValue; }

  private slots:

    void UpdateDOFLabel(int _newVal);
    void MoveSlider(QString _inputVal);

  private:

    virtual bool eventFilter(QObject* _target, QEvent* _event);

    QSlider* m_slider;
    QLineEdit* m_dofValue;
};


////////////////////////////////////////////////////////////////////////////////
/// \brief A dialog for modifying the cfg of a single node.
////////////////////////////////////////////////////////////////////////////////
class NodeEditDialog : public QDialog {

  Q_OBJECT

  public:

    typedef vector<NodeEditSlider*>::iterator SIT;
    typedef MapModel<CfgModel, EdgeModel> Map;
    typedef Map::Graph Graph;
    typedef Map::EID EID;
    typedef Map::EI EI;
    typedef Map::VID VID;
    typedef Map::VI VI;

    //Edit node constructor
    NodeEditDialog(MainWindow* _mainWindow, string _title,
        CfgModel* _originalNode, EdgeModel* _parentEdge = NULL);
    //Add new node constructor
    NodeEditDialog(MainWindow* _mainWindow, string _title);
    //Merge nodes constructor
    NodeEditDialog(MainWindow* _mainWindow, string _title, CfgModel* _tempNode,
        vector<VID> _nodesToConnect, vector<VID> _nodesToDelte);

  private slots:

    void UpdateDOF(int _id);  //Update value of DOF associated with m_sliders[_id]
    //Custom finishing slots for various use cases
    void FinalizeNodeEdit(int _accepted);  //For when dialog is used to modify existing node
    void FinalizeNodeAdd(int _accepted);  //For using dialog to add new nodes
    void FinalizeNodeMerge(int _accepted);  //For using dialog to merge nodes

    void DumpRobotInfo(); ///< Dump robot info to standard out.

  private:

    void Init();
    void SetUpSliders(vector<NodeEditSlider*>& _sliders);
    void InitSliderValues(const vector<double>& _vals);
    void ValidityCheck();

    vector<NodeEditSlider*> m_sliders; //destruction??
    string m_title;               ///< The node name.
    CfgModel* m_tempNode;         ///< Pointer to the temporary working node.
    CfgModel* m_originalNode;     ///< Pointer to the original node.
    vector<VID> m_nodesToConnect; ///< Neighbors of the editing node.
    vector<VID> m_nodesToDelete;  ///< Nodes to be removed on after merging.
    TempObjsModel m_tempObjs;     ///< Manages and draws temporary nodes/edges.
};

#endif
