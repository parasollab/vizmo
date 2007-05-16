#ifndef _MultiBodyInfo_H_
#define _MultiBodyInfo_H_

#include <iostream>
#include <string>
using namespace std;

#include "Transformation.h"

//class Transformation;

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
      
      int m_cNumberOfBody; //Total number of bodies
      int m_NumberOfConnections; //how many connections there are
      bool m_active; //Active or passive?

      CBodyInfo * m_pBodyInfo;
      vector< pair<int, int> > listConnections;     
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

      /////////////////////////////////////////////////////////////////////
      // Methods to perform transformations:current and previous are used
      // to compute the transformation of THIS body
      //////////////////////////////////////////////////////////////////////////

      ///@If m_Index==0, then transform m_X, m_Y, m_Z,m_Alpha, m_Beta, m_Gamma 
      void doTransform();
      ///@Copy t to m_prevTransform
      void setPrevTransform(Transformation t);
      ///@Return actual (previous*current) transformation 
      Transformation getTransform();
      ///@Compute prevtranform * TDH * dh * TBody2
      ///@receives the Body THIS body is connected to, the id of that connection and the new thetha
      void computeTransform(CBodyInfo &BodyInfo, int nextBody, double _theta);

      void operator=( const CBodyInfo & other );
      friend ostream & operator<<( ostream & out, const CBodyInfo & body );
      
      bool m_bIsFixed;
      bool m_IsBase;
      bool m_transformDone;
      int  m_Index;
      string m_strModelDataFileName;
      string m_strFileName;
      string m_strDirectory;
      bool m_isNew; // to detect whether this body has been added by the user
      double m_X, m_Y, m_Z;
      double m_Alpha, m_Beta, m_Gamma;
      float rgb[3]; //store initial color (read from env. file)
      
      int m_cNumberOfConnection;
      CConnectionInfo * m_pConnectionInfo;
      Transformation m_currentTransform, m_prevTransform;

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
      ///@Return DH params. as a Vecto4d
      Vector4d getDH();
      ///@Compute ad return the transformation from body1 to body2
      Transformation  transformToBody2();
      ///@Compute and return the transformation to DH frame
      Transformation  transformToDHframe();
      ///@Return id of the body THIS body is connected to
      int getNextBody(){return m_nextIndex;}
      void operator=( const CConnectionInfo & other );
      friend ostream & operator<<( ostream & out, const CConnectionInfo & con );
      
      int m_preIndex;
      int m_nextIndex;
      double m_posX, m_posY, m_posZ;
      double m_orientX, m_orientY, m_orientZ;
      double alpha, theta, a, d;
      // this will keep the original theta value read
      //for Revolute joints, theta is variable
      double m_theta; 
      string m_articulation;      
      bool m_actuated;
      double m_pos2X, m_pos2Y, m_pos2Z;
      double m_orient2X, m_orient2Y, m_orient2Z;
      
    };
    
}//namespace plum

#endif //_MultiBodyInfo_H_

