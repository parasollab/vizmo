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
    NodeEditSlider(QWidget* _parent, string _label);

    QSlider* GetSlider() { return m_slider; }
    QLabel* GetDOFName() { return m_dofName; }
    QLineEdit* GetDOFValue() { return m_dofValue; }

  private slots:
    void UpdateDOFLabel(int _newVal);
    void MoveSlider(QString _inputVal);

  private:
    virtual bool eventFilter(QObject* _target, QEvent* _event);

    QHBoxLayout* m_layout;
    QSlider* m_slider;
    QLabel* m_dofName;
    QLineEdit* m_dofValue;
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
    void ValidityCheck();

    QLabel* m_nodeLabel;
    QPushButton* m_doneButton;
    QGroupBox* m_scrollAreaBox;
    QVBoxLayout* m_overallLayout;
    QVBoxLayout* m_scrollAreaBoxLayout;
    QScrollArea* m_scrollArea;
    QSignalMapper* m_sliderMapper;
    vector<NodeEditSlider*> m_sliders; //destruction??

    GLWidget* m_glScene;
    CfgModel* m_currentNode;
};

#endif
