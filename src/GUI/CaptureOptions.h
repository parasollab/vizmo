#ifndef CAPTURE_OPTIONS_H_
#define CAPTURE_OPTIONS_H_

#include "OptionsBase.h"

////////////////////////////////////////////////////////////////////////////////
/// \brief CaptureOptions provides the crop box, screen shot, and movie making
/// tools.
////////////////////////////////////////////////////////////////////////////////

class CaptureOptions : public OptionsBase {

  Q_OBJECT

  public:

    CaptureOptions(QWidget* _parent = 0, MainWindow* _mainWindow = 0);

  public slots:

    void Record();               ///< Save an image as part of a movie.

  private slots:

    void CropRegion();           ///< Toggle the crop box on or off.
    void CapturePicture();       ///< Save an image of the current GL scene.
    void CaptureMovie();         ///< Save a series of images to make a movie.
    void StartLiveRecording();   ///< Record a movie of the current GL scene.
    void StopLiveRecording();    ///< Stop recording the current movie.

  signals:

    void SimulateMouseUp();      ///< Emulates mouse release.
    void ToggleSelectionSignal();///< Indicates that the crop box was toggled.
    void CallUpdate();           ///< Requests a redraw of the GL scene.
    void UpdateFrame(int);       ///< Indicates the current movie frame.

  private:

    void CreateActions();        ///< Create and connect actions.
    void SetUpToolbar();         ///< Create menu.
    void SetHelpTips();          ///< Set help messages for actions.
    void Reset();                ///< Reset actions to their enabled state.
    bool m_cropBox;              ///< Indicates whether the crop box is in use.

    //for live recording
    QString m_filename;          ///< Base filename for a movie.
    size_t m_frame;              ///< Current frame for the movie.
    size_t m_frameDigits;        ///< Number of digits in the movie filenames.
    size_t m_frameDigitStart;    ///< First index number in the movie filenames.
};

#endif
