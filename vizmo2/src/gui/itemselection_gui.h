// TO DO: rethink the way the configuraiotns are updated
// i think we can just call the updateConfiger(int) and make it control all motions
// TO DO: Set up the slider values initially. When loaded, signal slider
// and update slider values.


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

class QAction;
class QLineEdit;
class QLabel;


#include <vector>
#include <string>
using namespace std;

class QListView;

class VizmoItemSelectionGUI: public QToolBar
{
  Q_OBJECT

public:

  VizmoItemSelectionGUI(QMainWindow * parent=NULL,char *name=NULL);
  void reset();
  void fillTree();
 

 private:
  QListView *list;
  int maxNoModels;

};


  

