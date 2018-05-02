#include "image_data.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <QThread>
#include <string>
#include <cmath>
#include <string.h>
#include <glog/logging.h>
#include "misc.h"
#include "Imageloader/freeimage_loader.h"
#include "Imageloader/opticalflow_loader.h"


// threads
// ==========================================================================================
Utils::threads::ImageWriterThread::ImageWriterThread() {
	_running = false;
}

bool Utils::threads::ImageWriterThread::notify( float* copied_buffer,
    int top, int left, int bottom, int right,
    int height, int width, int channels,
    std::string fn) {
	if (_running)
		return false;
	_running = true;
	_buffer = copied_buffer;
	_top = std::max(top, 0);
	_left = std::max(left, 0);
	_bottom = std::min(bottom, height);
	_right = std::min(right, width);
	_height = height;
	_width = width;
	_channels = channels;
	_fn = fn;
	return true;
}

void Utils::threads::ImageWriterThread::run() {
	typedef FIBITMAP* FIBitmapPtr;
	const int n_height = _bottom - _top;
	const int n_width = _right - _left;

	auto read_accessor = [&](int h, int w, int c){return (_channels - c - 1) * (_height * _width) + h * _width + w;};
	auto write_accessor = [&](int h, int w, int c){return h * n_width * _channels + w * _channels + c;};

	BYTE* pixels = new BYTE[3 * n_width * n_height];

	for (int c = 0; c < _channels; ++c) {
		for (int hh = 0; hh < n_height; ++hh) {
			int h = hh + _top;
			for (int ww = 0; ww < n_width; ++ww) {
				int w = ww + _left;
				pixels[write_accessor(hh, ww, c)] = (BYTE) 255. * _buffer[read_accessor(h, w, c)];
			}
		}
	}

	const int bpp = 8 * 3;
	const bool topleft = true;
	const int scanwidth = n_width * 3;

	FIBitmapPtr Image = FreeImage_ConvertFromRawBits(pixels, n_width, n_height,
	                    scanwidth, bpp,
	                    0xFF0000, 0x00FF00, 0x0000FF, topleft);
	FreeImage_Save(FIF_PNG, Image, _fn.c_str(), 0);
	delete[] pixels;
	FreeImage_Unload(Image);
	delete[] _buffer;
	_running = false;
}


/* This file is responsible to load the image data

PNG:
- gray (8)
- gray + alpha (16)
- rgb (24)
- rgb + alpha (40)
JPG:
- rgb (24)
*/

bool Utils::ImageData::knownImageFormat(std::string filename) {
	// todo (remove this construction all the time)
	std::vector<Loader::ImageLoader*> _tmp_loaders;
	_tmp_loaders.push_back(new Loader::FreeImageLoader());
	_tmp_loaders.push_back(new Loader::OpticalFlowLoader());

	for (auto && loader : _tmp_loaders) {
		if (loader->canLoad(filename)) {
			return true;
		}
	}
	return false;

}

Utils::ImageData::~ImageData() {
	registerLoaders();
}

Utils::ImageData::ImageData(float*d, int h, int w, int c)
	: _raw_buf(d), _height(h), _width(w), _channels(c) {
	registerLoaders();
}

void Utils::ImageData::registerLoaders() {
	_loaders.push_back(new Loader::FreeImageLoader());
	_loaders.push_back(new Loader::OpticalFlowLoader());
}

Utils::ImageData::ImageData(Utils::ImageData *img) {
	_height = img->height();
	_width = img->width();
	_channels = img->channels();
	_raw_buf = new float[img->elements()];
	memcpy( _raw_buf, img->data(), sizeof(float) * img->elements() );
}

void Utils::ImageData::write(std::string filename) const {
	write(filename, 0, 0, _height, _width);
}

void Utils::ImageData::writerFinished() {
	QObject * sender = QObject::sender();
	if (sender) {
		threads::ImageWriterThread * t = dynamic_cast<threads::ImageWriterThread*>(sender);
		if ( ! t->isFinished()) {
			DLOG(INFO) << "found running thread";
			t->wait();
		}
		delete t;
	}
}

void Utils::ImageData::write(std::string filename, int t, int l, int b, int r) const {

	threads::ImageWriterThread *writer = new threads::ImageWriterThread();
	float *tmp_buf = new float[elements()];
	memcpy( tmp_buf, _raw_buf, sizeof(float) * elements() );
	if (writer->notify(tmp_buf, t, l, b, r, _height, _width, _channels, filename)) {
		connect( writer, SIGNAL( finished() ), this, SLOT( writerFinished() ));
		writer->start();
	}

}
Utils::ImageData::ImageData(std::string filename) {
	DLOG(INFO) << "Utils::ImageData::ImageData " << filename;
	registerLoaders();

	int l_id = 0;
	for (auto && loader : _loaders) {
		if (loader->canLoad(filename)) {
			DLOG(INFO) << "loader " << l_id << " can load " << filename;
			_raw_buf = loader->load(filename, &_height, &_width, &_channels, &_max_value);
			break;
		} else {
			DLOG(INFO) << "loader " << l_id << " cannot load " << filename;
		}
		l_id++;
	}

	// _raw_buf = _loaders[0]->load(filename, &_height, &_width, &_channels, &_max_value);
}

// pixel value accessors
float Utils::ImageData::operator()(int h, int w, int c) const {
	return value(h, w, c);
}
float Utils::ImageData::value(int h, int w, int c) const {
	return _raw_buf[c * (_height * _width) + h * _width + w];
}

float Utils::ImageData::value(int t, int c) const {
	return _raw_buf[c * (_height * _width) + t];
}


float* Utils::ImageData::data() const {return _raw_buf;}
size_t Utils::ImageData::elements() const {return _height * _width * _channels;}
int Utils::ImageData::width() const {return _width;}
int Utils::ImageData::height() const {return _height;}
int Utils::ImageData::channels() const {return _channels;}
int Utils::ImageData::area() const {return _height * _width;}
float Utils::ImageData::max() const {return _max_value;}

std::string Utils::ImageData::colorString(int h, int w, bool formated) const {
	std::stringstream stream;
	stream << std::setprecision(5);

	const int ch = height();
	const int cw = width();

	CHECK(channels() == 3 || channels() == 1) << "color string only for 1 or 3 channels";

	if (formated) {
		if (0 <= w && w < cw)
			if (0 <= h && h < ch) {
				// within image
				if (channels() == 1) {
					stream << "<font color=" << misc_theme_gray.name().toStdString() << ">"  << value(h, w, 0) << "</font>" << " ";
				}
				if (channels() == 3) {
					stream << "<font color=" << misc_theme_red.name().toStdString() << ">"  << value(h, w, 0) << "</font>" << " ";
					stream << "<font color=" << misc_theme_green.name().toStdString() << ">"  << value(h, w, 1) << "</font>" << " ";
					stream << "<font color=" << misc_theme_blue.name().toStdString() << ">"  << value(h, w, 2) << "</font>" << " ";
				}

			}

	} else {
		if (0 <= w && w < cw)
			if (0 <= h && h < ch) {
				// within image
				if (channels() == 1) {
					stream << value(h, w, 0);
				}
				if (channels() == 3) {
					stream << value(h, w, 0) << ", ";
					stream << value(h, w, 1) << ", ";
					stream << value(h, w, 2);
				}

			}
	}

	return stream.str();
}


void Utils::ImageData::clear(bool remove) {
	DLOG(INFO) << "Utils::ImageData::clear";
	// the _buf_data is already delete (so dont do it here again)
	if (remove)
		if (_raw_buf != nullptr)
			delete[] _raw_buf;
	_height = 0;
	_width = 0;
	_channels = 0;
}

