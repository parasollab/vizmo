#include <QDialog>
#include <QtGui>
///////////////////////////////////////////////////////////////////////////////
using namespace std;

///////////////////////////////////////////////////////////////////////////////
#define M_MAX 999.99
#define M_MIN -999.99
#define STEP 0.1
#define STEP_ANGLE 0.01
#define D_MAX 1
#define D_MIN 0
#define DECIMALS 6
///////////////////////////////////////////////////////////////////////////////

class QueryGUI: public QDialog {

  Q_OBJECT

  public:
    QueryGUI(QWidget* _parent, Qt::WFlags _f=0);

  signals:
    void CallUpdate();

  private slots:
    void NewCfg(const QString&);
    void UpdateQryCfg();
    void ResetCfg();

  private:
    void SetQuery(vector<double>& _q);
    void SetNodeVal(int _dof, double* _cfg);
    vector<double> GetNodeCfg();
    void ResetPointer();
    bool m_filledFirstTime;
    int m_dof;
    double* m_qCfgTmp;
    QDoubleSpinBox* m_stx,* m_sty,* m_stz;
    vector<QDoubleSpinBox*> m_spin;
    vector<double> m_queryOrg;
    double* m_nodeCfg;
    string m_objName;
};
