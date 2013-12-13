// This class is for the text output area--(for displaying selected item info,
// VDebug comments, etc.

#ifndef TEXTWIDGET_H_
#define TEXTWIDGET_H_

#include <QTextEdit>

class TextWidget : public QTextEdit {

  Q_OBJECT

  public:
    TextWidget(QWidget* _parent = NULL);

  private slots:
    void SetText();

};

#endif
