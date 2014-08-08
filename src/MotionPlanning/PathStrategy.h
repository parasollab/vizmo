#ifndef PATH_STRATEGY_H_
#define PATH_STRATEGY_H_

#include "MPStrategies/MPStrategyMethod.h"

#include "Models/CfgModel.h"
#include "Models/EnvModel.h"
#include "Models/TempObjsModel.h"
#include "Models/UserPathModel.h"
#include "Models/Vizmo.h"

#include "ValidityCheckers/CollisionDetectionValidity.h"

#include "Utilities/GLUtils.h"

template<class MPTraits>
class PathStrategy : public MPStrategyMethod<MPTraits> {
  public:
    typedef typename MPTraits::MPProblemType MPProblemType;
    typedef typename MPTraits::CfgType CfgType;
    typedef typename MPTraits::WeightType WeightType;
    typedef typename MPProblemType::RoadmapType::VID VID;
    typedef typename MPProblemType::LocalPlannerPointer LocalPlannerPointer;
    typedef typename MPProblemType::NeighborhoodFinderPointer NeighborhoodFinderPointer;
    typedef typename MPProblemType::ValidityCheckerPointer ValidityCheckerPointer;


    PathStrategy();
    PathStrategy(MPProblemType* _problem, XMLNodeReader& _node);

    void Initialize();
    void Run();
    void Finalize();

    virtual void Print(ostream& _os) const;

  protected:
    //helper functions
    void AddToRoadmap(vector<CfgType>& _samples, vector<VID>& _vids);
    void Connect(vector<VID>& _vids);
    void PathConnector();
    shared_ptr< vector<CfgModel> > RandomizeCfgs();
    shared_ptr< vector<CfgModel> > ValidateCfgs(vector<CfgType>& _cfgs);
    vector<double> ToVector(const CfgModel& _p);


  private:
    UserPathModel* m_samplingPath;
    string m_lpLabel;
    MedialAxisUtility<MPTraits> m_medialAxisUtility;
    EnvModel* envModel;
};

template<class MPTraits>
PathStrategy<MPTraits>::
PathStrategy() {
  this->SetName("PathStrategy");
  MedialAxisUtility<MPTraits>  m_medialAxisUtility(this->GetMPProblem(), "sl", "euclidean");
  envModel = GetVizmo().GetEnv();
}

template<class MPTraits>
PathStrategy<MPTraits>::
PathStrategy (MPProblemType* _problem, XMLNodeReader& _node) :
  MPStrategyMethod<MPTraits>(_problem, _node),m_medialAxisUtility(_problem, _node) {
    this->SetName("PathStrategy");
    envModel = GetVizmo().GetEnv();
    m_lpLabel= _node.stringXMLParameter("lp_label", false, "sl","Local Planner");

}


template<class MPTraits>
void
PathStrategy<MPTraits>::
Print(ostream& _os) const {
  _os << this->GetNameAndLabel() << endl;
}

template<class MPTraits>
void
PathStrategy<MPTraits>::
Initialize() {
  cout << "Initializing Path Strategy." << endl;

  string basename = this->GetBaseFilename();

  //Make non-user objects non-selectable while PathStrategy is running
  GetVizmo().GetMap()->SetSelectable(false);
  GetVizmo().GetEnv()->SetSelectable(false);
  GetVizmo().GetRobot()->SetSelectable(false);

  //start clocks
  GetVizmo().StartClock("PathStrategy");
  this->GetMPProblem()->GetStatClass()->StartClock("PathStrategyMP");
}

template<class MPTraits>
void
PathStrategy<MPTraits>::
Run() {
  cout << "Running Path Strategy." << endl;
  PathConnector();
}

template<class MPTraits>
void
PathStrategy<MPTraits>::
Finalize() {
  //stop clocks
  GetVizmo().StopClock("PathStrategy");
  this->GetMPProblem()->GetStatClass()->StopClock("PathStrategyMP");

  cout << "Finalizing Path Strategy." << endl;

  //redraw finished map
  GetVizmo().GetMap()->RefreshMap();

  //base filename
  string basename = this->GetBaseFilename();

  //print clocks + output a stat class
  StatClass* stats = this->GetMPProblem()->GetStatClass();
  GetVizmo().PrintClock("PathStrategy", cout);
  stats->PrintClock("PathStrategyMP", cout);

  ofstream ostats((basename + ".stats").c_str());
  ostats << "NodeGen+Connection Stats" << endl;
  stats->PrintAllStats(ostats, this->GetMPProblem()->GetRoadmap());
  GetVizmo().PrintClock("PathStrategy", ostats);
  stats->PrintClock("PathStrategyMP", ostats);

  //output roadmap
  ofstream ofs((basename + ".map").c_str());
  this->GetMPProblem()->GetRoadmap()->Write(ofs, this->GetMPProblem()->GetEnvironment());

  //Make things selectable again
  GetVizmo().GetMap()->SetSelectable(true);
  GetVizmo().GetEnv()->SetSelectable(true);
  GetVizmo().GetRobot()->SetSelectable(true);
}

template<class MPTraits>
void
PathStrategy<MPTraits>::
AddToRoadmap(vector<CfgType>& _samples, vector<VID>& _vids) {
  //lock map data
  QMutexLocker locker(&GetVizmo().GetMap()->AcquireMutex());

  //add nodes in _samples to graph. store VID's in _vids for connecting
  _vids.clear();
  for(typename vector<CfgType>::iterator cit = _samples.begin();
      cit != _samples.end(); cit++) {
    VID addedNode = this->GetMPProblem()->GetRoadmap()->GetGraph()->AddVertex(*cit);
    _vids.push_back(addedNode);
  }
}

template<class MPTraits>
void
PathStrategy<MPTraits>::
Connect(vector<VID>& _vids) {
  QMutexLocker locker(&GetVizmo().GetMap()->AcquireMutex());
  stapl::sequential::
    vector_property_map<typename MPProblemType::GraphType::GRAPH, size_t> cMap;
  typename MPProblemType::ConnectorPointer cp =
    this->GetMPProblem()->GetConnector("Neighborhood Connector");
  cp->Connect(this->GetMPProblem()->GetRoadmap(),
      *(this->GetMPProblem()->GetStatClass()), cMap, _vids.begin(), _vids.end());
}


template<class MPTraits>
void
PathStrategy< MPTraits >::
PathConnector(){
  LocalPlannerPointer lp = this->GetMPProblem()->GetLocalPlanner(m_lpLabel);
  Environment* env = this->GetMPProblem()->GetEnvironment();
  LPOutput<MPTraits> lpOutput;
  EnvModel* Vmodel = GetVizmo(). GetEnv();
  UserPathModel* Path = Vmodel->GetUserPaths()[0];
  shared_ptr<vector<CfgModel> > TempCfgs = Path->GetCfgs();
  for(vector<CfgModel>::iterator x = TempCfgs->begin(); x != TempCfgs->end()-1; x++) {
    if(lp->IsConnected(*x, *(x+1), &lpOutput,env->GetPositionRes(),env->GetOrientationRes())){
      vector<CfgModel> newNodes;
      if( x== TempCfgs->begin())
        newNodes.push_back(*x);
      else
        newNodes.push_back(*(x+1));
      vector<VID> newVIDs;
      AddToRoadmap(newNodes, newVIDs);
      Connect(newVIDs);
    }
    else {
      if(lp->IsConnected(*x, *(x+1), &lpOutput,env->GetPositionRes(),env->GetOrientationRes())){
        vector<CfgModel> newNodes;
        newNodes.push_back(*x);
        m_medialAxisUtility.PushToMedialAxis(*(x+1), env->GetBoundary());
        newNodes.push_back(*(x+1));
        vector<VID> newVIDs;
        AddToRoadmap(newNodes, newVIDs);
        Connect(newVIDs);

      }
    }
  }
}

/*
template<class MPTraits>
void
PathStrategy< MPTraits >::
PathConnector(){
    LocalPlannerPointer lp = this->GetMPProblem()->GetLocalPlanner(m_lpLabel);
    Environment* env = this->GetMPProblem()->GetEnvironment();
    LPOutput<MPTraits> lpOutput;
    EnvModel* envModel = GetVizmo().GetEnv();
    UserPathModel* path = envModel->GetUserPaths()[0];

    shared_ptr<vector<CfgModel> > tempCfgs = path->GetCfgs();
    //After Leslie finishes her code, update and use this instead of path->GetCfgs()
    //shared_ptr<vector<CfgModel> > tempCfgs = RandomizeCfgs();

    //Create a vector for adding new cfgs to roadmap
      vector<CfgModel> newCfgs;

      //cout<<(*tempCfgs)[0];
    //add the first cfg from user path
      newCfgs.push_back((*tempCfgs)[0]);

    //work through the chain of nodes checking connections
      for(vector<CfgModel>::iterator cit = tempCfgs->begin(); cit != tempCfgs->end() - 1; ++cit) {
          //check that node can connect. if not, push to MA
            if(!lp->IsConnected(*cit, *(cit + 1), &lpOutput, env->GetPositionRes(), env->GetOrientationRes()))
              m_medialAxisUtility.PushToMedialAxis(*(cit + 1), env->GetBoundary());
          newCfgs.push_back(*(cit + 1));
        }
    vector<VID> newVIDs;
    AddToRoadmap(newCfgs, newVIDs);
    Connect(newVIDs);
}
*/

//method takes a vector<Point3d> and uses it to generate valid configurations
template<class MPTraits>
shared_ptr< vector<CfgModel> >
PathStrategy<MPTraits>::
RandomizeCfgs() {
  UserPathModel* userPath = envModel->GetUserPaths()[0];
  shared_ptr<vector<CfgType> >newCfgs(new vector<CfgType>());

  //uses function GetCfgs to get configurations from Point3d's
  shared_ptr< vector<CfgModel> > cfgs = userPath->GetCfgs();

  CfgType newCfg;
  if(GetVizmo().GetRobot()->IsPlanar()){
    //go through all cfgs in the userPath
    for(vector<CfgModel>::iterator it = cfgs->begin();
        it != cfgs->end(); ++it) {
      vector<double> v = (*it).GetDataCfg();
      //gets data then skips over first two, randomizes the rest
      for(int i = 2; i < v.size(); ++i)
        v[i] = (2.0*DRand()-1.0);
      newCfg.SetData(v);
      newCfgs.push_back(newCfg);
    }
  } else {
    //robot is volumetric
    for(vector<CfgModel>::iterator it = cfgs->begin();
        it != cfgs->end(); ++it) {
      vector<double> v = (*it).GetDataCfg();
      //gets data then skips over first three, randomizes the rest
      for(int i = 3; i < v.size(); ++i)
        v[i] = (2.0*DRand()-1.0);
      newCfg.SetData(v);
      newCfgs.push_back(newCfg);
      }
    }
  return ValidateCfgs(newCfgs);
}

//method validates configurations, if invalid, will push to medial axis
template<class MPTraits>
shared_ptr< vector<CfgModel> >
PathStrategy<MPTraits>::
ValidateCfgs(vector<CfgType>& _cfgs) {

  vector<CfgModel> validCfgs;
  shared_ptr< vector<CfgModel> > c;
  typedef typename vector<CfgType>::iterator CIT;
  for(CIT cit = _cfgs.begin(); cit != _cfgs.end(); ++cit) {
    GetVizmo().CollisionCheck((*cit));
    if((*cit).IsValid()) {
      validCfgs.push_back(*cit);
    }
    else {
      //Push to medial axis
    }
  }
  size_t count = 0;
  for(vector<CfgModel>::iterator it = validCfgs.begin();
      it != validCfgs.end(); ++it) {
    //const vector<double>& v = (*it).GetDataCfg();
    //(*c)[count].SetCfg(v);
    (*c)[count] = (*it);
    count++;
  }
  return c;
}
/*
template<class MPTraits>
vector<double>
PathStrategy::
ToVector(const CfgModel& _p) {
  vector<double> data(m_newPos.DOF(), 0.);
  copy(_p->begin(), _p->end(), data.begin());
  return data;
}
*/
#endif
