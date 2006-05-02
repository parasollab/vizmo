#ifndef _MultiBodyInfo_H_
#define _MultiBodyInfo_H_

#include <iostream>
#include <string>
using namespace std;

namespace plum{

    class CBodyInfo;
    class CConnectionInfo;

    //////////////////////////////////////////////////////////////////////
    //
    // CMultiBodyInfo
    //
    //////////////////////////////////////////////////////////////////////
    class CMultiBodyInfo {

    public:
            //////////////////////////////////////////////////////////////////////
            // Constructor/Destructor
            //////////////////////////////////////////////////////////////////////
            CMultiBodyInfo();
            CMultiBodyInfo( const CMultiBodyInfo & other );
            ~CMultiBodyInfo();
            void operator=( const CMultiBodyInfo & other );
            friend ostream & operator<<( ostream & out, const CMultiBodyInfo & mbody );

            int m_cNumberOfBody;
            CBodyInfo * m_pBodyInfo;
    };

    //////////////////////////////////////////////////////////////////////
    //
    // CBodyInfo
    //
    //////////////////////////////////////////////////////////////////////
    class CBodyInfo {

    public:
            //////////////////////////////////////////////////////////////////////
            // Constructor/Destructor
            //////////////////////////////////////////////////////////////////////
            CBodyInfo();
            CBodyInfo( const CBodyInfo & other );
            ~CBodyInfo();
            void operator=( const CBodyInfo & other );
            friend ostream & operator<<( ostream & out, const CBodyInfo & body );

            bool m_bIsFixed;
            int  m_Index;
            string m_strModelDataFileName;
            double m_X, m_Y, m_Z;
            double m_Alpha, m_Beta, m_Gamma;

            int m_cNumberOfConnection;
            CConnectionInfo * m_pConnectionInfo;
    };

    //////////////////////////////////////////////////////////////////////
    //
    // ConnectionInfo
    //
    //////////////////////////////////////////////////////////////////////
    class CConnectionInfo {

    public:
            //////////////////////////////////////////////////////////////////////
            // Constructor/Destructor
            //////////////////////////////////////////////////////////////////////
            CConnectionInfo();
            CConnectionInfo( const CConnectionInfo & other );
            void operator=( const CConnectionInfo & other );
            friend ostream & operator<<( ostream & out, const CConnectionInfo & con );

            int m_preIndex;
            int m_nextIndex;

            //fake connection
    };

}//namespace plum

#endif //_MultiBodyInfo_H_

