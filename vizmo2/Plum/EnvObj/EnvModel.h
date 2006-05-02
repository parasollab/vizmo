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

            //output info to std ouput
            //void DumpSelected();

    private:
            CEnvLoader * m_envLoader;      //a pointer to CEnvLoader
            CMultiBodyModel ** m_pMBModel; //an array of CMultiBodyModel
            int m_MBSize;                  //number of multibody

            double m_R;    ///radius
            Point3d m_COM; ///center of mass
    };

}//namespace plum

#endif // !defined(_ENVMODEL_H_)
