#include <iostream>
#include <string>

#include <glog/logging.h>

#include "../Utils/image_data.h"
#include "../Utils/mipmap.h"
#include "../Utils/gl_manager.h"
#include "../Utils/Ops/img_op.h"
#include "../Utils/Ops/gamma_op.h"
#include "layer.h"

// threads
GUI::threads::MipmapThread::MipmapThread() {}
void GUI::threads::MipmapThread::notify( Mipmap_ptr mipmap,  ImageData_ptr img) {
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
void GUI::threads::OperationThread::notify(ImageData_ptr dst,
     ImageData_ptr src,
    Utils::Ops::ImgOp *op) {
  _src = src;
  _dst = dst;
  _op = op;

}
void GUI::threads::OperationThread::run() {
  LOG(INFO) << "GUI::threads::OperationThread::run()";
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
           this, SLOT( slotMipmapFinished() ));

  // _thread_opWorker = new threads::OperationThread();
  // connect( _thread_opWorker, SIGNAL( finished() ),
  //          this, SLOT( slotApplyOpFinished() ));
  LOG(INFO) << "GUI::Layer::Layer()";

  _watcher = new QFileSystemWatcher();
  connect(_watcher, SIGNAL(fileChanged(QString)),
          this, SLOT(slotPathChanged(QString)));

  _current_mipmap = std::make_shared<Utils::Mipmap>();

}

void GUI::Layer::draw(Utils::GlManager *gl,
                      uint top, uint left,
                      uint bottom, uint right,
                      double zoom) {

  _current_mipmap->draw(gl, top, left, bottom, right, zoom);

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
  LOG(INFO) << "GUI::Layer::clear()";

  _available = false;
  _current_mipmap->clear();
  LOG(INFO) << "_mipmap->clear()";
  _imgdata->clear();
  LOG(INFO) << "_imgdata->clear()";
  _bufdata->clear(false);
  LOG(INFO) << "_bufdata->clear()";

}
void GUI::Layer::loadImage(std::string fn) {
  _available = false;
  if (_path != "") {
    _watcher->removePath(QString::fromStdString(_path));
  }
  _path = fn;

  // we keep the original data here
  _imgdata = std::make_shared<Utils::ImageData>(fn);

  // and for diplaying purposes we use the buffer data
  _bufdata = std::make_shared<Utils::ImageData>(_imgdata.get());

  slotRebuildMipmap();
}

void GUI::Layer::slotRebuildMipmap()  {
  _available = false;

  _working_mipmap = std::make_shared<Utils::Mipmap>();
  _thread_mipmapBuilder->notify(_working_mipmap, _bufdata);
  _thread_mipmapBuilder->start();

}
void GUI::Layer::slotMipmapFinished()  {
  LOG(INFO) << "GUI::Layer::slotMipmapFinished()";
  _current_mipmap = _working_mipmap;
   _watcher->addPath(QString::fromStdString(_path));
  _available = true;
  emit sigRefresh();
}

// void GUI::Layer::slotApplyOpFinished()  {
//   LOG(INFO) << "GUI::Layer::slotApplyOpFinished()";
//   emit sigApplyOpFinished();
// }


std::string GUI::Layer::path() const {
  return _path;
}

// void GUI::Layer::slotApplyOp(Utils::Ops::ImgOp* op) {
//   _available = false;
//   _thread_opWorker->notify(_bufdata, _imgdata, op);
//   _thread_opWorker->start();
// }


void GUI::Layer::slotPathChanged(QString s){
  LOG(INFO) << "GUI::Layer::slotPathChanged() " << s.toStdString();
  loadImage(s.toStdString());
}