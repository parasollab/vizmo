#ifndef _ROADMAP_H_
#define _ROADMAP_H_
#include "vizmo2.h"

/** 
* This class deifines the animation GUI for vizmo2
* Interface for non-implemented features temporarily removed
*/

#ifdef WIN32
#pragma warning(disable : 4786)
#endif

///////////////////////////////////////////////////////////////////////////////
// QT Headhers
#include <q3mainwindow.h>
#include <q3toolbar.h>
#include <qlineedit.h>
#include <qstringlist.h> 
//Added by qt3to4:
#include <QLabel>
#include <QButtonGroup> 
#include <QPushButton> 
#include <QMenu> 

#include <vector>
#include <string>

using namespace std;

/*class Q3ListBox; */
class QToolButton;
class QLabel;
class Q3HBox;
class Q3VBox;


#include <QtGui>

class queryGUI;


class VizmoMainWin;
class VizGLWin;
class VizmoRoadmapGUI: public Q3ToolBar
{
    Q_OBJECT
        
public:
    
    VizmoRoadmapGUI(Q3MainWindow * parent=NULL,char *name=NULL);
    void reset();
    void createGUI();
    //to handle the saving of new roadmap file
    void SaveNewRoadmap(const char *filename);
    bool WriteHeader(const char *filename);
    // let users set Cfg for a new node (AddNode)
    void createWindow();

    //add node to a new CC
    void addNodeToCC();

    //For robot's cfg
    void createRobotToolBar();

    //print NodeCfg
    void printNodeCfg(CCfg *c);
    void createQGrid();

    //set node shape: used when adding nodes
    void setShape();

public slots:
    void changeSize();      //Changes road map node's size
    void changeColor();      //Changes CC's color
    void changeColorOfCCselected(); //Changes CC's color when one is selected
    void changeInvisibleOfNodeselected(); //Changes Node's color when one is selected
    void changeWireOfNodeselected(); //Changes Node's color when one is selected
    void changeSolidOfNodeselected(); //Changes Node's color when one is selected
    void changeColorOfNodeselected(); //Changes Node's color when one is selected
    void setSameColor();     // set all CC to the same color
    void changeNodeShape(QAction*);  //for dropdown menubar

    void handleSelect();
    void MoveNode();

    void printRobCfg();

    //used to update node's cfg when nodeGUI changes values
    void updateNodeCfg();


signals:
    void callUpdate();

private slots:
   void getSelectedItem();
   //void editMap();         //add/delete nodes and edges 
   //void addNode();
   //void addEdge();
   void handleAddEdge();
   void handleAddNode();
   void handleEditMap();

 public:
    QAction * sizeAction,
            * colorAction,
	    * colorSelectAction,
	    * invisibleSelectNodeAction,
	    * wireSelectNodeAction,
	    * solidSelectNodeAction,
	    * colorSelectNodeAction, 
            * m_ccsOneColor; 
    //Temporarily removed from QActions above: *editAction, *addNodeAction,
    //*addEdgeAction 

    double *m_cfg;
    int m_dof;
    vector<double> newValue;
    QLabel *l_cfg, *l_robCfg;
    bool robCfgOn;
    bool m_addNode;
    bool m_addEdge;
    //CCfg 

 private:
 
    // private:
    QButtonGroup* m_nodeView; 

    double size;

    string m_shapeString;
    bool m_bEditModel;

    list<CGLModel*> m_Nodes;  //selected nodes
    vector<CGLModel*> Node_Edge; //nodes to be connected
    double old_T[3], old_R[3];
    bool m_Map_Changed;

    QLineEdit *tx, *ty, *tz, *l_dof;
    vector<QLineEdit*> lineEdt;

    QLabel *l_message, *l_icon;
    QStringList sl_cfg;
    QString s_cfg, s_robCfg;

    Q3HBox *m_grid;
    Q3VBox *m_vbox;


    ///node GUI
    queryGUI *nodeGUI;
    bool noMap;
    vector<double> v_cfg;
};

#endif /*_ROADMAP_H_*/
