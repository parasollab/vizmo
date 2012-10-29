#ifndef FILE_LIST_DIALOG_H
#define FILE_LIST_DIALOG_H

#include <QDialog>

class QLabel;
class QDialog; 
class QPushButton; 
class QGridLayout;

class FileListDialog : public QDialog{
    
  Q_OBJECT
        
  public:
    FileListDialog(QWidget* _parent, Qt::WFlags _f = 0);
    
  private slots:
    void ChangeMap();
    void ChangeEnv();
    void ChangePath();
    void ChangeDebug();
    void ChangeQry();
    
  private:
    QGridLayout* m_layout;
    void SetUpLayout(); 
    //The bold, unchanging labels on the left side of the window
    QLabel* m_mapLabel;  
    QLabel* m_envLabel;
    QLabel* m_pathLabel;
    QLabel* m_debugLabel;
    QLabel* m_queryLabel;

    //The actual displayed file names/paths 
    QLabel* m_mapFilename; 
    QLabel* m_envFilename; 
    QLabel* m_pathFilename; 
    QLabel* m_debugFilename; 
    QLabel* m_queryFilename; 

    QPushButton* m_mapButton; 
    QPushButton* m_envButton; 
    QPushButton* m_pathButton; 
    QPushButton* m_debugButton; 
    QPushButton* m_queryButton; 
    
    QPushButton* m_doneButton; 
};

#endif//FILE_LIST_DIALOG_H

