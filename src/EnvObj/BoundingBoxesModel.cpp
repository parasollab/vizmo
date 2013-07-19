// BoundingBoxModel.cpp: implementation of the CBoundingBoxesModel class.
//
//////////////////////////////////////////////////////////////////////

#include "BoundingBoxesModel.h"
#include <stdlib.h>
#include <cmath>
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBoundingBoxesModel::CBoundingBoxesModel()
{
    m_pBBXParser=NULL;
}

CBoundingBoxesModel::~CBoundingBoxesModel()
{
    m_pBBXParser=NULL;
}

//////////////////////////////////////////////////////////////////////
// GLModel Methods
//////////////////////////////////////////////////////////////////////

bool CBoundingBoxesModel::BuildModels(){
   if( m_pBBXParser==NULL ) return false;

   for(int i = 0; i<m_pBBXParser->getNumBBXs(); i++){
      BoundingBoxModel* cbbm = new BoundingBoxModel();
      cbbm->BuildModels();
      m_BBXModels.push_back(cbbm);
   }

   for(unsigned int i = 0; i<m_BBXModels.size(); i++){
      for(unsigned int j = i; j<m_BBXModels.size(); j++){
         if(i!=j){
            //BuildOverlapModel(m_BBXModels[i]->getBBX(), m_BBXModels[j]->getBBX());
         }
      }
   }

   return true;
}

void CBoundingBoxesModel::Draw( GLenum mode ){
   glEnable(GL_CULL_FACE);
   glCullFace(GL_BACK);
   typedef vector<BoundingBoxModel*>::iterator BIT;
   for(BIT bit=m_BBXModels.begin(); bit!=m_BBXModels.end(); bit++){
      (*bit)->Draw(mode);
   }
   glDisable(GL_CULL_FACE);
   /*for(BIT obit = m_BBXOverlaps.begin(); obit != m_BBXOverlaps.end(); obit++){
      (*obit)->DrawLines(mode);
   }*/
}

vector<string>
CBoundingBoxesModel::GetInfo() const {

  vector<string> info;
  info.push_back("Bounding Box");
  info.push_back("");
  string name = "Num Bounding Boxes=";
  ostringstream temp;
  temp<<m_BBXModels.size();
  info.push_back(name+temp.str());
  return info;
}

void CBoundingBoxesModel::Select( unsigned int * index, vector<GLModel*>& sel )
{
   if( index==NULL ) return;
   if( *index>=m_BBXModels.size() ) //input error
      return;
   m_BBXModels[index[0]]->Select(index+1,sel);
}

/*
CBoundingBoxesModel::overlapType CBoundingBoxesModel::classify(double min1, double max1, double min2, double max2){
   double a = max1-min1;
   double b = max2-min2;
   double apb = a+b;
   double amb = abs(a-b);
   double dist = abs((max1+min1)/2 - (max2+min2)/2);
   if(dist<=amb){
      if(min1<min2)return EP;
      else return EN;
   }
   else if(dist>=apb) return D;
   else return O;
}

vector<double> CBoundingBoxesModel::overlap(vector<double>& a, vector<double>& b){
   vector<double> ov = vector<double>(6);
   ov[0]=max(a[0],b[0]);
   ov[2]=max(a[2],b[2]);
   ov[4]=max(a[4],b[4]);
   ov[1]=min(a[1],b[1]);
   ov[3]=min(a[3],b[3]);
   ov[5]=min(a[5],b[5]);
   return ov;
}

void CBoundingBoxesModel::BuildOverlapModel(vector<double>& a, vector<double>& b){
   overlapType x = classify(a[0],a[1],b[0],b[1]);
   overlapType y = classify(a[2],a[3],b[2],b[3]);
   overlapType z = classify(a[4],a[5],b[4],b[5]);

   bool accept = false;
   if(!((x==D && y==D && z==D) || (x==EP && y==EP && z==EP) || (x==EN && y==EN && y==EN))){
      accept = true;
   }

   if(accept){
      vector<double> ov = overlap(a,b);
      BoundingBoxModel* cbbm = new BoundingBoxModel(m_BBXModels.size() + 1 + m_BBXOverlaps.size(), ov);
      cbbm->BuildModels();
      m_BBXOverlaps.push_back(cbbm);
   }
}
*/
