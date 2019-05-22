#ifndef ANIMATION_WIDGET_H_
#define ANIMATION_WIDGET_H_

#include <string>

#include <QtGui>

////////////////////////////////////////////////////////////////////////////////
/// \brief Animation bar located above GLWidget
///
/// AnimationWidget is a play/movie bar above the GLWidget which allows
/// play forward, play backward, step forward/back, pause, go to frame, and a
/// variable step size. Currently both Path and Debug can be animated.
////////////////////////////////////////////////////////////////////////////////
class AnimationWidget : public QToolBar {

  Q_OBJECT

  public:
    AnimationWidget(QString _title, QWidget* _parent);

    void Reset();            ///< Reset buttons based on loaded models.

  private slots:

    void GoToFrame(int);     ///< Advance playbar and scene to selected frame
    void BackAnimate();      ///< Continuously play animation backwards
    void Animate();          ///< Continuously play animation forwards
    void PauseAnimate();     ///< Pause animation bar
    void PreviousFrame();    ///< Step to previous frame
    void NextFrame();        ///< Advance frame by 1
    void GoToFirst();        ///< Place scene in frame number 1
    void GoToLast();         ///< Place scene in last frame
    void SliderMoved(int);   ///< Callback for slider bar changes
    void UpdateStepSize();   ///< Callback when step size is altered
    void UpdateFrame();      ///< Callback for frame selection by value
    void Timeout();          ///< Callback for Animate and BackAnimate timer

  private:
    void CreateGUI();        ///< Define layout of playbar and connections
    void CreateActions();    ///< Specify all button actions: Play, pause, etc.
    void CreateSlider();     ///< Define slider bar
    void CreateStepInput();  ///< Initialize step size input with validator
    void CreateFrameInput(); ///< Initialize frame number input with validator

    std::string m_name;      ///< Name of either Path or Debug
    int m_frame;             ///< Current Frame number
    int m_maxFrame;          ///< Number of total animation frames in Path/Debug
    size_t m_stepSize;       ///< When playing/stepping number of frames to skip
    bool m_forward;          ///< True: Animate forward, false: animate back

    QSlider* m_slider;       ///< Slider bar
    QLineEdit* m_stepField;  ///< Step size input field
    QIntValidator* m_stepValidator;  ///< Step size validation of integer
    QLineEdit* m_frameField; ///< Frame number input field
    QIntValidator* m_frameValidator; ///< Frame number validation of integer
    QLabel* m_totalSteps;    ///< Label of m_maxFrame
    QTimer* m_timer;         ///< Timer for Animate and BackAnimate functions
};

#endif
