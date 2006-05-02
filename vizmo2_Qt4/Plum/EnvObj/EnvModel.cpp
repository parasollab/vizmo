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
    

  void CEnvModel::ChangeColor(){
        int MBSize=m_pMBModel.size();
	float R, G, B;
        
	for( int iP=0; iP<MBSize; iP++ ){
	  R  = ((float)rand())/RAND_MAX;
	  G = ((float)rand())/RAND_MAX;
	  B = ((float)rand())/RAND_MAX;
	  m_pMBModel[iP]->SetColor( R, G, B, 1);
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

  vector<CPolyhedronModel *> CEnvModel::getPoly(){

    int MBSize=m_pMBModel.size();
    vector<CPolyhedronModel *> pPoly; 

    for(int iP=0; iP<MBSize; iP++ ){
      pPoly.push_back(m_pMBModel[iP]->GetPolyhedron());
      
    }

    return pPoly;
  }

  vector<CMultiBodyModel *> CEnvModel::getMBody(){

    return m_pMBModel;
  }
    

  void CEnvModel::DeleteMBModel(CMultiBodyModel *mbl){
    vector<CMultiBodyModel *>:: iterator Imbm; 
    
    for(Imbm =  m_pMBModel.begin(); Imbm != m_pMBModel.end(); Imbm++){
      if( (*Imbm) == mbl){
	m_pMBModel.erase(Imbm);
	break;
      }    
    }      
  }

  void CEnvModel::AddMBModel(CMultiBodyInfo newMBI){
   int MBSize = m_envLoader->GetNumberOfMultiBody();
   int i = MBSize-1;

   CMultiBodyModel * mbm=new CMultiBodyModel(i, m_envLoader->GetMultiBodyInfo()[i]); 
   mbm->BuildModels();
   Vector3d com;
   com=com+(mbm->GetCOM()-Point3d(0,0,0));
   m_pMBModel.push_back(mbm);
  }

  bool CEnvModel::SaveFile(const char *filename){
  
    const CMultiBodyInfo* MBI = m_envLoader->GetMultiBodyInfo();

    //alpha, beta, and gamma are in DEGREES
    double PI_180=3.1415926535/180;

    FILE *envFile;
    if((envFile = fopen(filename, "a")) == NULL){
      cout<<"Couldn't open the file"<<endl;
      return 0;
    }

    int MBnum = m_envLoader->GetNumberOfMultiBody();//number of objects in env.
    //write num. of Bodies
    fprintf(envFile,"%d\n\n", MBnum );

  //getMBody() and then current position and orientation
    vector<CMultiBodyModel *> MBmodel = this->getMBody();
    int numMB = MBmodel.size();

    for(int i = 0; i<MBnum; i++){ //for each body in *.env

      if(MBI[i].m_active)
	fprintf(envFile,"Multibody   Active\n");
      else
	fprintf(envFile,"Multibody   Passive\n");

      if(MBI[i].m_cNumberOfBody != 0){
	int nB = MBI[i].m_cNumberOfBody;
	//write Num. of Bodies in the current MultiBody
	fprintf(envFile,"%d\n", nB);
	//write COLOR tag
	list<CGLModel*> tmpList;
	MBmodel[i]->GetChildren(tmpList);
	CGLModel* om = tmpList.front();
	if(MBI[i].m_active){
	fprintf(envFile,"#VIZMO_COLOR %2f %2f %2f\n",
		MBI[i].m_pBodyInfo[0].rgb[0],
		MBI[i].m_pBodyInfo[0].rgb[1], 
		MBI[i].m_pBodyInfo[0].rgb[2]);
	}
	else{	  
	fprintf(envFile,"#VIZMO_COLOR %2f %2f %2f\n", 
		om->GetColor()[0],om->GetColor()[1],om->GetColor()[2]);	
	}  
     
	for(int j = 0; j<nB; j++){
	  if(MBI[i].m_pBodyInfo[j].m_bIsFixed)
	    fprintf(envFile,"FixedBody    "); 
	  else
	    fprintf(envFile,"FreeBody    ");
	
	  fprintf(envFile,"%d  ",MBI[i].m_pBodyInfo[j].m_Index);
	  string s_tmp = MBI[i].m_pBodyInfo[j].m_strModelDataFileName;
	  const char* st;
	  st = s_tmp.c_str();
	  char *pos = strrchr(st, '/');
	  int position = pos-st+1;
	  string sub_string = s_tmp.substr(position);
	  
	  const char* f;
	  f = sub_string.c_str();
	  
	  if(!MBI[i].m_active){
	    string s = MBI[i].m_pBodyInfo[j].m_strFileName;
	    f = s.c_str();
	    fprintf(envFile,"%s  ",f);
	  }
	  else
	    fprintf(envFile,"%s  ",f);
	  
	  //get current (new) rotation
	  
	  Quaternion qtmp = MBmodel[i]->q();
	  //Matrix3x3 mtmp = qtmp.getMatrix();
	  //Vector3d vtmp = qtmp.MatrixToEuler(mtmp); 
	  
	  //get prev. rotation
	  
	  list<CGLModel*> objList;
	  MBmodel[i]->GetChildren(objList);
	  CGLModel* om = objList.front();
	  //get current quaternion from polyhedron0
	  Quaternion qt0;
	  qt0 = om->q();
	  Matrix3x3 pm = qt0.getMatrix();
	  Vector3d pv = qt0.MatrixToEuler(pm);
	  
	  //multiply polyhedron0 and multiBody quaternions
	  //to get new rotation
	  Quaternion finalQ;
	  finalQ = qtmp * qt0;
	  
	  Matrix3x3 fm = finalQ.getMatrix();
	  Vector3d fv = qt0.MatrixToEuler(fm);
	  
	  fprintf(envFile,"%.1f %.1f %.1f %.1f %.1f %.1f\n",
		  MBmodel[i]->tx(), MBmodel[i]->ty(), MBmodel[i]->tz(),
		  fv[0]/PI_180, 
		  fv[1]/PI_180, 
		  fv[2]/PI_180);
   
      }
      //write Connection tag

      if(MBI[i].m_NumberOfConnections !=0)
	fprintf(envFile,"\nConnection\n"); 
      else
	fprintf(envFile,"Connection\n"); 

      fprintf(envFile,"%d\n", MBI[i].m_NumberOfConnections); 

      //write Connection info.
      if(MBI[i].m_NumberOfConnections !=0){
	const char* str;
	int numConn = MBI[i].listConnections.size();
	
	for(int l=0; l<numConn; l++){
	  int indexList = MBI[i].listConnections[l].first;
	  if(MBI[i].m_pBodyInfo[indexList].m_pConnectionInfo->m_actuated)
	    str= "Actuated";
	  else
	    str = "NonActuated";

	  fprintf(envFile,"%d %d  %s\n",MBI[i].listConnections[l].first,
		  MBI[i].listConnections[l].second, str);

	  //get info. from current Body and current connection
	  int index;
	  for(int b=0; 
	      b<MBI[i].m_pBodyInfo[indexList].m_cNumberOfConnection; b++){
	    
	    int n = MBI[i].m_pBodyInfo[indexList].m_pConnectionInfo[b].m_nextIndex;

	    if( MBI[i].listConnections[l].second == n){
	      index = b;
	      break;
	    }
	  }

	  string s_tmp = MBI[i].m_pBodyInfo[indexList].m_pConnectionInfo[index].m_articulation;
	  const char* f = s_tmp.c_str();
	  
	  fprintf(envFile, "%.1f ",
		  MBI[i].m_pBodyInfo[indexList].m_pConnectionInfo[index].m_posX);
	  fprintf(envFile, "%.1f ",
		  MBI[i].m_pBodyInfo[indexList].m_pConnectionInfo[index].m_posY);
	  fprintf(envFile, "%.1f ",
		  MBI[i].m_pBodyInfo[indexList].m_pConnectionInfo[index].m_posZ);
	  fprintf(envFile, "%.1f ",
		  MBI[i].m_pBodyInfo[indexList].m_pConnectionInfo[index].m_orientX*57.29578);
	  fprintf(envFile, "%.1f ",
		  MBI[i].m_pBodyInfo[indexList].m_pConnectionInfo[index].m_orientY* 57.29578);
	  fprintf(envFile, "%.1f\t",
		  MBI[i].m_pBodyInfo[indexList].m_pConnectionInfo[index].m_orientZ* 57.29578);

	  fprintf(envFile, "%.1f ", 
		  MBI[i].m_pBodyInfo[indexList].m_pConnectionInfo[index].alpha);
	  fprintf(envFile, "%.1f ",
		  MBI[i].m_pBodyInfo[indexList].m_pConnectionInfo[index].a);
	  fprintf(envFile, "%.1f ",
		    MBI[i].m_pBodyInfo[indexList].m_pConnectionInfo[index].d);
	  fprintf(envFile, "%.1f        ",
		    MBI[i].m_pBodyInfo[indexList].m_pConnectionInfo[index].m_theta);
	  fprintf(envFile, "%s        ", f);
	  fprintf(envFile, "%.1f ",
		  MBI[i].m_pBodyInfo[indexList].m_pConnectionInfo[index].m_pos2X);
	  fprintf(envFile, "%.1f ",
		  MBI[i].m_pBodyInfo[indexList].m_pConnectionInfo[index].m_pos2Y);
	  fprintf(envFile, "%.1f ",
		  MBI[i].m_pBodyInfo[indexList].m_pConnectionInfo[index].m_pos2Z);
	  fprintf(envFile, "%.1f ",
		  MBI[i].m_pBodyInfo[indexList].m_pConnectionInfo[index].m_orient2X* 57.29578);
	  fprintf(envFile, "%.1f ",
		  MBI[i].m_pBodyInfo[indexList].m_pConnectionInfo[index].m_orient2Y* 57.29578);
	  fprintf(envFile, "%.1f\n\n",
		  MBI[i].m_pBodyInfo[indexList].m_pConnectionInfo[index].m_orient2Z* 57.29578);
	}
	
      }
      
    }
    fprintf(envFile,"\n");
  }
  
  fclose(envFile);

}

}//namespace plum
