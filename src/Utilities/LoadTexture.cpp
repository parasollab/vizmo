#include "LoadTexture.h"

using namespace std;

#include <QImageReader>
#include <qimage.h>
#include <glu.h>

#include "VizmoExceptions.h"

GLuint
LoadTexture(const string& _filename) {
  GLuint tid = -1;

  // Load image from file
  QImageReader reader(_filename.c_str());
  QImage buf = reader.read();
  if(buf.isNull())
    throw RunTimeException(WHERE, "Cannot load texture '" + _filename +
        "'. QImageReader error '" + ::to_string(reader.error()) + "'.");

  QImage texture = QGLWidget::convertToGLFormat(buf);

  //Bring into GLContext
  glGenTextures(1, &tid);
  glBindTexture(GL_TEXTURE_2D, tid);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
  gluBuild2DMipmaps(GL_TEXTURE_2D, 3, texture.width(), texture.height(),
      GL_RGBA, GL_UNSIGNED_BYTE, texture.bits());

  return tid;
}
