#ifndef IO_H_
#define IO_H_

#include <iostream>
#include <string>
#include <cstdlib>
#include <algorithm>
using namespace std;

#include "Utilities/IOUtils.h"

#include "VizmoExceptions.h"
#include "Color.h"

//optionally read a color from a comment line
Color4 GetColorFromComment(istream& _is);

#endif
