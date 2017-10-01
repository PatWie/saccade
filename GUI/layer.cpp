#include <iostream>
#include <string>

#include <glog/logging.h>
// #include <QTest>

#include "../Utils/image_data.h"
#include "../Utils/histogram_data.h"
#include "../Utils/mipmap.h"
#include "../Utils/gl_manager.h"
#include "../Utils/Ops/img_op.h"
#include "../Utils/Ops/gamma_op.h"
#include "../Utils/Ops/histogram_op.h"
#include "layer.h"

// threads
// ==========================================================================================
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

// ------------------------------------------------------------------------------------------
GUI::threads::HistogramThread::HistogramThread() {}
void GUI::threads::HistogramThread::notify(ImageData_ptr img,  HistogramData_ptr hist) {
  _img = img;
  _hist = hist;
}

void GUI::threads::HistogramThread::run() {
  _hist->setData(_img.get(), _img->max());
}

// ------------------------------------------------------------------------------------------
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
  #pragma omp parallel for
  for (size_t i = 0; i < _src->elements(); ++i)
    dst[i] = _op->apply(src[i]);
  LOG(INFO) << "GUI::threads::OperationThread::run() done";
}

// ------------------------------------------------------------------------------------------
GUI::threads::ReloadThread::ReloadThread() {}
void GUI::threads::ReloadThread::notify(std::string fn, int attempts) {
  _fn = fn;
  _attempts = attempts;
}

void GUI::threads::ReloadThread::run() {
  LOG(INFO) << "GUI::threads::ReloadThread::run()";
  for (int i = 0; i < _attempts; ++i)
  {
    if(Utils::ImageData::validFile(_fn)){
      emit sigFileIsValid(QString::fromStdString(_fn));
      break;
    }
    QThread::msleep(500);
  }
}

// class
// ==========================================================================================

GUI::Layer::~Layer() {}

GUI::Layer::Layer() {
  LOG(INFO) << "GUI::Layer::Layer()";
  _path = "";
  _available = false;

  // connection to all threads
  _thread_mipmapBuilder = new threads::MipmapThread();
  connect( _thread_mipmapBuilder, SIGNAL( finished() ),
           this, SLOT( slotMipmapFinished() ));

  _thread_Reloader = new threads::ReloadThread();
  connect( _thread_Reloader, SIGNAL( sigFileIsValid(QString) ),
           this, SLOT( slotFileIsValid(QString) ));

  _thread_histogram = new threads::HistogramThread();
  connect( _thread_histogram, SIGNAL( finished() ),
           this, SLOT( slotHistogramFinished() ));

  _thread_opWorker = new threads::OperationThread();
  connect( _thread_opWorker, SIGNAL( finished() ),
           this, SLOT( slotApplyOpFinished() ));

  _watcher = new QFileSystemWatcher();
  connect(_watcher, SIGNAL(fileChanged(QString)),
          this, SLOT(slotPathChanged(QString)));

  // this needs to be available all time
  _current_mipmap = std::make_shared<Utils::Mipmap>();
  _histdata = std::make_shared<Utils::HistogramData>();

  // we currently only use one operation to account for histogram changes
  Utils::Ops::HistogramOp *o = new Utils::Ops::HistogramOp();
  o->_scaling.scale = 1.f;
  o->_scaling.min = 0.f;
  o->_scaling.max = 1.f;
  _op = o;

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

const Utils::ImageData* GUI::Layer::img() const{
  return _imgdata.get();
}

bool GUI::Layer::available() const {
  return _available;
}

void GUI::Layer::clear() {
  LOG(INFO) << "GUI::Layer::clear()";

  _available = false;

  _current_mipmap->clear();
  _imgdata->clear();
  _bufdata->clear(false);
}

void GUI::Layer::loadImage(std::string fn) {
  LOG(INFO) << "GUI::Layer::loadImage()";
  _available = false;

  if (_path != "")
    _watcher->removePath(QString::fromStdString(_path));

  _path = fn;
  // we keep the original data here (unscaled)
  _imgdata = std::make_shared<Utils::ImageData>(fn);
  // and for diplaying purposes we use the buffer data (scaled to be within [0, 1])
  _bufdata = std::make_shared<Utils::ImageData>(_imgdata.get());

  // first build histogram
  slotRebuildHistogram();

  Utils::Ops::HistogramOp *o = static_cast<Utils::Ops::HistogramOp*>(_op);
  o->_scaling.scale = _imgdata->max();
  o->_scaling.min = 0;
  o->_scaling.max = _imgdata->max();
  _op = o;
  slotApplyOp(_op);

}

void GUI::Layer::slotRebuildMipmap()  {
  _available = false;
  _working_mipmap = std::make_shared<Utils::Mipmap>();
  _thread_mipmapBuilder->notify(_working_mipmap, _bufdata);
  _thread_mipmapBuilder->start();
}

void GUI::Layer::slotRebuildHistogram()  {
  _thread_histogram->notify(_imgdata, _histdata);
  _thread_histogram->start();
}

void GUI::Layer::slotMipmapFinished()  {
  LOG(INFO) << "GUI::Layer::slotMipmapFinished()";
  // override mipmap with new one
  _current_mipmap = _working_mipmap;
  // watch again for file changes
  _watcher->addPath(QString::fromStdString(_path));
  // allow OpenGL to display
  _available = true;
  //request to display new data
  emit sigRefresh();
}

void GUI::Layer::slotHistogramFinished()  {
  LOG(INFO) << "GUI::Layer::slotHistogramFinished()";
  // slotRefresh(0.f, _imgdata->max());
  emit sigHistogramFinished();
}

void GUI::Layer::slotRefresh(float min, float max)  {

  Utils::Ops::HistogramOp *o = static_cast<Utils::Ops::HistogramOp*>(_op);
  o->_scaling.scale = _imgdata->max();
  o->_scaling.min = min;
  o->_scaling.max = max;
  _op = o;

  LOG(INFO) << "img max " << _imgdata->max();
  LOG(INFO) << "scale " << o->_scaling.scale;
  LOG(INFO) << "min " << o->_scaling.min;
  LOG(INFO) << "max " << o->_scaling.max;

  slotApplyOp(_op);
}

void GUI::Layer::slotApplyOpFinished()  {
  LOG(INFO) << "GUI::Layer::slotApplyOpFinished()";
  slotRebuildMipmap();
  emit sigApplyOpFinished();
}


Utils::HistogramData* GUI::Layer::histogram() const{
  return _histdata.get();
}

std::string GUI::Layer::path() const {
  return _path;
}

void GUI::Layer::slotApplyOp(Utils::Ops::ImgOp* op) {
  _available = false;
  _bufdata = std::make_shared<Utils::ImageData>(_imgdata.get());
  _thread_opWorker->notify(_bufdata, _bufdata, op);
  _thread_opWorker->start();
}

void GUI::Layer::slotPathChanged(QString s) {
  // prevent to much reloads
  _watcher->removePath(QString::fromStdString(s.toStdString()));
  LOG(INFO) << "GUI::Layer::slotPathChanged() " << s.toStdString();
  // wait until file is written
  _thread_Reloader->notify(s.toStdString());
  _thread_Reloader->start();
}

void GUI::Layer::slotFileIsValid(QString s) {
  // file seems to be a valid image file
  LOG(INFO) << "GUI::Layer::slotFileIsValid(" << s.toStdString();
  loadImage(s.toStdString());
}