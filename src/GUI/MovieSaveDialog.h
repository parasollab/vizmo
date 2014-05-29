/**************************************************************
 * Class moved from SnapshotGUI.h/cpp, where it was combined
 * with class VizmoScreenshotGUI, to its own files here.
 * Used in close conjunction with SnapshotGUI utilities...
 **************************************************************/

#ifndef MOVIE_SAVE_DIALOG_H
#define MOVIE_SAVE_DIALOG_H

#include <QtGui>

class MainWindow;
class QGridLayout;
class QLineEdit;
class QLabel;

class MovieSaveDialog : public QDialog {

  Q_OBJECT

  public:
    MovieSaveDialog(MainWindow* _mainWindow, Qt::WFlags _f = 0);
    size_t m_startFrame; //start frame of video
    size_t m_endFrame; //end frame of video
    size_t m_stepSize; //step size for video
    size_t m_frameDigits; //number of # in the filename
    size_t m_frameDigitStart; //first index of # in the filename
    QString m_filename; //base filename for movie

  private slots:
    void SaveImages();
    void ShowFileDialog();

  private:
    QLineEdit* m_startFrameEdit;
    QLineEdit* m_endFrameEdit;
    QLineEdit* m_stepSizeEdit;
    QLabel* m_fileNameLabel;
};

#endif
