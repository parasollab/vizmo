#include "IO.h"

#include <fstream>

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

//optionally read a color from a comment line
Color4
GetColorFromComment(istream& _is) {
  string line;
  while(!_is.eof()) {
    char c;
    while(isspace(_is.peek()))
      _is.get(c);

    c = _is.peek();
    if(!IsCommentLine(c))
      return Color4(0.5, 0.5, 0.5, 1);
    else{
      getline(_is, line);
      //colors begin with VIZMO_COLOR
      if(line[7] == 'C'){
        size_t loc = line.find(" ");
        string sub = line.substr(loc+1);
        istringstream iss(sub);
        float r, g, b;
        if(!(iss >> r >> g >> b))
          throw ParseException(WHERE, "Cannot parse MultiBody color");
        return Color4(r, g, b, 1);
      }
    }
  }
  return Color4();
}

