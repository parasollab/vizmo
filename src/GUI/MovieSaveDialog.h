#ifndef MOVIE_SAVE_DIALOG_H
#define MOVIE_SAVE_DIALOG_H

#include <QtGui>
#include <QDialog>

class MainWindow;
class QGridLayout;
class QLineEdit;
class QLabel;

////////////////////////////////////////////////////////////////////////////////
/// \brief This dialog sets up parameters for saving a sequence of still images
/// for external compilation into a movie.
/// \details Output images will be named in the format \c (base)(index).jpg.
////////////////////////////////////////////////////////////////////////////////
class MovieSaveDialog : public QDialog {

  Q_OBJECT

  public:

    MovieSaveDialog(MainWindow* _mainWindow, Qt::WindowFlags _f = 0);

    size_t m_startFrame;      ///< The start frame.
    size_t m_endFrame;        ///< The end frame.
    size_t m_stepSize;        ///< The step size.
    size_t m_frameDigits;     ///< The number of digits in the filename indexes.
    size_t m_frameDigitStart; ///< The first filename index.
    QString m_filename;       ///< The base filename for output images.

  private slots:

    void SaveImages();        ///< Set output parameters.
    void ShowFileDialog();    ///< Display the dialog.

  private:

    QLineEdit* m_startFrameEdit; ///< GUI component for setting the start frame.
    QLineEdit* m_endFrameEdit;   ///< GUI component for setting the end frame.
    QLineEdit* m_stepSizeEdit;   ///< GUI component for setting the step size.
    QLabel* m_fileNameLabel;     ///< GUI component for setting the base name.
};

#endif
