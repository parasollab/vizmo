#ifndef FILE_LIST_DIALOG_H_
#define FILE_LIST_DIALOG_H_

using namespace std;

#include <QtGui>
#include <QDialog>

class QCheckBox;
class QLabel;

////////////////////////////////////////////////////////////////////////////////
/// \brief Provides a file selection pop-up for loading data files.
////////////////////////////////////////////////////////////////////////////////
class FileListDialog : public QDialog {

  Q_OBJECT

  public:

    FileListDialog(const vector<string>& _filename, QWidget* _parent,
        Qt::WindowFlags _f = Qt::WindowFlags());

  public slots:

    void ChangeEnv();     ///< Select an Environment file to load.
    void ChangeMap();     ///< Select a Map file to load.
    void ChangeQuery();   ///< Select a Query file to load.
    void ChangePath();    ///< Select a Path file to load.
    void ChangeDebug();   ///< Select a debug file to load.
    void ChangeXML();     ///< Select an XML file to load.
    void Accept();        ///< Load selected files.
    void PathChecked();   ///< Prohibits loading a debug file with a path file.
    void DebugChecked();  ///< Prohibits loading a path file with a debug file.

  private:

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Search an XML file for a parameter.
    /// \param[in] _filename The XML file to search.
    /// \param[in] _key The parameter label to search for.
    /// \return The parameter value or an empty string if not found.
    void ParseXML(const string& _xmlfile, string& _envfile, string& _queryfile);

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Locate all possible filenames related to the input files.
    /// \param[in] _filename The list of input files.
    void GetAssociatedFiles(const vector<string>& _filename);

    //accepting check boxes on left side of window
    QCheckBox* m_envCheckBox;   ///< Indicate whether an env will be loaded.
    QCheckBox* m_mapCheckBox;   ///< Indicate whether a map will be loaded.
    QCheckBox* m_queryCheckBox; ///< Indicate whether a query will be loaded.
    QCheckBox* m_pathCheckBox;  ///< Indicate whether a path will be loaded.
    QCheckBox* m_debugCheckBox; ///< Indicate whether a debug file will be loaded.

    //The actual displayed file names/paths
    QLabel* m_envFilename;   ///< The environment file to load.
    QLabel* m_mapFilename;   ///< The map file to load.
    QLabel* m_queryFilename; ///< The query file to load.
    QLabel* m_pathFilename;  ///< The path file to load.
    QLabel* m_debugFilename; ///< The debug file to load.
    QLabel* m_xmlFilename;   ///< The XML file to load.
};

#endif
