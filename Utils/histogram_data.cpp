#include "histogram_data.h"

#include <limits>
#include <random>

#include <glog/logging.h>

#include "image_data.h"


void Utils::HistogramData::setScale(int k) {
  _scale_mapping_id = k;
}

float Utils::HistogramData::scale_func(float k) {
  return _scale_mappings[_scale_mapping_id](k);
}

Utils::HistogramData::HistogramData() :
  _nbins(0), _channels(0), _available(false) {
  _bin_info.min = 0.;
  _bin_info.max = 1.;

  _scale_mappings.push_back([](float i)->float{ return i;});
  _scale_mappings.push_back([](float i)->float{ return log(i);});
  _scale_mapping_id = 0;
}

void Utils::HistogramData::setData(const ImageData *data, float scale) {
  _available = false;

  _nbins = 256;
  _channels = data->channels();
  _img = data;

  // TODO remove assumption of range [0, 1]
  const int sampling_freq = 1;

  // create bin data
  _bin_info.clear();
  _data.clear();

  _range.min = 0;
  _range.max = scale;

  // range
  const double bin_width = _range.range() / static_cast<double>(_nbins);
  DLOG(INFO) << "bin_width " << bin_width;

  for (int c = 0; c < _channels; ++c) {
    std::vector<double> channelBins(_nbins, 0.);
    for (int n = 0; n < data->area(); n += sampling_freq) {
      const double value = data->value(n, c);

      int idx = value / bin_width;
      if (idx < 0 || idx >= _nbins) continue;
      channelBins[idx]++;
      _bin_info.update(channelBins[idx]);
    }
    _data.push_back(channelBins);
  }

  _available = true;
}

const Utils::ImageData* Utils::HistogramData::image() const {
  return _img;
}

Utils::HistogramData::range_t* Utils::HistogramData::range() {
  return &_range;
}

const Utils::HistogramData::range_t* Utils::HistogramData::range() const {
  return &_range;
}

int Utils::HistogramData::amount(int channel, int bin) const {
  if (channel < 0 || channel >= _channels ||
      bin < 0 || bin >= _nbins)
    return 0;
  else
    return _data[channel][bin];
}

bool Utils::HistogramData::available() const {
  return _available;
}
int Utils::HistogramData::bins() const {
  return _nbins;
}
int Utils::HistogramData::channels() const {
  return _channels;
}
const Utils::HistogramData::bin_info_t Utils::HistogramData::bin_info() const {
  return _bin_info;
}
