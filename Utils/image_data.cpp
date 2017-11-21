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


// threads
// ==========================================================================================
Utils::threads::ImageWriterThread::ImageWriterThread() {
	_running = false;
}

bool Utils::threads::ImageWriterThread::notify( float* copied_buffer,
    int t, int l, int b, int r,
    int height, int width, int channels,
    std::string fn) {
	if (_running)
		return false;
	_running = true;
	_buf = copied_buffer;
	_t = std::max(t, 0);
	_l = std::max(l, 0);
	_b = std::min(b, height);
	_r = std::min(r, width);
	_height = height;
	_width = width;
	_channels = channels;
	_fn = fn;
	return true;
}

void Utils::threads::ImageWriterThread::run() {
	typedef FIBITMAP* FIBitmapPtr;
	const int n_height = _b - _t;
	const int n_width = _r - _l;

	BYTE* pixels = new BYTE[3 * n_width * n_height];

	for (int c = 0; c < _channels; ++c) {
		for (int hh = 0; hh < n_height; ++hh) {
			int h = hh + _t;
			for (int ww = 0; ww < n_width; ++ww) {
				int w = ww + _l;
				pixels[hh * n_width * _channels + ww * _channels + c] = (BYTE) 255. * _buf[(_channels - c - 1) * (_height * _width) + h * _width + w];
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
	delete[] _buf;
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

bool Utils::ImageData::validFile(std::string filename) {
	const FREE_IMAGE_FORMAT format = FreeImage_GetFileType(filename.c_str(), 0);
	return (format != FIF_UNKNOWN);
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
}

Utils::ImageData::ImageData(Utils::ImageData *i) {
	_height = i->height();
	_width = i->width();
	_channels = i->channels();
	_raw_buf = new float[i->elements()];
	memcpy( _raw_buf, i->data(), sizeof(float) * i->elements() );
}

void Utils::ImageData::write(std::string filename) const {

	threads::ImageWriterThread *writer = new threads::ImageWriterThread();
	float *tmp_buf = new float[elements()];
	memcpy( tmp_buf, _raw_buf, sizeof(float) * elements() );
	if (writer->notify(tmp_buf, 0, 0, _height, _width, _height, _width, _channels, filename)) {
		connect( writer, SIGNAL( finished() ), this, SLOT( writerFinished() ));
		writer->start();

	}
}

void Utils::ImageData::writerFinished() {
	QObject * sender = QObject::sender();
	if (sender) {
		threads::ImageWriterThread * t = dynamic_cast<threads::ImageWriterThread*>(sender);
		if ( ! t->isFinished()) {
			DLOG(INFO) << "found running thread";
			t->wait();
			delete t;
		} else
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
	registerLoaders();
	_raw_buf = _loaders[0]->load(filename, &_height, &_width, &_channels, &_max_value);
}


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

std::string Utils::ImageData::color(int h, int w, bool formated) const {
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
	// the _bud_data is already delete (so dont do it here again)
	if (remove)
		if (_raw_buf != nullptr)
			delete[] _raw_buf;
	_height = 0;
	_width = 0;
	_channels = 0;
}

