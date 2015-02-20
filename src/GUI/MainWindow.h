#ifndef MAIN_WINDOW_H_
#define MAIN_WINDOW_H_

using namespace std;

#include <QtGui>

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

    MainWindow(QWidget* _parent = 0);

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Create the GUI and OpenGL scene.
    /// \return A \c bool indicating success or failure.
    bool Init();
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Initializes the Vizmo object if it is not already initialized.
    /// \return A \c bool indicating success or failure.
    bool InitVizmo();
    vector<string>& GetArgs() { return m_args; }
    void SetVizmoInit(bool _tf) { m_vizmoInit = _tf; }
    bool GetVizmoInit() { return m_vizmoInit; }
    GLWidget* GetGLWidget() { return m_gl; }
    AnimationWidget* GetAnimationWidget() { return m_animationWidget; }
    ModelSelectionWidget* GetModelSelectionWidget() {
      return m_modelSelectionWidget;
    }
    QDockWidget* GetDialogDock() { return m_dialogDock; }
    QTimer* GetMainClock() {return m_timer;}
    const QString& GetLastDir() const {return m_lastDir;}
    void SetLastDir(const QString& _dir) {m_lastDir = _dir;}
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Display a dialog in the dialog dock.
    /// \param[in] _dialog The dialog to display.
    void ShowDialog(QDialog* _dialog);
    void ResetDialogs(); ///< Close all dialog tabs and hide the dialog dock.
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Display a pop-up message.
    /// \param[in] _s The message to display.
    void AlertUser(string _s);
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Handle unexpected close events.
    /// \param[in] _event The close event to handle.
    void closeEvent(QCloseEvent* _event);

    MainMenu* m_mainMenu;   ///< The top menu bar. Manages options classes.
    ToolTabWidget* m_toolTabWidget; ///< The tool tab widget. Owns tool tabs.

  signals:

    void Alert(QString _s); ///< Signals a pop-up alert.

  private slots:

    void UpdateScreen(); ///< Instructs the GLWidget to redraw the scene.
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Hides the dialog calling this signal, and also the dialog dock if
    /// no other dialogs are visible.
    void HideDialogDock();
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Displays a pop-up message.
    /// \param[in] _s The message to display.
    void ShowAlert(QString _s);

  private:

    void CreateGUI();   ///< Create the various GUI elements.
    void SetUpLayout(); ///< Create the GUI layout.
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Handle keyboard events.
    /// \param[in] _e The keyboard event to handle.
    void keyPressEvent(QKeyEvent* _e);

    string m_command;  ///< Terminal command to make a new roadmap.
    bool m_setQS;  ///< Determines if values in window will need to be updated.
    bool m_setQG;  ///< Determines if values in window will need to be updated.
    string m_firstQryFile; ///< Holds the name of the first query file.

    vector<string> m_args; ///< User input arguments.
    bool m_vizmoInit;      ///< True if Vizmo is initialized.

    GLWidget* m_gl;                     ///< Displays the OpenGL scene.
    AnimationWidget* m_animationWidget; ///< Provides controls playable files.
    ModelSelectionWidget* m_modelSelectionWidget; ///< Displays model list.
    QDockWidget* m_dialogDock; ///< Displays dialogs in a tabbed dock.
    TextWidget* m_textWidget;  ///< Displays info on the current selection.

    QTimer* m_timer;   ///< The main rendering clock.
    QString m_lastDir; ///< The last directory used in a load/save operation.
};

#endif
