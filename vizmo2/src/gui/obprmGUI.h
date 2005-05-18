#ifndef VIZMO2_OBPRM_GUI
#define VIZMO2_OBPRM_GUI

///////////////////////////////////////////////////////////////////////////////
// QT Headhers
#include <qmainwindow.h>
#include <qnamespace.h>
#include <qdialog.h>
#include <qtoolbar.h>
#include <qwidget.h>
#include <qvbuttongroup.h> 

#include "vizmoEditor.h"

class obprmFileSelectDialog;
class textEditDialog;

class QLabel;
class QButtonGroup;



#endif//VIZMO2_OBPRM_GUI

#include <qtabdialog.h>
#include <qstring.h>
#include <qvbox.h>
#include <qapplication.h> 
#include <qlineedit.h>
#include <qgrid.h>
#include <qcombobox.h>
#include <qlayout.h>
#include <qbuttongroup.h> 
#include <qcheckbox.h> 
#include <qlistbox.h> 
// used to store and manipulate strings that belong together.
#include <qstringlist.h> 

#include <qtextedit.h>
#include <qfile.h> 

class obprmGUI : public QTabDialog
{
    Q_OBJECT

public:
    obprmGUI( QWidget *parent, const char *name, WFlags f=0);
    

    QStringList command;
    textEditDialog * txtDialog;
    QString strComm;
    QString obprmPath;

    //tabs
    QVBox  *tab4;
    //file name
    QLineEdit *fname;

    obprmFileSelectDialog *fileDialog;
    //bbox variables
    QButtonGroup * bGrp;
    QLineEdit *xmin, *ymin, *zmin, *xmax, *ymax, *zmax;
    QLineEdit *scale;

    ///////////////////
    //GNodes GUI
    ///////////////////

    QGridLayout *basicGrid, *commonGrid;
    QWidget *basicWidgt;
    QWidget *leftArea, *rightArea;
    QHBoxLayout *hlay, *h;
    QComboBox *combo;
    QListBox *lbMethod;

    //Basic
    QLineEdit *numNodes;
    //General tab
    QLineEdit *addPartEdge, *addAllEdgs, *posRes, *oriRes;
    QCheckBox *cb_allEdges, *cb_partEdges, *cb_posRes, *cb_oriRes;
    QLineEdit * numJoints;
    QCheckBox *cb_numJoints;
    //obprm
    QLineEdit *numShells, *clearFact, *pctSurf;
    QLabel *l_s, *l_collPair, *l_freePair, *l_clear, *l_pct;
    QComboBox *cb_coll1, *cb_free1,*cb_coll2, *cb_free2;
    //gauss
    QLineEdit *distance, *nodes_gauss;
    QLabel *l_nodesGauss, *l_d;
    //MaprmGUI
    QLabel *l_app_ray, *l_appx, *l_nodes;
    QLineEdit *app_ray, *appx,*numNodesMaprm;

    ///////////////////
    //CNodes GUI
    ///////////////////
    QPushButton *pbCN;
    QLineEdit * closest, * closestVE, *componts1, *componts2;
    QLineEdit *obstB_o, *obstB_s;
    QLineEdit *rrt1, *rrt2, *rrt3,*rrt4, *rrt5;
    QLineEdit *rrtc1, *rrtc2, *rrtc3,*rrtc4, *rrtc5;
    QLineEdit *rt1, *rt2, *rt3, *rt4, *rt5, *rt6, *rt7;
    QLineEdit *lm_p, *lm_a, *lm_r;
    QListBox *lbCnodes;
    QHBoxLayout *hc;
    ///////////////////
    //lp GUI
    ///////////////////
    QLineEdit *lp_strLine1,*lp_strLine2, *lp_rotate;
    QLineEdit *lp_starD1,*lp_starD2, *lp_starC1,*lp_starC2;
    QLineEdit *lp_appx;
    //to generate comman line
    QListBox *lbLP;
    QPushButton *pb;
    ///////////////////
    //CD GUI
    ///////////////////
    QButtonGroup *bg_cd;
 
    ////////////////////////////////////////
    // Generate string of methods, LPs, etc
    ////////////////////////////////////////
    QStringList methods, lps,  cNode;

    vizmoEditor *vizEditor;

private slots:

  void addMethod(); //-gNodes opt.
  void makeCommandLine();
  void deleteMethod(const QString &s);
  void showFileDialog();
  void showBoxes();
  void getOption();
  ///////////////////
  //GNodes GUI
  ///////////////////
  void BasicObprmGUI();
  void ObprmGUI();
  void GaussGUI();
  void BMaprmGUI();
  void CSpaceGUI();
  void OBMaprmGUI();
  void hideBasicObprmGUI();
  void changeOption(const QString &s);
  ///////////////////
  //-lp GUI
  ///////////////////
  void deleteLP(const QString & );
  void LP1(); void LP2(); void LP3();
  void LP4(); void LP5();
  ///////////////////
  //-cNodes GUI
  ///////////////////
  void deleteCnodes(const QString &s);
  void addLM(); void addObst(); void addRT();
  void addRandom(); void addClosest(); void addClosestVE();
  void addComp(); void addRRTe(); void addRRTc();

protected:

    void setupTabFile();
    void setupTabBbox();
    void setupTabGnodes();
    void setupTabCnodes();
    void setupTabLp();
    void setupTabCD();

    void createFileDialog();

    void createTxtDialog();
    void showTxtDilaog();

    void addBoxes(QGrid *grid);


    void getOBPRMpath(); // get path of obprm executable

    ///////////////////
    //GNodes GUI
    ///////////////////

    void createElemtsBasicPRM();
    void createElemtsBasicObprm();
    void createElemtsObprm();
    void createElmtsGauss();
    void createElmtsBMaprm();
 
    void hideElemtsBasicPRM();
    void hideElemtsBasicObprm();
    void hideElemtsObprm();
    void hideElmtsGauss();
    void hideElmtsBMaprm();
 
    void showElemtsBasicPRM();
    void showElemtsBasicObprm();
    void showElemtsObprm();
    void showElmtsGauss();
    void showElmtsBMaprm();

    //CSpaceMAPRM and BasicMAPRM
    //will share variables
    void resetElmtsBMaprm();
    void resetElmtsCSpace();
 
    ///////////////////
    //GNodes GUI
    ///////////////////
    void createCNodesGUI();

    ///////////////////
    //-lp GUI
    ///////////////////

 

};


class obprmFileSelectDialog: public QDialog
{
    Q_OBJECT
        
public:
    obprmFileSelectDialog(QWidget *parent, const char *name, WFlags f=0);
    

private slots:

    void changeEnv();
    void changeMapIN();
    void changeMapOUT();

private:


public:

    QLabel * INMap_label,* OUTMap_label;
    QLabel * Env_label;


};


class textEditDialog : public QDialog
{
  Q_OBJECT;

public:

  textEditDialog(QWidget *parent, const char *name, WFlags f=0);

  QTextEdit *txt;
  QPushButton *pb;
  QString fn;
  QFile file;

private slots:

  void createFile();
  void execFile();
};
