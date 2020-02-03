//
// Created by zvone on 02-Feb-20.
//

#ifndef PA1_PROJECT_CVMATRENDERER_H
#define PA1_PROJECT_CVMATRENDERER_H


#include <opencv2/core/mat.hpp>   //cv::Mat

class CvMatRenderer {
public: //methods
  
  CvMatRenderer(unsigned int rows, unsigned int cols, const char *imgName);
  
  explicit CvMatRenderer(unsigned int rows, unsigned int cols, const uint8_t *data, const char *imgName);
  
  virtual ~CvMatRenderer() = default;
  
  void render(float scale = 1.f);
  
  unsigned int getGpuTexturePtrId() const;

public: //fields
  const char *name;
  void refresh(unsigned int rows, unsigned int cols, const uint8_t *data);
private: //methods
  void refresh();
  
  void assign();

private: //fields
  unsigned int gpuTexturePtrId;
  union {
    unsigned int cols;
    unsigned int width;
  };
  union {
    unsigned int rows;
    unsigned int height;
  };
  const uint8_t *data_;
};


#endif //PA1_PROJECT_CVMATRENDERER_H
