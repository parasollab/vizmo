/**********************************************************
 * A custom dialog for selection of path outline width and
 * gradient color scheme.
 **********************************************************/

#ifndef CUSTOMIZE_PATH_DIALOG_H
#define CUSTOMIZE_PATH_DIALOG_H

#include <QtGui>

class MainWindow;

using namespace std;

class CustomizePathDialog : public QDialog {

  Q_OBJECT

  using QDialog::paintEvent;

  public:
    CustomizePathDialog(MainWindow* _mainWindow = 0);
    QLineEdit* GetWidthLine() { return m_widthLineEdit; }
    QLineEdit* GetModLine() { return m_modLineEdit; }

  public slots:
    void RestoreDefault();

  private slots:
    void AddColor();
    void AcceptData();

  private:
    void paintEvent(QPaintEvent* _p);  //overridden to display gradient
    void SetUpDialog();

    QWidget* m_gradientRect;
    QLineEdit* m_widthLineEdit;
    QLineEdit* m_modLineEdit;
    vector<QColor> m_colors;
    bool m_isDefault;
};

#endif // CUSTOMIZE_PATH_H
