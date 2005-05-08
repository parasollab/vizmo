#include "vizmo2.h"

/** 
* This class deifines the animation GUI for vizmo2
*/

#ifdef WIN32
#pragma warning(disable : 4786)
#endif

///////////////////////////////////////////////////////////////////////////////
// QT Headhers
#include <qmainwindow.h>
#include <qtoolbar.h>
#include <vector>
#include <string>

using namespace std;

class QListBox;
class QToolButton;

#include <qwidget.h>
#include <qaction.h>

class VizmoRoadmapGUI: public QToolBar
{
    Q_OBJECT
        
public:
    
    VizmoRoadmapGUI(QMainWindow * parent=NULL,char *name=NULL);
    void reset();
    void createGUI();
    //to handle the saving of new roadmap file
    void SaveNewRoadmap(const char *filename);
    bool WriteHeader(const char *filename);

public slots:
    void changeSize();      //Changes road map node's size
    void changeColor();      //Changes CC's color
    void changeColorOfCCselected(); //Changes CC's color when one is selected
    void setSameColor();     // set all CC to the same color

    void handleSelect();
    void MoveNode();

signals:
    void callUpdate();

private slots:
   void getSelectedItem();
   void editMap();         //add/delete nodes and edges 
   void addNode();
   void addEdge();
   void handleAddEdge();
   void handleAddNode();
   void handleEditMap();

 public:
    QAction * editAction,
            * addNodeAction,
            * addEdgeAction,
            * sizeAction,
            * colorAction;
 private:
 
    // private:
    QListBox *l;
    double size;
    QToolButton *nodesColor, *nodesSameColor;

    string m_shapeString;
    bool m_bEditModel;
    bool m_addNode;
    bool m_addEdge;

    list<CGLModel*> m_Nodes;  //selected nodes
    double old_T[3], old_R[3];
    bool m_Map_Changed;
};
