/**********************************************************
 * A custom dialog for selection of path outline width and
 * gradient color scheme.
 **********************************************************/

#ifndef CUSTOMIZE_PATH_DIALOG_H
#define CUSTOMIZE_PATH_DIALOG_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QPixmap>
#include <QtGui/QPainter>
#include <QtGui/QColor>
#include <QtGui/QBrush>
#include <QtGui/QGradient>
#include <QtGui/QColorDialog>
#include <QtGui/QFrame>

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

    QDialogButtonBox* m_okayCancel;
    QLabel* m_gradientLabel;
    QLabel* m_instructions1;
    QLabel* m_instructions2;
    QLabel* m_widthLabel;
    QLabel* m_modLabel;
    QPushButton* m_addColorButton;
    QPushButton* m_clearGradButton;
    QFrame* m_line;
    QColorDialog* m_addColorDialog;
    QBrush* m_brush;
    vector<QColor> m_colors; //larger units of color in the gradient
    QGradient* m_gradient;
    bool m_isDefault;
};

#endif // CUSTOMIZE_PATH_H

