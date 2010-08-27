#include "MultiBodyModel.h"
#include "PlumState.h"
#include <src/vizmo2.h>

namespace plum{

   //////////////////////////////////////////////////////////////////////
   // Cons/Des
   CMultiBodyModel::CMultiBodyModel
      (unsigned int index, const CMultiBodyInfo & MBInfo)
      :m_MBInfo(MBInfo)
      {
         m_index=index;
         m_PolySize=0;
         m_pPoly=NULL;
         SetColor(0.4f,0.4f,0.4f,1);
         m_bFixed=true;
         m_R=0;
      }

   CMultiBodyModel::~CMultiBodyModel()
   {
      // delete [] m_pPoly;
      m_pPoly=NULL;
   }

   //////////////////////////////////////////////////////////////////////
   // Core
   //////////////////////////////////////////////////////////////////////
   bool CMultiBodyModel::BuildModels()
   {
      m_PolySize=m_MBInfo.m_cNumberOfBody;

      if( (m_pPoly=new CPolyhedronModel[m_PolySize])==NULL )
         return false; //Out of memory

      int iM=0;
      //build for each body and compute com
      for( iM=0; iM<m_PolySize; iM++ ){
         CBodyInfo & info=m_MBInfo.m_pBodyInfo[iM];
         //only build fixed, free body will not be built (when m_bFixed is set)
         if( !info.m_bIsFixed && m_bFixed==true )
            continue;

         m_pPoly[iM].SetBody(info);

         if( m_pPoly[iM].BuildModels()==false) {
            cout<<"Couldn't build models in Polyhedron class"<<endl;
            return false;
         }

         m_pPoly[iM].SetColor(info.rgb[0],info.rgb[1],info.rgb[2],1);

         m_COM[0]+=info.m_X;
         m_COM[1]+=info.m_Y;
         m_COM[2]+=info.m_Z;
      }

      for( int id=0;id<3;id++ ) m_COM[id]/=m_PolySize;
      //set position of multi-body as com
      tx()=m_COM[0]; ty()=m_COM[1]; tz()=m_COM[2];

      //compute radius
      m_R=0; //set radius to 0 and compute it later
      for( iM=0; iM<m_PolySize; iM++ ){
         CBodyInfo & info=m_MBInfo.m_pBodyInfo[iM];
         //only build fixed, free body will not be build (when m_bFixed is set)
         if( !info.m_bIsFixed && m_bFixed==true )
            continue;

         double dist=(Point3d(info.m_X,info.m_Y,info.m_Z)-m_COM).norm()
            +m_pPoly[iM].GetRadius();
         if( m_R<dist ) m_R=dist;
         //change to local coorindate of multibody
         m_pPoly[iM].tx()-=tx(); m_pPoly[iM].ty()-=ty(); m_pPoly[iM].tz()-=tz();
      }

      return true;
   }

   void CMultiBodyModel::Select( unsigned int * index, vector<gliObj>& sel )
   {
      if(index!=NULL)
         sel.push_back(this);
   }


   //Draw
   void CMultiBodyModel::Draw( GLenum mode )
   {
      glColor4fv(m_RGBA);
      glPushMatrix();
      glTranslated(m_pPoly[0].tx(), m_pPoly[0].ty(), m_pPoly[0].tz());
      glTransform();
#ifdef USE_PHANTOM
      if(fabs(m_pPoly[0].tx())>.01||fabs(m_pPoly[0].ty())>.01||fabs(m_pPoly[0].tz())>.01){
         glBegin(GL_LINES);
         glVertex3f(-10*GetRobot()->rotation_axis[0],-10*GetRobot()->rotation_axis[1],-10*GetRobot()->rotation_axis[2]);
         glVertex3f(10*GetRobot()->rotation_axis[0],10*GetRobot()->rotation_axis[1],10*GetRobot()->rotation_axis[2]);
         glEnd();
      }
#endif
      glTranslated(-m_pPoly[0].tx(), -m_pPoly[0].ty(), -m_pPoly[0].tz());
      for( int i=0;i<m_PolySize;i++ )
         m_pPoly[i].Draw( mode );
      glPopMatrix();
   }



   void CMultiBodyModel::DrawSelect()
   {
      glPushMatrix();
      glTranslated(m_pPoly[0].tx(), m_pPoly[0].ty(), m_pPoly[0].tz());
      glTransform();
      glTranslated(-m_pPoly[0].tx(), -m_pPoly[0].ty(), -m_pPoly[0].tz());

      for( int i=0;i<m_PolySize;i++ )
         m_pPoly[i].DrawSelect();
      glPopMatrix();
   }

   void CMultiBodyModel::SetRenderMode(int mode)
   {
      m_RenderMode=mode;
      for( int i=0;i<m_PolySize;i++ )
         m_pPoly[i].SetRenderMode(mode);

   }

   void CMultiBodyModel::SetColor(float r, float g, float b, float a)
   {
      CGLModel::SetColor(r,g,b,a);
      for( int i=0;i<m_PolySize;i++ )
         m_pPoly[i].SetColor(r,g,b,a);

   }

   const float *  CMultiBodyModel::GetColor() const{
      float c[3];
      c[0] = m_MBInfo.m_pBodyInfo[0].rgb[0];
      c[1] = m_MBInfo.m_pBodyInfo[0].rgb[1];
      c[2] = m_MBInfo.m_pBodyInfo[0].rgb[2];
      return c;
   }

   void CMultiBodyModel::Scale(double x, double y, double z)
   {
      CGLModel::Scale(x,y,z);

   }

   list<string> CMultiBodyModel::GetInfo() const 
   {	
      list<string> info; 
      ostringstream temp, os;
      if( m_bFixed ){
         info.push_back(string("Obstacle"));
         temp << "Position ( "<< tx()<<", "<<ty()<<", "<<tz()<<" )";
      }
      else {	
         info.push_back(string("Robot"));

         temp << m_PolySize;
         //string s = string("( ")+string(" link(s)) ");
         //info.push_back(s);
         temp << "Cfg ( ";
         info.push_back(string("Cfg ( "));
         for(int i=0; i< m_dof; i++){
            if(i==0) temp<<queryCfg[0];//temp<<tx() + queryPos[0];
            else if(i==1) temp<<queryCfg[1];//temp<<ty()+ queryPos[1];
            else if(i==2) temp<<queryCfg[2];//temp<<tz()+ queryPos[2];
            else
               temp<<queryCfg[i];
            if(i == m_dof-1)
               temp << " )";
            else
               temp << ", ";
         }

         //temp<<"There are "<<m_PolySize<<" bodies"<<endl;
         //info.push_back(temp.str());
         //temp<< queryCfg[0]<<", " << 
         //queryCfg[1]<<", " <<queryCfg[2]<<endl;
         //temp<< rx() <<", " <<ry() <<", " <<rz()<<endl;
      }
      info.push_back(temp.str());
      return info;
   }


   //configuration of robot is got from OBPRMView_Robot::getFinalCfg()
   //queryCfg[i] used to print the robot's cfg.
   void CMultiBodyModel::setCurrCfg(double * Cfg, int dof){
      m_dof = dof;
      queryCfg = new double [dof];
      m_dof = dof;
      for(int i=0; i< dof; i++){
         queryCfg[i] = Cfg[i];
      }

   }

   //position of robot is got from OBPRMView_Robot::getFinalCfg()
   //probably this function will be deleted...
   void CMultiBodyModel::setPos(double * cfg){
      queryPos = new double [3];
      for(int i=0; i<3; i++)
         queryPos[i] = cfg[i]; 
   }    
}//namespace plum
