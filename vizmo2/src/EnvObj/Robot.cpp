#include "Robot.h"

OBPRMView_Robot::OBPRMView_Robot(CEnvLoader * pEnv)
{
    m_pEnvLoader=pEnv;
    m_RobotModel=NULL;
    
    size = 1.0;
    change = false;
    come='r';
}

OBPRMView_Robot::OBPRMView_Robot(OBPRMView_Robot &other_robot)
{
    
    m_pEnvLoader=other_robot.getEnvLoader();
    m_RobotModel = other_robot.getRobotModel();
    
    size = other_robot.size;
    change = other_robot.change;
    come= other_robot.come;
}

CEnvLoader * OBPRMView_Robot::getEnvLoader() {
    return m_pEnvLoader;
}

CMultiBodyModel * OBPRMView_Robot::getRobotModel() {
    return m_RobotModel;
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
        //cout<<"DATA IFO: "<<pMInfo->m_pBodyInfo[iM]<<endl;
        if( !pMInfo->m_pBodyInfo[iM].m_bIsFixed ){
            break;
        }
    }
    
    //create MB for robot.
    m_RobotModel=new CMultiBodyModel(iM,pMInfo[iM]);
    if( m_RobotModel==NULL ) return false;
    m_RobotModel->setAsFree(); //set as free body
    SetColor(1,0,0,1);
    return m_RobotModel->BuildModels();
}

void OBPRMView_Robot::Draw(GLenum mode){
    if( GL_RENDER!=mode ) return;
    static double rate = 360/(3.1415926*2);
    glPushMatrix();
    glTransform();
    glScalef(size, size, size);    
    m_RobotModel->Draw(GL_RENDER);
    glPopMatrix();
}