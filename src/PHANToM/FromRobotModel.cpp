
//in constructor
phantomdelta = .1;
pthread_mutex_init(&mutex, NULL);

//in configure
pthread_mutex_lock(&mutex);
pthread_mutex_unlock(&mutex);

//key press implementation
//need to add a call to this in GLWidget
bool
RobotModel::KP( QKeyEvent * e ) {
#ifdef USE_PHANTOM
  Matrix3x3 mFinal; Vector3d vFinal;
  Quaternion qt0,qrm,qrmconj,finalQ,tmpq;
  hduVector3Dd pposition;
  hduVector3Dd protation;
  hduVector3Dd position;
  hduVector3Dd rotation;
  Vector3d axis;
  Vector3d tmpaxis;
  double TwoPI = 2*3.14159;
  double angle;
  double R;
  double tx,ty,tz;
  double theta,phi,xi;
  vector<double> tmpCfg;

  switch(e->key()){
    case Qt::Key_PageUp :

      pthread_mutex_lock(&mutex);

      pposition=GetPhantomManager().getEndEffectorPosition();
      protation=GetPhantomManager().getRotation();
      rotation=GetPhantomManager().getRotation();
      double prx,pry,prz;
      double drx,dry,drz;
      prx = m_robotModel->rx();
      pry = m_robotModel->ry();
      prz = m_robotModel->rz();

      axis[0] = 0;
      axis[1] = 0;
      axis[2] = 1;


      tx = axis[0];
      ty = axis[1];
      tz = axis[2];


      GetCamera()->Transform(tx,ty,tz);


      axis[0] = tx;
      axis[1] = ty;
      axis[2] = tz;


      tmpaxis[0] = axis[0];
      tmpaxis[1] = axis[1];
      tmpaxis[2] = axis[2];
      phi = rotation[1];

      axis[1] = tmpaxis[1]*cos(phi) + tmpaxis[2]*sin(phi);
      axis[2] =-tmpaxis[1]*sin(phi) + tmpaxis[2]*cos(phi);


      tmpaxis[0] = axis[0];
      tmpaxis[1] = axis[1];
      tmpaxis[2] = axis[2];
      theta = rotation[0];

      axis[0] = tmpaxis[0]*cos(theta) + tmpaxis[2]*sin(theta);
      axis[2] =-tmpaxis[0]*sin(theta) + tmpaxis[2]*cos(theta);


      tmpaxis[0] = axis[0];
      tmpaxis[1] = axis[1];
      tmpaxis[2] = axis[2];
      xi = prz;

      axis[0] = tmpaxis[0]*cos(xi) + tmpaxis[1]*sin(xi);
      axis[1] =-tmpaxis[0]*sin(xi) + tmpaxis[1]*cos(xi);


      tmpaxis[0] = axis[0];
      tmpaxis[1] = axis[1];
      tmpaxis[2] = axis[2];
      theta = -pry;

      axis[0] = tmpaxis[0]*cos(theta) + tmpaxis[2]*sin(theta);
      axis[2] =-tmpaxis[0]*sin(theta) + tmpaxis[2]*cos(theta);


      tmpaxis[0] = axis[0];
      tmpaxis[1] = axis[1];
      tmpaxis[2] = axis[2];
      phi = prx;

      axis[1] = tmpaxis[1]*cos(phi) + tmpaxis[2]*sin(phi);
      axis[2] =-tmpaxis[1]*sin(phi) + tmpaxis[2]*cos(phi);

      angle = -100*(rotation[2] - protation[2]);

      tempCfg = StCfg;

      //Need to compute rotation from Quaternion

      //get rotation quaternion
      qt0 = Quaternion(cos(angle),sin(angle)*axis);

      //get new rotation from multiBody
      qrm = m_robotModel->q();

      //multiply polyhedron0 and multiBody quaternions //to get new rotation
      finalQ = qrm * qt0;

      //set new rotation angles to multiBody rx(), ry(), and rz()
      mFinal = finalQ.getMatrix();
      vFinal = finalQ.MatrixToEuler(mFinal);

      m_robotModel->rx() = vFinal[0];
      m_robotModel->ry() = vFinal[1];
      m_robotModel->rz() = vFinal[2];


      //set new angles for first polyhedron //NOTE:: This works for
      //**FREE** robots

      tempCfg[3] =  vFinal[0]/TwoPI;
      tempCfg[4] =  vFinal[1]/TwoPI;
      tempCfg[5] =  vFinal[2]/TwoPI;

      m_robotModel->setCurrCfg(tempCfg, dof);

      m_robotModel->Euler2Quaternion();
      GetVizmo().TurnOn_CD();
      if(GetPhantomManager().Collision > .5 && !GetPhantomManager().UseFeedback){
        m_robotModel->setCurrCfg(m_currCfg, dof);
        return true;
      }
      m_robotModel->Euler2Quaternion();
      StCfg = tempCfg;

      pthread_mutex_unlock(&mutex);

      return true;
    case Qt::Key_PageDown :

      pthread_mutex_lock(&mutex);

      tempCfg = StCfg;

      pposition=GetPhantomManager().getEndEffectorPosition();
      protation=GetPhantomManager().getRotation();
      position=GetPhantomManager().getEndEffectorPosition();
      double dx,dy,dz;
      dx=phantomdelta*(position[0]-pposition[0]);
      dy=phantomdelta*(position[1]-pposition[1]);
      dz=phantomdelta*(position[2]-pposition[2]);
      GetCamera()->Transform(dx,dy,dz);
      if(fabs(dx) < 10 && fabs(dy) < 10 && fabs(dz) < 10){
        m_robotModel->tx() += dx;
        m_robotModel->ty() += dy;
        m_robotModel->tz() += dz;
        tempCfg[0] += dx;
        tempCfg[1] += dy;
        tempCfg[2] += dz;
      }
      GetVizmo().TurnOn_CD();
      if(GetPhantomManager().Collision > .5 && !GetPhantomManager().UseFeedback){
        tempCfg[0] -= dx;
        tempCfg[1] -= dy;
        tempCfg[2] -= dz;
        return true;
      }
      StCfg = tempCfg;

      pthread_mutex_unlock(&mutex);

      return true;
    case Qt::Key_Insert :
      phantomdelta *= 1.1;
      return true;
    case Qt::Key_Delete :
      phantomdelta /= 1.1;
      return true;
    case Qt::Key_Home:
      if(GetPhantomManager().phantomforce < 5)
        GetPhantomManager().phantomforce *= 1.1;
      return true;
    case Qt::Key_End:
      if(GetPhantomManager().phantomforce > .05)
        GetPhantomManager().phantomforce /= 1.1;
      return true;
    case '.':
      GetPhantomManager().UseFeedback = !GetPhantomManager().UseFeedback;
      return true;
  }
  return false;
#endif
}
