// This class is for the text output area--(for displaying selected item info,
// VDebug comments, etc.   

#ifndef TEXTGUI_H_
#define TEXTGUI_H_ 

#include <q3textview.h> 

class TextGUI : public Q3TextView{

  Q_OBJECT

  public:
    TextGUI(QWidget* _parent = NULL, char* _name = NULL); 
    
  public slots:
    void SetText();  

}; 

#endif 
