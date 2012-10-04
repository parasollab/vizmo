/**************************************************************
 * Class moved from SnapshotGUI.h/cpp, where it was combined 
 * with class VizmoScreenshotGUI, to its own files here. 
 * Used in close conjunction with SnapshotGUI utilities...
 **************************************************************/

#ifndef MOVIE_SAVE_DIALOG_H
#define MOVIE_SAVE_DIALOG_H

#include "MainWin.h"  //just because... 
#include <QDialog> 

class QDialog; 
class QLineEdit;
class QLabel;  

using namespace std; 

class MovieSaveDialog : public QDialog{

  Q_OBJECT

  public:
    MovieSaveDialog(QWidget* _parent, const char* _name, Qt::WFlags f=0);
    void UpdateAttributes();

  private slots:
    void SaveImages();
    void ShowFileDialog();

  private:
    void StoreAttributes();

    QLineEdit* m_startFrame;
    QLineEdit* m_endFrame;
    QLineEdit* m_stepSize;
    QLabel* m_fnameLabel;

  public:
    int m_startIntFrame;
    int m_endIntFrame;
    int m_stepIntSize;
    QString m_sFileName;
    QString m_sFileExt;
    int m_frameDigit;
};

#endif 


