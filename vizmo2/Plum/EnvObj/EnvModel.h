// EnvModel.h: interface for the CEnvModel class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_ENVMODEL_H_)
#define _ENVMODEL_H_

#include "GLModel.h"
#include "MultiBodyModel.h"
#include "EnvLoader.h"

namespace plum{

    class CEnvLoader;

    class CEnvModel : public CGLModel
    {
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

            //////////////////////////////////////////////////////////////////////
            // Action functions
            //////////////////////////////////////////////////////////////////////

            virtual bool BuildModels();
            virtual void Draw( GLenum mode );
            virtual void Select( unsigned int * index, vector<gliObj>& sel );
			virtual const string GetName() const { return "Environment"; }

			virtual void GetChildren( list<CGLModel*>& models ){ 
				typedef vector<CMultiBodyModel *>::iterator MIT;
				for(MIT i=m_pMBModel.begin();i!=m_pMBModel.end();i++)
					models.push_back(*i);
			}

			virtual list<string> GetInfo() const { 
				list<string> info; 
				info.push_back(string(m_envLoader->GetFileName()));

				char strsize[256]=""; 
				sprintf(strsize,"There are %d multibodies",m_pMBModel.size());
				info.push_back(string(strsize));


				return info;
			}

    private:
            CEnvLoader * m_envLoader;      //a pointer to CEnvLoader
            vector<CMultiBodyModel *> m_pMBModel; //an array of CMultiBodyModel

            double m_R;    ///radius
            Point3d m_COM; ///center of mass
    };

}//namespace plum

#endif // !defined(_ENVMODEL_H_)
