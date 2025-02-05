#include "LoadTexture.h"

using namespace std;

#include <QImageReader>
#include <qimage.h>
#include <QOpenGLWidget>

//#ifdef __APPLE__
//  #include <OpenGL/glu.h>
//#else
//  #include <glu.h>
//#endif

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

//  QImage texture = QOpenGLWidget::convertToGLFormat(buf);
  QImage texture = buf.convertToFormat(QImage::Format_RGBA8888);

  //Bring into GLContext
  glGenTextures(1, &tid);
  glBindTexture(GL_TEXTURE_2D, tid);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
//  gluBuild2DMipmaps(GL_TEXTURE_2D, 3, texture.width(), texture.height(),
 //     GL_RGBA, GL_UNSIGNED_BYTE, texture.bits());
 //
glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture.width(), texture.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, buf.bits());


    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
f->glGenerateMipmap(GL_TEXTURE_2D);
  return tid;
}
