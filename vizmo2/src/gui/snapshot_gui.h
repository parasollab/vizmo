// GUI for screen capture and movie

#include "vizmo2.h"
//#include "main_win.h"
/** 
 * This class deifines the animation GUI for vizmo2
 *
 */



#ifdef WIN32
#pragma warning(disable : 4786)
#endif



///////////////////////////////////////////////////////////////////////////////// QT Headhers
#include <qmainwindow.h>
#include <qnamespace.h>


class QToolBar;
class QAction;


#include <vector>
#include <string>


using namespace std;


class gliPickBox;

class VizmoScreenShotGUI : public QToolBar
{
  Q_OBJECT

   
  public:
  
  VizmoScreenShotGUI(QMainWindow *,char * );


 signals:
  void getScreenSize(int *w,int *h);
  void getBoxDimensions(int *xOffet,int *yOffset,int *w,int *h);
  void togleSelectionSignal();
  void simulateMouseUp();
  void callUpdate();



 private:
  void CreateActions();
  bool CreateGUI();

private slots:
 void  takeSnapshot();
 void takeBoxSnapshot(); 



 private:
 QAction *takePictureAction;     // the whole opengl screen
 QAction *takeBoxPictureAction; // only the selected box region

 gliPickBox *pictureRegion;
 bool boxPicture;
 

};

  
