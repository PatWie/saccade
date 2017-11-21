#include "histogram_op.h"
#include <cmath>

#include <glog/logging.h>

namespace Utils {
namespace Ops {
#ifdef CUDA_ENABLED
namespace {
__global__ void cuda(const float *src, float *dst,
                     size_t H, size_t W, size_t C,
                     Utils::Ops::HistogramOp::scaling_t _scaling) {
  const int c = blockIdx.z;
  for (int w = blockIdx.x * blockDim.x + threadIdx.x; w < W; w += blockDim.x * gridDim.x) {
    for (int h = blockIdx.y * blockDim.y + threadIdx.y; h < H; h += blockDim.y * gridDim.y) {
      float inp = src[c * H * W + h * W + w];
      float scaled = inp / _scaling.scale;
      scaled -= _scaling.min / _scaling.scale;
      scaled /= (_scaling.max - _scaling.min) / _scaling.scale;
      // apply clipping
      scaled = min(scaled, 1.f);
      scaled = max(scaled, 0.f);
      dst[c * H * W + h * W + w] = scaled;
    }
  }
}
}; // anonymous namespace
#endif // CUDA_ENABLED

HistogramOp::HistogramOp() {}

void HistogramOp::apply_cpu(const float* src, float* dst,
                            size_t H, size_t W, size_t C) {

  #pragma omp parallel for
  for (size_t i = 0; i < H * W * C; ++i) {
    dst[i] = src[i];
    // map from [0, img_max] _> [0, 1]
    float scaled = src[i] / _scaling.scale;
    scaled -= _scaling.min / _scaling.scale;
    scaled /= (_scaling.max - _scaling.min) / _scaling.scale;
    // apply clipping
    scaled = std::min(scaled, 1.f);
    dst[i] = std::max(scaled, 0.f);
  }
}

#ifdef CUDA_ENABLED
void HistogramOp::apply_gpu(const float* src, float* dst,
                            size_t H, size_t W, size_t C) {

  float *d_src;
  float *d_dst;

  cudaMalloc(&d_src, sizeof(float) * H * W * C);
  cudaMalloc(&d_dst, sizeof(float) * H * W * C);

  cudaMemcpy(d_src, src, sizeof(float) * H * W * C, cudaMemcpyHostToDevice);
  cudaMemcpy(d_dst, dst, sizeof(float) * H * W * C, cudaMemcpyHostToDevice);

  const int num_threads = 32;
  dim3 threads(num_threads, num_threads);
  dim3 grid((W - 1) / threads.x + 1, (H - 1) / threads.y + 1, C);
  cuda <<< grid, threads>>> (d_src, d_dst, H, W, C, _scaling);
  cudaDeviceSynchronize();

  cudaMemcpy(dst, d_dst, sizeof(float) * H * W * C, cudaMemcpyDeviceToHost);

  cudaFree(d_src);
  cudaFree(d_dst);

}
#endif // CUDA_ENABLED


}
}
