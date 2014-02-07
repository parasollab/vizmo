/********************************************************************************
 * A dialog for modifying the cfg of a single node.
 * 2 input options from here: Slider adjustment or explicit value entry
 ********************************************************************************/

#ifndef NODE_EDIT_DIALOG_H
#define NODE_EDIT_DIALOG_H

#include <string>
#include <vector>

#include <QtGui>

using namespace std;

class GLWidget;
class CfgModel;
class EdgeModel;

class NodeEditValidator : public QDoubleValidator{

  public:
    NodeEditValidator(double _min, double _max, int _decimals, QWidget* _parent)
      : QDoubleValidator(_min, _max, _decimals, _parent), m_min(_min), m_max(_max){}
    virtual QValidator::State validate(QString& _s, int& _i) const;

  private:
    double m_min;
    double m_max;
};

class NodeEditSlider : public QWidget {

  Q_OBJECT

  public:
    NodeEditSlider(QWidget* _parent, string _label, string _details);

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

class NodeEditDialog : public QDialog {

  Q_OBJECT

  public:
    typedef vector<NodeEditSlider*>::iterator SIT;

    NodeEditDialog(QWidget* _parent, CfgModel* _node, GLWidget* _scene, string _title);
    ~NodeEditDialog();
    void SetUpSliders(vector<NodeEditSlider*>& _sliders);
    void SetCurrentNode(CfgModel* _node, QLabel* _nodeLabel, string _title);
    void SetCurrentEdges(vector<EdgeModel*>* _edges) {m_currentEdges = _edges;}
    void InitSliderValues(const vector<double>& _vals);
    virtual int exec(); //override

  private slots:
    void UpdateDOF(int _id); //Update value of DOF associated with m_sliders[_id]
    virtual void reject(); //override

  private:
    void ValidityCheck();

    vector<NodeEditSlider*> m_sliders; //destruction??
    vector<int> m_oldValues;
    GLWidget* m_glScene;
    CfgModel* m_currentNode;
    vector<EdgeModel*>* m_currentEdges;
};

#endif
