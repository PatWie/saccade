#include <iostream>
#include "gl_manager.h"
#include "mipmap_tile.h"
#include "gl_object.h"

typedef unsigned int uint;


Utils::MipmapTile::MipmapTile(float* ptr,
                              uint height, uint width, uint channels) {
  _obj = new GlObject<float>();
  _obj->data = ptr;
  _obj->height = height;
  _obj->width = width;
  _obj->channels = channels;
  _obj->loaded = false;
  // _obj->interpolation = GL_LINEAR;
}

Utils::MipmapTile::~MipmapTile() {}

void Utils::MipmapTile::clear() {
  glDeleteTextures(1, &(_obj->texture_id) );
  delete _obj;
}

const Utils::GlObject<float> *Utils::MipmapTile::obj() const {
  return _obj;
}

void Utils::MipmapTile::draw(Utils::GlManager *gl,
                             double posH, double posW) {
  if (!_obj->loaded)
    gl->prepare<float>(_obj);
  gl->draw<float>(_obj, posH, posW,
                  posH + _obj->height, posW + _obj->width, 1);
}
