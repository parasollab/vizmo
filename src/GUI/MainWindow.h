#ifndef MAIN_WINDOW_H_
#define MAIN_WINDOW_H_

using namespace std;

#include <QtGui>
#include <QMainWindow>

class QAction;
class QGridLayout;
class QHBoxLayout;
class QVBoxLayout;
class QTimer;

class AnimationWidget;
class VizmoScreenShotGUI;
class ModelSelectionWidget;
class VizmoAttributeSelectionGUI;
class TextWidget;
class ToolTabWidget;
class MainMenu;
class GLWidget;

#define M_MAX 999.99
#define M_MIN -999.99
#define STEP 0.1
#define D_MAX 1
#define D_MIN 0

class MainWindow;
MainWindow*& GetMainWindow();

////////////////////////////////////////////////////////////////////////////////
/// \brief MainWindow serves as the primary application container.
/// \details This class creates the GUI and Vizmo object. It also manages
/// some aspects of the GUI (such as the dialog dock, alert pop-up, and main
/// timer) and resetting/reinitializing the system with new input.
////////////////////////////////////////////////////////////////////////////////
class MainWindow : public QMainWindow {

  Q_OBJECT

  public:

    ///@name Construction
    ///@{

    MainWindow(const vector<string>& _filenames);

    ///@}
    ///@name Application State
    ///@{

    vector<string>& GetArgs() {return m_args;}
    void SetVizmoInit(bool _i) {m_vizmoInit = _i;}
    bool GetVizmoInit() {return m_vizmoInit;}

    const QString& GetLastDir() const {return m_lastDir;}
    void SetLastDir(const QString& _dir) {m_lastDir = _dir;}

    ///@}
    ///@name Public Subwidgets and Components
    ///@{

    MainMenu* m_mainMenu{nullptr};           ///< The top menu bar.
    ToolTabWidget* m_toolTabWidget{nullptr}; ///< The tool tab widget.

    GLWidget* GetGLWidget() {return m_gl;}
    AnimationWidget* GetAnimationWidget() {return m_animationWidget;}
    ModelSelectionWidget* GetModelSelectionWidget() {
      return m_modelSelectionWidget;
    }

    QTimer* GetMainClock() {return m_timer;}

    ///@}
    ///@name Dialog Dock
    ///@{

    QDockWidget* GetDialogDock() {return m_dialogDock;}

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Display a dialog in the dialog dock.
    /// \param[in] _dialog The dialog to display.
    void ShowDialog(QDialog* _dialog);

    void ResetDialogs(); ///< Close all dialog tabs and hide the dialog dock.

    ///@}
    ///@name Alerts
    ///@{

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Display a pop-up message.
    /// \param[in] _s The message to display.
    void AlertUser(string _s);

    ///@}

  public slots:

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Initializes the Vizmo object if it is not already initialized.
    /// \return A \c bool indicating success or failure.
    bool InitVizmo();

  signals:

    void Alert(QString _s); ///< Signals a pop-up alert.

  private slots:

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Hides the dialog calling this signal, and also the dialog dock if
    /// no other dialogs are visible.
    void HideDialogDock();

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Displays a pop-up message.
    /// \param[in] _s The message to display.
    void ShowAlert(QString _s);

  private:

    ///@name Construction Helpers
    ///@{

    void CreateGUI();   ///< Create the various GUI elements.

    void SetUpLayout(); ///< Create the GUI layout.

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Handle unexpected close events.
    /// \param[in] _event The close event to handle.
    virtual void closeEvent(QCloseEvent* _event) override;

    ///@}
    ///@name Internal State
    ///@{

    string m_command;        ///< Terminal command to make a new roadmap.
    string m_firstQryFile;   ///< Holds the name of the first query file.

    vector<string> m_args;   ///< User input arguments.
    bool m_vizmoInit{false}; ///< True if Vizmo is initialized.

    GLWidget* m_gl{nullptr};            ///< Displays the OpenGL scene.
    AnimationWidget* m_animationWidget{nullptr}; ///< Controls playable files.
    ModelSelectionWidget* m_modelSelectionWidget{nullptr}; ///< Lists models.
    QDockWidget* m_dialogDock{nullptr}; ///< Displays dialogs in a tabbed dock.
    TextWidget* m_textWidget{nullptr};  ///< Displays info on current selection.

    QTimer* m_timer{nullptr};   ///< The main rendering clock.
    QString m_lastDir;     ///< The last directory used in a load/save operation.

    ///@}
};

#endif
