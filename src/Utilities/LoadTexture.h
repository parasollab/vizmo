#ifndef LOAD_TEXTURE_H_
#define LOAD_TEXTURE_H_

#include <string>

#include <qgl.h>

////////////////////////////////////////////////////////////////////////////////
/// @brief Load texture from file into open GL context
/// @param _filename Texture file
/// @return Texture ID in GL context
GLuint LoadTexture(const std::string& _filename);

#endif
