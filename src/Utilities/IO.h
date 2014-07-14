#ifndef IO_H_
#define IO_H_

#include <iostream>
#include <string>
#include <cstdlib>
#include <algorithm>
using namespace std;

#include "Utilities/IOUtils.h"

#include "Models/RegionModel.h"

#include "VizmoExceptions.h"
#include "Color.h"

//parse filename out of map header
string ParseMapHeader(const string& _filename);

//optionally read a color from a comment line
Color4 GetColorFromComment(istream& _is);

void VDAddRegion(RegionModel* _region);
void VDRemoveRegion(RegionModel* _region);
void AddInitialRegions();

#endif
