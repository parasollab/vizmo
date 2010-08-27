#include "addObjDialog.h"



#include <sys/types.h>
#include <dirent.h>
#include <cerrno>


///////////////////////////////////////////////////////////////////////////////
// QT Headhers

#include <qlabel.h>
#include <qstring.h>
#include <qmessagebox.h>
#include <qfile.h>
#include <qfiledialog.h>
#include <qfileinfo.h>
#include <qlayout.h>
//#include <qgrid.h>
#include <qpushbutton.h>
#include <QtGui>

//Added by qt3to4:
#include <q3grid.h>

///////////////////////////////////////////////////////////////////////////////
// Images
#include "icon/folder.xpm"
#include "icon/eye.xpm"

AddObjDial::AddObjDial
(QWidget *parent, const char *name, Qt::WFlags f)
:QDialog(parent,name,true,f)
{

  r = true;
  this->setPaletteBackgroundColor(QColor(238, 238, 238));
}

bool AddObjDial::create(){

  fn=QFileDialog::getOpenFileName(this, "File name", QString::null,"BYU Files (*.g)");
  
  if(!fn.isEmpty() ){

    QGridLayout *g = new QGridLayout(this, 5, 3);
    
    QVBoxLayout * controls = new QVBoxLayout();
    controls->setSpacing(1);

    controls->addWidget(new QLabel("<b>File to be added</b>:", this));
    controls->addWidget(new QLabel(fn, this));

    g->addMultiCellLayout(controls, 0, 2, 0, 2);    
    
    QPushButton *go = new QPushButton("Done",this);
    go->setPaletteBackgroundColor(QColor(212, 212, 212));
    connect(go,SIGNAL(clicked()),this,SLOT(updateData()));
    g->addWidget(go, 4,1);
    
  }

  else
    r = false;

  return r;
}

void AddObjDial::updateData(){
  vector<PlumObject*>& objs=GetVizmo().GetPlumObjects();
  typedef vector<PlumObject*>::iterator POI;
  for(POI i=objs.begin();i!=objs.end();i++){

    CGLModel * m=(*i)->getModel();
    if(m == NULL) continue;
    if(m->GetName() == "Environment"){
      CEnvLoader* envLoader=(CEnvLoader*)(*i)->getLoader();

      int nMB = envLoader->GetNumberOfMultiBody();

      ///////////////////////////////////////////////////////////////
      //  Get MultiBody object                                     //
      //  1. create tmp MB for the new object (mbiTmp)             //
      //  2. copy previous MBInfo to mbiNew and add mbiTmp         //
      //  3. call CEnvLoader::SetNewMultiBodyInfo(mbiNew)          //
      ///////////////////////////////////////////////////////////////

      const CMultiBodyInfo* MBI = envLoader->GetMultiBodyInfo();
      createMBInfo();

      envLoader->IncreaseNumMB();

      mbiNew = new CMultiBodyInfo [envLoader->GetNumberOfMultiBody()];

      int idx=0;
      for(; idx<nMB; idx++)
	mbiNew[idx] = MBI[idx];

      mbiNew[idx] = mbiTmp[0];

      envLoader->SetNewMultiBodyInfo(mbiNew);

      //////////////////////////////////////////////////////////
      //  Recreate MBModel
      //////////////////////////////////////////////////////////
      CEnvModel* env=(CEnvModel*)(*i)->getModel();

      createMBModel(mbiNew[idx], env);

      //commands();

    }// if Environment

  }// for PlumObject 

  this->close();
}

void AddObjDial::createMBInfo(){
  //////////////////////////////////////////////
  //create "directory" if doesn't exist
  //then copy in "directory" new *.g
  // NOTE:: this function is called here to 
  // get the correct value for m_strFileName
  // which is used in vizmo2 :: saveEnv()
  /////////////////////////////////////////////

  commands();

  /// crerate new MBI

  mbiTmp = new CMultiBodyInfo[1];
  
  mbiTmp[0].m_NumberOfConnections = 0;
  mbiTmp[0].m_cNumberOfBody = 1;
  mbiTmp[0].m_active = 0;
  mbiTmp[0].m_pBodyInfo = new CBodyInfo[1];
  
  mbiTmp[0].m_pBodyInfo[0].m_bIsFixed = true;
  mbiTmp[0].m_pBodyInfo[0].m_IsBase = true;
  mbiTmp[0].m_pBodyInfo[0].m_Index = 0;
  mbiTmp[0].m_pBodyInfo[0].m_isNew = 1;
  mbiTmp[0].m_pBodyInfo[0].m_strFileName = m_fileName;
  mbiTmp[0].m_pBodyInfo[0].m_strModelDataFileName = fn.toStdString();
  mbiTmp[0].m_pBodyInfo[0].m_X = 0; 
  mbiTmp[0].m_pBodyInfo[0].m_Y = 0;  
  mbiTmp[0].m_pBodyInfo[0].m_Z = 0; 
  mbiTmp[0].m_pBodyInfo[0].m_Alpha = 0; 
  mbiTmp[0].m_pBodyInfo[0].m_Beta = 0; 
  mbiTmp[0].m_pBodyInfo[0].m_Gamma = 0;
  mbiTmp[0].m_pBodyInfo[0].doTransform();  
  mbiTmp[0].m_pBodyInfo[0].rgb[0] = 0.5;
  mbiTmp[0].m_pBodyInfo[0].rgb[1] = 0.5;
  mbiTmp[0].m_pBodyInfo[0].rgb[2] = 0.5;

}


void AddObjDial::createMBModel(CMultiBodyInfo &mbi, CEnvModel* env ){

  env->AddMBModel(mbi);

}

void AddObjDial::commands(){
  string s_path, s_tmp;

  if(GetVizmo().getMapFileName() != "")
	s_tmp = GetVizmo().getMapFileName();
  else
  	s_tmp = GetVizmo().getEnvFileName();

  s_path = getSubstr(s_tmp, '/', 2);

  string sub_string = s_tmp.substr(position);
  sub_string = getSubstr(sub_string, '.', 3);
  
  const char* f;
  f = sub_string.c_str();
 
  string dirname = (string)f+"_newFiles";
  const char* c_dirname = dirname.c_str();
 
  string command = "mkdir " + s_path+dirname;
  const char * comm = command.c_str();

  bool FOUND = false;

  DIR * dirp = opendir(s_path.c_str());
  struct dirent *dp;
  while (dirp) {
    errno = 0;
    if ( (dp=readdir(dirp)) != NULL) {
      
      if (strcmp(dp->d_name, c_dirname) == 0) {
	FOUND = true;
	
	closedir(dirp);
	break;
      } 
    }
    else
      if(!FOUND){
	closedir(dirp);
	break;	
      }
  }

  string s_copy = "cp "+fn.toStdString()+ " " +s_path + dirname;
  int value;
  if(!FOUND){
    
    value = system(comm); 
  }
  system(s_copy.c_str());
  
  m_fileName = "./"+dirname+"/"+getSubstr(fn.toStdString(), '/', 1);

}

string AddObjDial::getSubstr(string s, char c, int i){

  const char* st;
  st = s.c_str();
  char *pos = strrchr(st, c);
  position = pos-st+1;
  string subS;
  if(i==1)
    subS = s.substr(position);
  else if(i==2)
    subS = s.substr(0, position);
  else
    subS = s.substr(0, position-1);
  return subS;

}


