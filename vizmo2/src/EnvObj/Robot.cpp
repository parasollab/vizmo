#include "Robot.h"

OBPRMView_Robot::OBPRMView_Robot(CEnvLoader * pEnv)
{
    m_pEnvLoader=pEnv;
    m_RobotModel=NULL;
}

OBPRMView_Robot::~OBPRMView_Robot()
{
    delete m_RobotModel;
}

bool OBPRMView_Robot::BuildModels(){
    
    //find robot name
    const CMultiBodyInfo * pMInfo = m_pEnvLoader->GetMultiBodyInfo();
    
    int iM;
    for( iM=0; iM<pMInfo->m_cNumberOfBody; iM++ ){
        if( !pMInfo->m_pBodyInfo[iM].m_bIsFixed ){
            break;
        }
    }
    
    //create MB for robot.
    m_RobotModel=new CMultiBodyModel(iM,pMInfo[iM]);
    if( m_RobotModel==NULL ) return false;
    m_RobotModel->setAsFree(); //set as free body
    SetColor(0.9f,0.7f,0.3f,1);
    return m_RobotModel->BuildModels();
}

void OBPRMView_Robot::Draw(GLenum mode){
    
    if( GL_RENDER!=mode ) return;
    
    //Move Robot accroding to Cfg
    static double rate = 360/(3.1415926*2);
    glPushMatrix();
    glTransform();
    m_RobotModel->Draw(GL_RENDER);
    glPopMatrix();
}

