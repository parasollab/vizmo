/*TextGUI.cpp--implementation of text output for node cfgs, VDebug, etc.*/ 

#include "TextGUI.h"
#include <vector>
#include <string>
#include <QString>
#include "vizmo2.h"
#include "EnvObj/DebugModel.h"

using namespace std; 

TextGUI::TextGUI(QWidget* _parent)
  :QTextEdit(_parent){
    
  setFixedSize(205, 225);
  setReadOnly(true); 
}

void 
TextGUI::SetText(){
  vector<gliObj>& sel=GetVizmo().GetSelectedItem();

  typedef vector<gliObj>::iterator SIT;

  //Holds whatever will go into the QTextEdit box: a selected node/group of
  //nodes' configurations, VDebug comments, etc. 
  vector<string> infoVect;

  //If selecting one or more nodes, edges, etc., add their cfg(s) to the
  //infoVect
  if(sel.size() > 0){
    for(SIT i=sel.begin(); i!=sel.end(); i++){
      CGLModel* gl = (CGLModel *)(*i);
      if(gl != NULL){
        vector<string> info = gl->GetInfo();
        if(info.size() > 0)
          infoVect.insert(infoVect.end(), info.begin(), info.end());
      }
    }
  }

  //If we are trying to view VDebug comments, add those to the infoVect
  if(GetVizmo().GetDebug()!=NULL){ 
    DebugModel* cdm = (DebugModel*)GetVizmo().GetDebug()->GetModel();
    if(cdm != NULL){
      vector<string> comments = cdm->GetComments();
      if(!comments.empty())
        infoVect.insert(infoVect.end(), comments.begin(), comments.end());
    }
  }

  clear(); 

  string allInfo = ""; 
  typedef vector<string>::iterator VSI;
  for(VSI i=infoVect.begin(); i!=infoVect.end(); i++)
    allInfo += *i + "\n"; 

  QString qstr = QString::fromStdString(allInfo); 
  setText(qstr); 
}

