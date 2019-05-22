#ifndef IO_H_
#define IO_H_

#include <iostream>
#include <string>
#include <cstdlib>
#include <algorithm>
using namespace std;

#include "Utilities/Color.h"
#include "Utilities/IOUtils.h"

#include "Models/RegionModel.h"

#include "VizmoExceptions.h"

//parse filename out of map header
string ParseMapHeader(const string& _filename);

void VDAddRegion(const RegionModel* _region);
void VDRemoveRegion(const RegionModel* _region);
void AddInitialRegions();

#endif
