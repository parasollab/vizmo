//this code is from key press event in GLWidget

//cout << "key" << endl;
if(CDOn){
  RobotModel* rob = (RobotModel*)(GetVizmo().GetRobot()->getModel());
  //cout << "on " << endl;
  GetPhantomManager().CDOn = true;
  GetVizmo().TurnOn_CD();
  double x = rob -> gettx();
  double y = rob -> getty();
  double z = rob -> gettz();
  //cout << x << " " << y << " " << z << endl;
  GetPhantomManager().fpos.clear();
  GetPhantomManager().fpos.push_back(x);
  GetPhantomManager().fpos.push_back(y);
  GetPhantomManager().fpos.push_back(z);
  GetPhantomManager().proceed = true;
}
else
GetPhantomManager().CDOn = false;

