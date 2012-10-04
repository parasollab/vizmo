// TO DO: rethink the way the configuraiotns are updated
// i think we can just call the updateConfiger(int) and make it control all motions
// TO DO: Set up the slider values initially. When loaded, signal slider
// and update slider values.

#include "vizmo2.h"

// This class deifines the animation GUI for vizmo2

#ifdef WIN32
#pragma warning(disable : 4786)
#endif

///////////////////////////////////////////////////////////////////////////////
// QT Headers
#include <QLabel>
#include <QToolBar>  

class QMainWindow; 
class QTimer;
class QSlider;
class QAction;
class QLineEdit;
class QLabel;
class QWidget;

#include <vector>
#include <string>

using namespace std;

class VizmoAnimationGUI : public QToolBar {
    
  Q_OBJECT
        
  public:
    VizmoAnimationGUI(QString _title, QWidget* _parent = NULL, string _name = NULL); 
    void reset(); //reset everything
    
  signals:
    void callUpdate();
    
  private slots:
    void animate2();
    void timeout();
    void pauseAnimate();
    void sliderMoved(int);
    void updateFrameCounter(int);
    void goToFrame();
    void goToFrame(int);
    void updateStepSize();
    void getStepSize(int& size);
    void gotoFirst();
    void gotoLast();
    void backAnimate();
    void nextFrame();
    void previousFrame();
    
  private:
    bool CreateActions(); //Play, pause, etc. 
    bool CreateGUI();
    void CreateSlider();
    void CreateFrameInput();
    void CreateStepInput();
    void UpdateCurValue(int value);
    
    QAction* m_playPathAction;
    QAction* m_pausePathAction;
    QAction* m_firstFrame;
    QAction* m_lastFrame;
    QAction* m_playBackAction;
    QAction* m_nextFrameAction;
    QAction* m_previousFrameAction;
    
    QSlider* m_slider; 
    QLabel* m_stepLabel;
    QLineEdit* m_stepField; 
    QLabel* m_frameLabel; 
    QLineEdit* m_frameField;
    QLabel* m_slash; 
    QLabel* m_totalSteps;
    QLabel* m_framesLast; 

    QTimer* m_timer; 

    int m_stepSize;
    int m_maxValue;
    int m_curValue;
    bool m_forwardDirection;
    string m_name;
};






