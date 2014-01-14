#ifndef BOUNDINGSPHEREWIDGET_H_
#define BOUNDINGSPHEREWIDGET_H_

#include <QtGui>

class BoundingSphereWidget : public QWidget {

  Q_OBJECT

  public:
    BoundingSphereWidget(QWidget* _parent);

  public slots:
    void SetBoundary();

  private:
    void ShowCurrentValues();

    QLineEdit* m_lineX;
    QLineEdit* m_lineY;
    QLineEdit* m_lineZ;
    QLineEdit* m_lineR;
};

#endif
