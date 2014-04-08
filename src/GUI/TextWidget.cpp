/*TextWidget.cpp--implementation of text output for node cfgs, VDebug, etc.*/

#include "TextWidget.h"
#include <vector>
#include <string>

#include "Models/Vizmo.h"
#include "Models/DebugModel.h"

TextWidget::TextWidget(QWidget* _parent)
  :QTextEdit(_parent) {

  setFixedSize(205, 225);
  setReadOnly(true);
}

void
TextWidget::SetText(){
  const vector<Model*>& sel=GetVizmo().GetSelectedModels();

  typedef vector<Model*>::const_iterator SIT;

  //Holds whatever will go into the QTextEdit box: a selected node/group of
  //nodes' configurations, VDebug comments, etc.
  vector<string> infoVect;

  //If selecting one or more nodes, edges, etc., add their cfg(s) to the
  //infoVect
  if(sel.size() > 0) {
    for(SIT i=sel.begin(); i!=sel.end(); i++) {
      ostringstream oss;
      Model* gl = (Model *)(*i);
      gl->Print(oss);
      infoVect.push_back(oss.str());
    }
  }

  //If we are trying to view VDebug comments, add those to the infoVect
  if(GetVizmo().GetDebug()) {
    DebugModel* cdm = GetVizmo().GetDebug();
    vector<string> comments = cdm->GetComments();
    if(!comments.empty())
      infoVect.insert(infoVect.end(), comments.begin(), comments.end());
  }

  clear();

  string allInfo = "";
  typedef vector<string>::iterator VSI;
  for(VSI i=infoVect.begin(); i!=infoVect.end(); i++)
    allInfo += *i + "\n";

  QString qstr = QString::fromStdString(allInfo);
  setText(qstr);
}

