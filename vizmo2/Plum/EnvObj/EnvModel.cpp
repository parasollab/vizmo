// EnvModel.cpp: implementation of the CEnvModel class.
//
//////////////////////////////////////////////////////////////////////

#include "EnvModel.h"

namespace plum {
    //////////////////////////////////////////////////////////////////////
    // Construction/Destruction
    //////////////////////////////////////////////////////////////////////
    
    CEnvModel::CEnvModel()
    {
        m_envLoader=NULL;
        m_MBSize=0;
        m_R=0;
    }
    
    bool CEnvModel::BuildModels(){
        if( m_envLoader==NULL ) return false;
        
        //create MutileBody Model
        m_MBSize = m_envLoader->GetNumberOfMultiBody();
        m_pMBModel=(CMultiBodyModel**)calloc(m_MBSize,sizeof(CMultiBodyModel*));
        if( m_pMBModel==NULL ) return false;
        
        //Build each
        Vector3d com;
        int iP;
        for( iP=0; iP<m_MBSize; iP++ ) {
            m_pMBModel[iP]=new CMultiBodyModel(iP,m_envLoader->GetMultiBodyInfo()[iP]);
            if( m_pMBModel[iP]==NULL ) return false;
            if( m_pMBModel[iP]->BuildModels()==false )
                return false;
            com=com+(m_pMBModel[iP]->GetCOM()-Point3d(0,0,0));
        }
        for( int id=0;id<3;id++ ) m_COM[id]=com[id]/m_MBSize;

        //compute radius
        m_R=0;
        for( iP=0; iP<m_MBSize; iP++ ) {
            double dist=(m_pMBModel[iP]->GetCOM()-m_COM).norm()
                        +m_pMBModel[iP]->GetRadius();
            if( m_R<dist ) m_R=dist;
        }

        return true;
    }
    
    void CEnvModel::Draw( GLenum mode )
    {
        if( mode==GL_SELECT && !m_EnableSeletion ) return;
        
        glLineWidth(1);
        for( int iP=0; iP<m_MBSize; iP++ ) {
            if( mode==GL_SELECT ) glPushName(iP);
            m_pMBModel[iP]->Draw( mode );
            if( mode==GL_SELECT ) glPopName();
        }
    }
    
    void CEnvModel::Select( unsigned int * index, vector<gliObj>& sel )
    {      
        //unselect old one       
        if( index==NULL ) return;
        if( *index>=(unsigned int)m_MBSize ) //input error
            return;
        m_pMBModel[index[0]]->Select(index+1,sel);
    }
    
}//namespace plum
