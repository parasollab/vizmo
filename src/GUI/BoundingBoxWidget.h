#ifndef BOUNDINGBOXWIDGET_H_
#define BOUNDINGBOXWIDGET_H_

#include <QtGui>

class BoundingBoxWidget : public QWidget {

  Q_OBJECT

  public:
    BoundingBoxWidget(QWidget* _parent);

  public slots:
    void ChangeBoxTo2D();
    void SetBoundary();

  private:
    void ShowCurrentValues();

    bool m_is2D;
    QCheckBox* m_checkIs2D;
    QLineEdit* m_lineXMin;
    QLineEdit* m_lineXMax;
    QLineEdit* m_lineYMin;
    QLineEdit* m_lineYMax;
    QLineEdit* m_lineZMin;
    QLineEdit* m_lineZMax;
};

#endif
