#include "Texture.h"

#include <QImage>

#include <glu.h>

#include "Utilities/Exceptions.h"

GLuint CreateTexture(const string& _file) {
  GLuint tid=-1;
  QImage texture, buf;

  if(!buf.load(_file.c_str()))
    throw ParseException(WHERE, "Could not read image file '" + _file +"'.");

  texture = QGLWidget::convertToGLFormat(buf);  // flipped 32bit RGBA

  //generate the texture
  glGenTextures(1, &tid);
  glBindTexture(GL_TEXTURE_2D, tid);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
  gluBuild2DMipmaps(GL_TEXTURE_2D, 3,
      texture.width(), texture.height(),
      GL_RGBA, GL_UNSIGNED_BYTE, texture.bits());

  return tid;
}
