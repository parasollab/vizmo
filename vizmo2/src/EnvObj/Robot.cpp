#include "Robot.h"

OBPRMView_Robot::OBPRMView_Robot(CEnvLoader * pEnv)
{
    m_pEnvLoader=pEnv;
    m_RobotModel=NULL;
}

OBPRMView_Robot::OBPRMView_Robot(const OBPRMView_Robot &other_robot)
:CGLModel(other_robot)
{
    m_pEnvLoader=other_robot.getEnvLoader();
    m_RobotModel = other_robot.getRobotModel();
}

CEnvLoader * OBPRMView_Robot::getEnvLoader() const {
    return m_pEnvLoader;
}

CMultiBodyModel * OBPRMView_Robot::getRobotModel() const {
    return m_RobotModel;
}

OBPRMView_Robot::~OBPRMView_Robot()
{
    delete m_RobotModel;
}

bool OBPRMView_Robot::BuildModels(){
    
    //find robot name
    const CMultiBodyInfo * pMInfo = m_pEnvLoader->GetMultiBodyInfo();
    int numMBody=m_pEnvLoader->GetNumberOfMultiBody();

    int iM=0;
    for( ; iM<numMBody; iM++ ){
        //cout<<"DATA IFO: "<<pMInfo->m_pBodyInfo[iM]<<endl;
        if( !pMInfo[iM].m_pBodyInfo[0].m_bIsFixed ){
            break;
        }
    }

	//no robot info found
	if( iM==numMBody ) 
		return true; //no need to build
    
    //create MB for robot.
    m_RobotModel=new CMultiBodyModel(iM,pMInfo[iM]);
    if( m_RobotModel==NULL ) return false;
    m_RobotModel->setAsFree(); //set as free body
    SetColor(1,0,0,1);
    return m_RobotModel->BuildModels();
}

void OBPRMView_Robot::Draw(GLenum mode){
	if( m_RobotModel==NULL || GL_RENDER!=mode ) return;
    //static double rate = 360/(3.1415926*2);
    glPushMatrix();
    glTransform();
    m_RobotModel->Draw(GL_RENDER);
    glPopMatrix();
}

void OBPRMView_Robot::DrawSelect()
{
	if( m_RobotModel==NULL ) return;
    glPushMatrix();
    glTransform();
    m_RobotModel->DrawSelect();
    glPopMatrix();
}