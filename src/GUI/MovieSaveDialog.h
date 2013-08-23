/**************************************************************
 * Class moved from SnapshotGUI.h/cpp, where it was combined
 * with class VizmoScreenshotGUI, to its own files here.
 * Used in close conjunction with SnapshotGUI utilities...
 **************************************************************/

#ifndef MOVIESAVEDIALOG_H
#define MOVIESAVEDIALOG_H

#include <QDialog>

class QGridLayout;
class QLineEdit;
class QLabel;

class MovieSaveDialog : public QDialog {

  Q_OBJECT

  public:
    MovieSaveDialog(QWidget* _parent, Qt::WFlags _f = 0);

  private slots:
    void SaveImages();
    void ShowFileDialog();

  private:
    void SetUpLayout();

    QLineEdit* m_startFrameEdit;
    QLineEdit* m_endFrameEdit;
    QLineEdit* m_stepSizeEdit;

    QLabel* m_startFrameLabel;
    QLabel* m_endFrameLabel;
    QLabel* m_stepSizeLabel;

    QPushButton* m_selectNameButton;
    QLabel* m_fileNameLabel;

    QPushButton* m_go;
    QPushButton* m_cancel;

    QGridLayout* m_layout;

  public:
    size_t m_startFrame; //start frame of video
    size_t m_endFrame; //end frame of video
    size_t m_stepSize; //step size for video
    size_t m_frameDigits; //number of # in the filename
    size_t m_frameDigitStart; //first index of # in the filename
    QString m_filename; //base filename for movie
};

#endif
