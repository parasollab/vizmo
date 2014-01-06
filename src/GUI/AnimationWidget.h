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

    QSlider* m_slider;
    QLineEdit* m_stepField;
    QLineEdit* m_frameField;
    QLabel* m_totalSteps;

    QTimer* m_timer;

    int m_stepSize;
    int m_maxValue;
    int m_curValue;
    bool m_forwardDirection;
    string m_name;
};

