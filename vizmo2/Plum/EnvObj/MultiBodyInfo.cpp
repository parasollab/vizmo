#include "MultiBodyInfo.h"

namespace plum{

    //////////////////////////////////////////////////////////////////////
    //
    // CMultiBodyInfo
    //
    //////////////////////////////////////////////////////////////////////

    CMultiBodyInfo::CMultiBodyInfo()
    {
        m_cNumberOfBody = 0;
        m_pBodyInfo = NULL;
    }

    CMultiBodyInfo::CMultiBodyInfo( const CMultiBodyInfo & other )
    {
        *this = other;
    }

    CMultiBodyInfo::~CMultiBodyInfo()
    {
        m_cNumberOfBody = 0;
        delete [] m_pBodyInfo;
        m_pBodyInfo = NULL;
    }

    void CMultiBodyInfo::operator=( const CMultiBodyInfo & other )
    {
        m_cNumberOfBody = other.m_cNumberOfBody;
        m_pBodyInfo = new  CBodyInfo[m_cNumberOfBody];
        
        for( int iM=0; iM<m_cNumberOfBody; iM++ )
            m_pBodyInfo[iM] = other.m_pBodyInfo[iM];
    }

    ostream & operator<<( ostream & out, const CMultiBodyInfo & mbody ){
        out<< "- Number of bodies = " <<mbody.m_cNumberOfBody << endl;
        for( int iM=0; iM<mbody.m_cNumberOfBody; iM++ )
            out << "- Body["<< iM <<"] "<< endl << mbody.m_pBodyInfo[iM];
        return out;
    }


    //////////////////////////////////////////////////////////////////////
    //
    // CBodyInfo
    //
    //////////////////////////////////////////////////////////////////////

    CBodyInfo::CBodyInfo()
    {
        m_bIsFixed = false;
        m_Index    = -1;
        m_X=0;     m_Y=0;    m_Z=0;
        m_Alpha=0; m_Beta=0; m_Gamma=0;
        
        m_cNumberOfConnection =0;
        m_pConnectionInfo     =NULL;
        m_strModelDataFileName[0]='\0';
    }

    CBodyInfo::CBodyInfo( const CBodyInfo & other )
    {
        *this = other;
    }

    CBodyInfo::~CBodyInfo()
    {
        m_bIsFixed = false;
        m_Index    = -1;
        m_X=0;     m_Y=0;    m_Z=0;
        m_Alpha=0; m_Beta=0; m_Gamma=0;
        
        m_cNumberOfConnection =0;
        delete [] m_pConnectionInfo;
        m_pConnectionInfo=NULL;
    }

    void CBodyInfo::operator=( const CBodyInfo & other )
    {
        m_bIsFixed = other.m_bIsFixed;
        m_Index    = other.m_Index;
        m_X=other.m_X;
        m_Y=other.m_Y;
        m_Z=other.m_Z;
        m_Alpha=other.m_Alpha; 
        m_Beta=other.m_Beta; 
        m_Gamma=other.m_Gamma;
        
        m_strModelDataFileName[0]='\0';
        m_strModelDataFileName=other.m_strModelDataFileName;
        
        m_cNumberOfConnection =other.m_cNumberOfConnection;
        for( int iC=0; iC<m_cNumberOfConnection; iC++ )
            m_pConnectionInfo[iC] = other.m_pConnectionInfo[iC];
    }

    ostream & operator<<( ostream & out, const CBodyInfo & body ){
        
        out<<"-\t Data File = "<<body.m_strModelDataFileName<<endl
            <<"-\t Location = ("<<body.m_X<<", "<<body.m_Y<<", "<<body.m_Z<<")"<<endl
            <<"-\t Orientation = ("<<body.m_Alpha*360<<", "<<body.m_Beta*360<<", "<<body.m_Gamma*360<<")"<<endl
            <<"-\t Number of Connection = "<< body.m_cNumberOfConnection <<endl;
        for( int iC=0; iC<body.m_cNumberOfConnection; iC++ )
            out << "-\t Connection["<< iC <<"] "<< endl << body.m_pConnectionInfo[iC];
        
        return out;
    }

    //////////////////////////////////////////////////////////////////////
    //
    // ConnectionInfo
    //
    //////////////////////////////////////////////////////////////////////
    CConnectionInfo::CConnectionInfo()
    {
        m_preIndex = -1;
        m_nextIndex= -1;
    }

    CConnectionInfo::CConnectionInfo( const CConnectionInfo & other )
    {
        *this = other;
    }

    void CConnectionInfo::operator=( const CConnectionInfo & other )
    {
        m_preIndex = other.m_preIndex;
        m_nextIndex= other.m_nextIndex; 
    }

    ostream & operator<<( ostream & out, const CConnectionInfo & con ){
        
        cout<<"-\t\t Pre Index = "<<con.m_preIndex<<endl
            <<"-\t\t Next Index = "<<con.m_nextIndex<<endl;
        return out;
    }

}//end of plum

