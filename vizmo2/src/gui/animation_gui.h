// TO DO: rethink the way the configuraiotns are updated
// i think we can just call the updateConfiger(int) and make it control all motions
// TO DO: Set up the slider values initially. When loaded, signal slider
// and update slider values.


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


class QTimer;
class QToolBar;
class QSlider;
class QAction;
class QLineEdit;
class QBoxLayout;
class QWidget;


#include <vector>
#include <string>
using namespace std;


//#include <qvbox.h>

class VizmoAnimationGUI : public QToolBar
{

  Q_OBJECT


 
 public:
  VizmoAnimationGUI(QMainWindow *,char *);

 signals:
  void callUpdate();

private slots:

    void animate2();
    void timeout();
    void pauseAnimate();
    void sliderMoved(int);
    void updateFrameCounter(int);
    void goToFrame();
    void updateStepSize();
    void gotoFirst();
    void gotoLast();
    void backAnimate();
    void nextFrame();
    void previousFrame();

 private:

  bool CreateActions();
  bool CreateGUI();
  void CreateSlider();
  void CreateFrameInput();
  void CreateStepInput();

  QAction *playPathAction;
  QAction *pausePathAction;
  QAction *firstFrame;
  QAction *lastFrame;
  QAction *playBackAction;
  QAction *nextFrameAction;
  QAction *previousFrameAction;

  
  QSlider *slider;
  QTimer *QTtimer;
  QLineEdit *frameCounter;
  QLineEdit *stepField;

  int stepSize;


  bool forwardDirection;




};






