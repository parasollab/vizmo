// This class is for the text output area--(for displaying selected item info,
// VDebug comments, etc.   

#ifndef TEXTGUI_H_
#define TEXTGUI_H_ 

#include <QTextEdit> 

class TextGUI : public QTextEdit{

  Q_OBJECT

  public:
    TextGUI(QWidget* _parent = NULL, char* _name = NULL); 
    
  public slots:
    void SetText();  

}; 

#endif 
