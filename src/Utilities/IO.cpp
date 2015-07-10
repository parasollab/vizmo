#include "IO.h"

#include <fstream>

#include <Models/EnvModel.h>
#include <Models/Vizmo.h>

//parse filename out of map header
string ParseMapHeader(const string& _filename) {
  string envDir = GetPathName(_filename);

  if(!FileExists(_filename))
    throw ParseException(WHERE, "File '" + _filename + "' does not exist.");

  ifstream ifs(_filename.c_str());

  //Open file for reading data
  string env, s;

  //Get env file name info
  GoToNext(ifs);
  getline(ifs, env);

  return envDir + env;
}

void VDAddRegion(RegionModel const* _region) {
  if(vdo != NULL) {
    (*vdo) << "AddRegion ";
    _region->OutputDebugInfo(*vdo);
  }
}

void VDRemoveRegion(RegionModel const* _region) {
  if(vdo != NULL) {
    (*vdo) << "RemoveRegion ";
    _region->OutputDebugInfo(*vdo);
  }
}

void AddInitialRegions() {
  if(vdo != NULL) {
    typedef vector<shared_ptr<RegionModel> > VecRM;
    typedef VecRM::const_iterator CRIT;
    const VecRM& attractRegions = GetVizmo().GetEnv()->GetAttractRegions();
    const VecRM& avoidRegions = GetVizmo().GetEnv()->GetAvoidRegions();
    const VecRM& nonCommitRegions = GetVizmo().GetEnv()->GetNonCommitRegions();

    for(CRIT i = attractRegions.begin(); i != attractRegions.end(); ++i)
      VDAddRegion(i->get());
    for(CRIT i = avoidRegions.begin(); i != avoidRegions.end(); ++i)
      VDAddRegion(i->get());
    for(CRIT i = nonCommitRegions.begin(); i != nonCommitRegions.end(); ++i)
      VDAddRegion(i->get());
  }
}

