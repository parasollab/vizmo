// EnvModel.h: interface for the CEnvModel class.
//
//////////////////////////////////////////////////////////////////////

#ifndef ENVMODEL_H_
#define ENVMODEL_H_

#include "GLModel.h"
#include "MultiBodyModel.h"
#include "EnvLoader.h"
#include "EnvObj/BoundaryModel.h"

namespace plum{

  class CEnvLoader;

  class CEnvModel : public CGLModel {
    public:
      //////////////////////////////////////////////////////////////////////
      // Constructor/Destructor
      //////////////////////////////////////////////////////////////////////
      CEnvModel();

      //////////////////////////////////////////////////////////////////////
      // Access functions
      //////////////////////////////////////////////////////////////////////
      void SetEnvLoader( CEnvLoader * envLoader ){ m_envLoader = envLoader; }
      double GetRadius() const { return m_R; }
      const Point3d& GetCOM() const { return m_COM; }
      void ChangeColor(); //changes object's color randomly
      vector<vector<CPolyhedronModel> > getPoly();
      vector<MultiBodyModel *> getMBody();

      void DeleteMBModel(MultiBodyModel *mbl);

      void AddMBModel(CMultiBodyInfo newMBI);

      //////////////////////////////////
      // TESTING to save Env. file
      /////////////////////////////////
      bool SaveFile(const char *filename);

      //////////////////////////////////////////////////////////////////////
      // Action functions
      //////////////////////////////////////////////////////////////////////

      virtual bool BuildModels();
      virtual void Draw( GLenum mode );
      virtual void Select( unsigned int * index, vector<gliObj>& sel );
      virtual const string GetName() const { return "Environment"; }
      virtual void GetChildren( list<CGLModel*>& models ){ 
        typedef vector<MultiBodyModel *>::iterator MIT;
        for(MIT i=m_pMBModel.begin();i!=m_pMBModel.end();i++){
          if((*i)->IsFixed())
            models.push_back(*i);
        }
        models.push_back(m_boundary);
      }
      virtual vector<string> GetInfo() const;

      BoundaryModel* GetBoundary() {return m_boundary;}

    private:
      CEnvLoader * m_envLoader;      //a pointer to CEnvLoader
      vector<MultiBodyModel *> m_pMBModel; //an array of MultiBodyModel

      double m_R;    ///radius
      Point3d m_COM; ///center of mass

      BoundaryModel* m_boundary;
  };

}//namespace plum

#endif // !defined(_ENVMODEL_H_)
