#if !defined( _PLUMObJECT_H_ )
#define _PLUMObJECT_H_

/**
 * This is an interface for object that is going to insert into Plum.
 */
namespace plum{

    class CGLModel;     //the interface for gl display
    class I_Loadable;   //the interface for loading files.

    class PlumObject {

    public:
        PlumObject(CGLModel * model=0, I_Loadable * load=0)
        { 
            m_pModel=model; m_pLoad=load; 
        }

        CGLModel *   getModel() const { return m_pModel; }
        I_Loadable * getLoader() const { return m_pLoad; }

    protected:

        CGLModel * m_pModel;
        I_Loadable * m_pLoad;
    };

}//namespace plum

#endif //_PLUMObJECT_H_

