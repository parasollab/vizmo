// TO DO: rethink the way the configuraiotns are updated
// i think we can just call the updateConfiger(int) and make it control all motions
// TO DO: Set up the slider values initially. When loaded, signal slider
// and update slider values.

using namespace std;

#include <QtGui>

class AnimationWidget : public QToolBar {
  Q_OBJECT

  public:
    AnimationWidget(QString _title, QWidget* _parent);
    void Reset(); //reset everything

  signals:
    void CallUpdate();

  private slots:
    void Animate();
    void Timeout();
    void PauseAnimate();
    void SliderMoved(int);
    void UpdateFrameCounter(int);
    void GoToFrame();
    void UpdateFrame(int);
    void UpdateStepSize();
    void GetStepSize(int& _size);
    void GoToFirst();
    void GoToLast();
    void BackAnimate();
    void NextFrame();
    void PreviousFrame();

  private:
    bool CreateActions(); //Play, pause, etc.
    bool CreateGUI();
    void CreateSlider();
    void CreateFrameInput();
    void CreateStepInput();
    void UpdateCurValue(int _value);

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

