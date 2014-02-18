#ifndef FILELISTDIALOG_H_
#define FILELISTDIALOG_H_

using namespace std;

#include <QtGui>

class FileListDialog : public QDialog {

  Q_OBJECT

  public:
    FileListDialog(const string& _filename, QWidget* _parent, Qt::WFlags _f = 0);

  private slots:
    void ChangeEnv();
    void ChangeMap();
    void ChangeQuery();
    void ChangePath();
    void ChangeDebug();
    void Accept();
    void PathChecked();
    void DebugChecked();

  private:
    //This file locates all possible filenames related to the input file.
    //For example, *.path, *.env, *.query...etc.
    void GetAssociatedFiles(const string& _filename);
    void SetUpSubwidgets();

    //accepting check boxes on left side of window
    QCheckBox* m_envCheckBox;
    QCheckBox* m_mapCheckBox;
    QCheckBox* m_queryCheckBox;
    QCheckBox* m_pathCheckBox;
    QCheckBox* m_debugCheckBox;

    //The actual displayed file names/paths
    QLabel* m_envFilename;
    QLabel* m_mapFilename;
    QLabel* m_queryFilename;
    QLabel* m_pathFilename;
    QLabel* m_debugFilename;
};

#endif
