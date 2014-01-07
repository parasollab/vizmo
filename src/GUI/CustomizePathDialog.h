/**********************************************************
 * A custom dialog for selection of path outline width and
 * gradient color scheme.
 **********************************************************/

#ifndef CUSTOMIZE_PATH_DIALOG_H
#define CUSTOMIZE_PATH_DIALOG_H

#include <QtGui>

using namespace std;

class CustomizePathDialog : public QDialog {

  Q_OBJECT

  using QDialog::paintEvent;

  public:
    CustomizePathDialog(QWidget* _parent = 0);

    QLineEdit* m_widthLineEdit;
    QLineEdit* m_modLineEdit;

  public slots:
    void RestoreDefault(); //restore to default cyan-green-yellow gradient

  private slots:
    void AddColor();
    void AcceptData();

  private:
    void paintEvent(QPaintEvent*);  //overridden to display gradient
    void SetUpDialog(QDialog* _dialog);

    vector<QColor> m_colors; //larger units of color in the gradient
    bool m_isDefault;
};

#endif // CUSTOMIZE_PATH_H
