#ifndef MULTIBODYMODEL_H_
#define MULTIBODYMODEL_H_

#include "MultiBodyInfo.h"
#include "GLModel.h"
#include "PolyhedronModel.h"

namespace plum{

  class MultiBodyModel : public CGLModel {
    public:
      //////////////////////////////////////////////////////////////////////
      // Cons/Des
      MultiBodyModel(const CMultiBodyInfo & MBInfo);
      virtual ~MultiBodyModel();

      //////////////////////////////////////////////////////////////////////
      // Core
      //////////////////////////////////////////////////////////////////////
      virtual bool BuildModels();
      virtual void Select(unsigned int * index, vector<gliObj>& sel);

      //Draw
      virtual void Draw( GLenum mode );
      virtual void DrawSelect();

      //set wire/solid/hide
      virtual void SetRenderMode( int mode );
      virtual void SetColor(float r, float g, float b, float a);
      const float * GetColor() const;

      virtual const string GetName() const { return "MultiBody"; }

      virtual void GetChildren( list<CGLModel*>& models ){ 
	for(size_t i=0; i<m_poly.size(); i++ )
	  models.push_back(&m_poly[i]);
      }

      virtual list<string> GetInfo() const;

      //used to print the confg. of the MultiBody
      void SetCfg(vector<double>& _cfg);
      virtual void Scale(double x, double y, double z);


      //////////////////////////////////////////////////////////////////////
      // Access
      //////////////////////////////////////////////////////////////////////
      void SetAsFree(bool free=true){m_bFixed = !free;}
      
      double GetRadius() const{return m_radius;}
      const Point3d& GetCOM() const{return m_COM;}
      vector<CPolyhedronModel>& GetPolyhedron(){return m_poly;}
      const CMultiBodyInfo& GetMBinfo(){return m_MBInfo;}
      bool IsFixed() const{return m_bFixed;}

      //public variables
      double posX, posY, posZ;
      list<CGLModel *> objlist; // to have access from glitransTool class
    
    private:
      const CMultiBodyInfo & m_MBInfo; //a reference to the CMultiBodyInfo
      vector<CPolyhedronModel> m_poly;

      bool m_bFixed; //is this multibody fixed. i.e obstacle
      double m_radius; //Radius
      Point3d m_COM; // center of mass

      vector<double> m_cfg;
  };
}//namespace plum

#endif //MULTIBODYMODEL_H_

