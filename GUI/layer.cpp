#include "../Utils/image_data.h"
#include "../Utils/mipmap.h"
#include "../Utils/gl_manager.h"
#include "../Utils/Ops/img_op.h"
#include "layer.h"
#include <iostream>
#include <string>
#include "../Utils/Ops/gamma_op.h"

// threads
GUI::threads::MipmapThread::MipmapThread() {}
void GUI::threads::MipmapThread::notify( Utils::Mipmap* mipmap,  Utils::ImageData *img) {
  _mipmap = mipmap;
  _img = img;
}
void GUI::threads::MipmapThread::run() {
  if (!_mipmap->empty())
    _mipmap->clear();
  _mipmap->setData(_img->data(),
                   _img->height(), _img->width(), _img->channels());

}

GUI::threads::OperationThread::OperationThread() {}
void GUI::threads::OperationThread::notify(Utils::ImageData *dst,
    Utils::ImageData *src,
    Utils::Ops::ImgOp *op) {
  _src = src;
  _dst = dst;
  _op = op;

}
void GUI::threads::OperationThread::run() {
  std::cout << "GUI::threads::OperationThread::run(" << std::endl;
  const float *src = _src->data();
  float *dst = _dst->data();
  for (size_t i = 0; i < _src->elements(); ++i)
    dst[i] = _op->apply(src[i]);
}

// class

GUI::Layer::~Layer() {}
GUI::Layer::Layer() {
  _path = "";
  _available = false;

  _thread_mipmapBuilder = new threads::MipmapThread();
  connect( _thread_mipmapBuilder, SIGNAL( finished() ),
           this, SLOT( slotLoadFinished() ));

  _thread_opWorker = new threads::OperationThread();
  connect( _thread_opWorker, SIGNAL( finished() ),
           this, SLOT( slotApplyOpFinished() ));
  std::cout << "GUI::Layer::Layer()" << std::endl;
}

void GUI::Layer::draw(Utils::GlManager *gl,
                      uint top, uint left,
                      uint bottom, uint right,
                      double zoom) {

  _mipmap->draw(gl, top, left, bottom, right, zoom);

}

size_t GUI::Layer::width() const {
  return available() ? _imgdata->width() : 0 ;
}
size_t GUI::Layer::height() const {
  return available() ? _imgdata->height() : 0 ;
}

bool GUI::Layer::available() const {
  return _available;
}

void GUI::Layer::clear() {
  std::cout << "GUI::Layer::clear" << std::endl;
    
  _available = false;
  _mipmap->clear();
  std::cout << "_mipmap->clear();" << std::endl;
  _imgdata->clear();
  std::cout << "_imgdata->clear();" << std::endl;
  _bufdata->clear(false);
  std::cout << "_bufdata->clear();" << std::endl;

}
void GUI::Layer::loadImage(std::string fn) {
  _path = fn;
  _available = false;
  // we keep the original data here
  _imgdata = new Utils::ImageData(fn);

  // and for diplaying purposes we use the buffer data
  _bufdata = new Utils::ImageData(_imgdata);

  _mipmap = new Utils::Mipmap();

  slotRebuildMipmap();
}

void GUI::Layer::slotRebuildMipmap()  {
  _available = false;

  _thread_mipmapBuilder->notify(_mipmap, _bufdata);
  _thread_mipmapBuilder->start();

}
void GUI::Layer::slotLoadFinished()  {
  std::cout << "GUI::Layer::slotLoadFinished()" << std::endl;
  _available = true;
  emit sigRefresh();
}

void GUI::Layer::slotApplyOpFinished()  {
  std::cout << "GUI::Layer::slotApplyOpFinished()" << std::endl;
  // for zooming and tiling the image we use the mipmap data structure
  emit sigApplyOpFinished();
}


std::string GUI::Layer::path() const {
  return _path;
}

void GUI::Layer::slotApplyOp(Utils::Ops::ImgOp* op) {
  _available = false;
  _thread_opWorker->notify(_bufdata, _imgdata, op);
  _thread_opWorker->start();
}
