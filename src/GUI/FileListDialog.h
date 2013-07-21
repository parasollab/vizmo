#ifndef FILELISTDIALOG_H_
#define FILELISTDIALOG_H_

#include <string>
using namespace std;

#include <QDialog>

class QGridLayout;
class QCheckBox;
class QLabel;
class QPushButton;

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
    void SetUpLayout();

    QGridLayout* m_layout;

    //accepting check boxes on left side of window
    QCheckBox* m_envCheckBox;
    QCheckBox* m_mapCheckBox;
    QCheckBox* m_queryCheckBox;
    QCheckBox* m_pathCheckBox;
    QCheckBox* m_debugCheckBox;

    //The bold, labels for each of the check boxes
    QLabel* m_envLabel;
    QLabel* m_mapLabel;
    QLabel* m_queryLabel;
    QLabel* m_pathLabel;
    QLabel* m_debugLabel;

    //The actual displayed file names/paths
    QLabel* m_envFilename;
    QLabel* m_mapFilename;
    QLabel* m_queryFilename;
    QLabel* m_pathFilename;
    QLabel* m_debugFilename;

    QPushButton* m_envButton;
    QPushButton* m_mapButton;
    QPushButton* m_queryButton;
    QPushButton* m_pathButton;
    QPushButton* m_debugButton;

    QPushButton* m_loadButton;
    QPushButton* m_cancelButton;
};

#endif

