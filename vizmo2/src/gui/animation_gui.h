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

class QTimer;
class QSlider;
class QAction;
class QLineEdit;
class QBoxLayout;
class QLabel;
class QWidget;

#include <vector>
#include <string>
using namespace std;

class VizmoAnimationGUI : public QToolBar
{
	
	Q_OBJECT
		
public:
	VizmoAnimationGUI(QMainWindow * parent=NULL,char * name=NULL);
	void reset(); //reset every thing
	
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
	void UpdateCurValue(int value);

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
	QLabel * totalStep;
	
	int stepSize;
	int max_value;
	int cur_value;
	bool forwardDirection;
};






