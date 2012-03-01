// Cfg.h: interface for the CCfg class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SIMPLECFG_H__2A4319F9_0BA6_4F63_85B4_AE6226B6BC69__INCLUDED_)
#define AFX_SIMPLECFG_H__2A4319F9_0BA6_4F63_85B4_AE6226B6BC69__INCLUDED_

#ifdef WIN32
#include <windows.h>
#endif

#include <GL/gl.h>
#include <GL/glut.h>
#include <iostream>
#include <string>

#include "src/EnvObj/Robot.h"

using namespace std;

namespace plum{

   //////////////////////////////////////////////////////////////////////
   //
   //      CCfg 
   //      To support articulated robots.
   //
   //////////////////////////////////////////////////////////////////////
   class CCModelBase;
   class CCfg : public CGLModel  
   {
      friend ostream & operator<<( ostream & out, const CCfg & cfg );
      friend istream & operator>>( istream &  in, CCfg & cfg );

      public:
      //type for the shape of node representation
      enum Shape { Robot, Box, Point};

      //////////////////////////////////////////
      // DOF variable
      /////////////////////////////////////////

      static int dof;

      /////////////////////////
      // for collison detection 
      //////////////////////////

      bool coll;

      //testing:
      OBPRMView_Robot* m_robot;

      //////////////////////////////////////////////////////////////////////
      //      Constructor/Destructor
      //////////////////////////////////////////////////////////////////////

      CCfg();
      ~CCfg();

			CCfg(const CCfg& _cfg);

      bool operator==( const CCfg & other ) const;
      void Set( int index , OBPRMView_Robot* robot, CCModelBase* cc);

      virtual void SetColor( float r, float g, float b, float a ){
         CGLModel::SetColor(r,g,b,a);
         //m_robot->SetColor(r,g,b,a);
      }
      virtual void SetRenderMode( int mode ){ //cout << "setting render mode" << endl;
         m_RenderMode=mode; }

         void DrawRobot();
         void DrawBox();
         //void DrawBox(double scale);
         void DrawPoint();

         //////////////////////////////////////////////////////////////////////
         bool BuildModels(){ /*do nothing*/ return true; }
         void Draw( GLenum mode );
         void DrawSelect();



         const string GetName() const {

            ostringstream temp;
            temp<<"Node"<<m_index;
            return temp.str(); 
         }
         list<string> GetInfo() const;
         list<string> GetNodeInfo() const;

         //////////////////////////////////////////////////////////////////////
         //      Access Method
         //////////////////////////////////////////////////////////////////////
         static CCfg & InvalidData(){return m_InvalidCfg;}
         virtual void Dump();

         int GetIndex() const {return m_index;}
         int GetCC_ID();
         vector<double> GetDataCfg() {
            return dofs;}
         //function accessed to write data into a new .map file
         vector<double> GetUnknown(){
            vector<double> v;
            v.push_back(m_Unknow1); 
            v.push_back(m_Unknow2);
            v.push_back(m_Unknow3);
            return v;
         }
         static int GetDof(void) { return dof; }
         CCModelBase * GetCC() const { return m_CC; }
				 OBPRMView_Robot* GetRobot() const {return m_robot;}
			

         static void SetDof(int d) { dof = d; }
         void SetShape(Shape shape){ m_Shape=shape; }
         //set new values to dofs vector
         void setCfg(vector<double> newCfg);
         void setIndex(int i);
         void setCCModel(CCModelBase* cc);

         //function accessed from gliDataStructure
         void CopyCfg() {
            ObjCfg.clear();
            ObjCfg.assign(dofs.begin(), dofs.end());
         }

         ///Translation
         //@{
         double& tx(){ ObjName="Node";  CopyCfg(); return dofs[0]; }
         double& ty(){ return dofs[1]; }
         double& tz(){ return dofs[2]; }
         const double& tx() const { return dofs[0]; }
         const double& ty() const { return dofs[1]; }
         const double& tz() const { return dofs[2]; }
         ///@}


         //////////////////////////////////////////////////////////////////////
         //      Protected Method & Data
         //////////////////////////////////////////////////////////////////////
      protected:

         vector<double> dofs;
         int m_index;
         double m_Unknow1, m_Unknow2, m_Unknow3;
         //original declaration:
         //	OBPRMView_Robot* m_robot;
         Shape m_Shape;
         CCModelBase * m_CC;       

         //////////////////////////////////////////////////////////////////////
         //      Private Method & Data
         //////////////////////////////////////////////////////////////////////
      private:
         static CCfg m_InvalidCfg;
         //tmp
         friend class CSimpleEdge;

   };

   //////////////////////////////////////////////////////////////////////
   //
   //      CEdge
   //
   //////////////////////////////////////////////////////////////////////

   class CSimpleEdge : public CGLModel
   {
      friend ostream & operator<<( ostream & out, const CSimpleEdge & edge );
      friend istream & operator>>( istream &  in, CSimpleEdge & edge );

      public:

      //////////////////////////////////////////////////////////////////////
      //      Constructor/Destructor
      //////////////////////////////////////////////////////////////////////

      CSimpleEdge();
      CSimpleEdge(double weight);
      ~CSimpleEdge();

      bool operator==( const CSimpleEdge & other );
      //void Set(const Point3d& p1, const Point3d& p2){ m_s=p1; m_e=p2; }
      void Set(int id, CCfg * c1, CCfg * c2, OBPRMView_Robot* _robot=NULL); 

      //////////////////////////////////////////////////////////////////////
      bool BuildModels(){ /*do nothing*/ return true; }
      void Draw( GLenum mode );
      void DrawSelect();

      const string GetName() const { 

         ostringstream temp;
         temp<<"Edge"<<m_ID;
         return temp.str(); 

      }
      list<string> GetInfo() const;
      vector<int> GetEdgeNodes();

      void SetCfgShape(char _shape) {
        
        switch (_shape) {

          case 'r':
            m_cfgShape = CCfg::Robot;
            break;
      
          case 'b':
            m_cfgShape = CCfg::Box;
            break;

          case 'p':
            m_cfgShape = CCfg::Point;
            break;
  
          default:
            break;

        }



      }

      //////////////////////////////////////////////////////////////////////
      //      Access Method
      //////////////////////////////////////////////////////////////////////
      int & GetLP(){ return m_LP; }
      double & GetWeight(){ return m_Weight; }
      double & Weight(){ return m_Weight; }
      int GetID() { return m_ID; }
      const CCfg & GetStartCfg() { return m_s; }
      //////////////////////////////////////////////////////////////////////
      //      Protected Method & Data
      //////////////////////////////////////////////////////////////////////
      protected:

      //Point3d m_s, m_e;
      CCfg m_s, m_e;

      int    m_LP;
      double m_Weight;
      int m_ID;
    
      CCfg::Shape m_cfgShape;

      //allow an edge to contain a sequence of cfgs
      vector <CCfg> m_IntermediateCfgs;

   };

}//namespace plum

#endif 
