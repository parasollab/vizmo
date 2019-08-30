#include "Font.h"

#ifdef __APPLE__
  #include <GLUT/glut.h>
#else
  #include <GL/glut.h>
#endif

GLvoid* fontStyle = GLUT_BITMAP_HELVETICA_12;

//Set font
void
SetFont(const string& _name, int _size) {
  fontStyle = GLUT_BITMAP_HELVETICA_10;
  if(_name == "helvetica") {
    if(_size == 12)
      fontStyle = GLUT_BITMAP_HELVETICA_12;
    else if(_size == 18)
      fontStyle = GLUT_BITMAP_HELVETICA_18;
  }
  else if(_name == "times roman") {
    if(_size == 10)
      fontStyle = GLUT_BITMAP_TIMES_ROMAN_10;
    else if(_size == 24)
      fontStyle = GLUT_BITMAP_TIMES_ROMAN_24;
  }
  else if(_name == "8x13") {
    fontStyle = GLUT_BITMAP_8_BY_13;
  }
  else if(_name == "9x15") {
    fontStyle = GLUT_BITMAP_9_BY_15;
  }
}

//Draw a string at given x, y, z
void
DrawStr(double _x, double _y, double _z, const string& _str) {
  glRasterPos3f(_x, _y, _z);
  for(string::const_iterator i = _str.begin(); i!=_str.end(); ++i)
    glutBitmapCharacter(fontStyle, *i);
}

