#include <QDialog>
#include <QtGui>
///////////////////////////////////////////////////////////////////////////////
// std Headers
#include <vector>
#include <string>
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


class queryGUI: public QDialog
{

  Q_OBJECT

 public:

  queryGUI(QWidget *parent, Qt::WFlags f=0);
  char m_SorG;

  void setQuery(double * q);
  void setNodeVal(int dof, double *cfg);
  vector<double> getNodeCfg();
  bool filledFirstTime;

signals:
    void callUpdate();


private slots:

    void SaveSG();
    void newCfg(const QString&);
    void updateQryCfg();
    void resetCfg();

private:

    void resetPointer();
   
    int m_dof;
    double * QcfgTmp, *Qcfg;
    QDoubleSpinBox *stx, *sty, *stz;
    vector<QDoubleSpinBox*> vSpin;
    double * query_org;
    double * node_cfg;
    string m_ObjName; 

};
