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
        m_R=0;
    }
    
    bool CEnvModel::BuildModels(){
        if( m_envLoader==NULL ) return false;
        
        //create MutileBody Model
        int MBSize = m_envLoader->GetNumberOfMultiBody();
        m_pMBModel.reserve(MBSize);
        
        //Build each
        Vector3d com;
        int iP;
        for( iP=0; iP<MBSize; iP++ ) {
            CMultiBodyModel * pMBModel=new CMultiBodyModel(iP,m_envLoader->GetMultiBodyInfo()[iP]);
            if( pMBModel==NULL ) return false;
            if( pMBModel->BuildModels()==false )
                return false;
            com=com+(pMBModel->GetCOM()-Point3d(0,0,0));
            m_pMBModel.push_back(pMBModel);
        }
        for( int id=0;id<3;id++ ) m_COM[id]=com[id]/MBSize;
        
        //compute radius
        m_R=0;
        for( iP=0; iP<MBSize; iP++ ) {
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
        int MBSize=m_pMBModel.size();
        for( int iP=0; iP<MBSize; iP++ ) {
            if( mode==GL_SELECT ) glPushName(iP);
            m_pMBModel[iP]->Draw( mode );
            if( mode==GL_SELECT ) glPopName();
        }
    }
    
    void CEnvModel::Select( unsigned int * index, vector<gliObj>& sel )
    {      
        //unselect old one       
        if( index==NULL ) return;
        if( *index>=m_pMBModel.size() ) //input error
            return;
        m_pMBModel[index[0]]->Select(index+1,sel);
    }
    
    list<string> CEnvModel::GetInfo() const { 
        list<string> info; 
        info.push_back(string(m_envLoader->GetFileName()));
        
        {
            ostringstream temp;
            temp<<"There are "<<m_pMBModel.size()<<" multibodies";
            info.push_back(temp.str());
        }
        return info;
    }
    
}//namespace plum
