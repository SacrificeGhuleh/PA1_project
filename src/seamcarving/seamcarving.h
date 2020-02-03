//
// Created by zvone on 22-Nov-19.
//
#ifndef SEAMCARVING_H
#define SEAMCARVING_H

#define IMGUI_IMPL_OPENGL_LOADER_GL3W

#include <memory>
#include <mutex>
#include <atomic>
#include <vector>

#include <gui.h>
#include <opencv2/core/mat.hpp>   //cv::Mat

#include "cvmatrenderer.h"

class SeamCarving : public Gui {
public:
  SeamCarving(const char* filename);
  
  virtual ~SeamCarving() = default;
  
  virtual void render() override;

protected:
  
  virtual void ui() override;
  
  virtual void initTextures() override;
  
  virtual void producer();

private:
  static float producerTime_;
  static int computeThreads_;
  std::mutex tex_data_lock_;
  std::atomic<bool> finish_request_{false};
  
  unsigned int generation_;
  
  const char* filename_;
  
  cv::Mat seamImage_;
  cv::Mat seamImageMask_;
  cv::Mat seamImageRenderHelperMat_;
  cv::Mat allSeamsMat_;
  
  std::unique_ptr<CvMatRenderer> matRenderer_;
  std::unique_ptr<CvMatRenderer> energyRenderer_;
  std::unique_ptr<CvMatRenderer> seamsRenderer_;
};


#endif //SEAMCARVING_H