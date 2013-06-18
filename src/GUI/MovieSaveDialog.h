/**************************************************************
 * Class moved from SnapshotGUI.h/cpp, where it was combined 
 * with class VizmoScreenshotGUI, to its own files here. 
 * Used in close conjunction with SnapshotGUI utilities...
 **************************************************************/

#ifndef MOVIE_SAVE_DIALOG_H
#define MOVIE_SAVE_DIALOG_H

#include "MainWin.h"  //just because... 
#include <QDialog> 

class QGridLayout; 
class QDialog; 
class QLineEdit;
class QLabel;  

using namespace std; 

class MovieSaveDialog : public QDialog{

  Q_OBJECT

  public:
    MovieSaveDialog(QWidget* _parent, Qt::WFlags _f = 0);
    void UpdateAttributes();

  private slots:
    void SaveImages();
    void ShowFileDialog();

  private:
    void SetUpLayout(); 
    void StoreAttributes();
    
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
    int m_startIntFrame;
    int m_endIntFrame;
    int m_stepIntSize;
    QString m_sFileName;
    QString m_sFileExt;
    int m_frameDigit;
};

#endif 


