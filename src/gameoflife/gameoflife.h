//
// Created by zvone on 22-Nov-19.
//
#ifndef NXPCARINTERFACE_NXPCARINTERFACE_H
#define NXPCARINTERFACE_NXPCARINTERFACE_H

#define IMGUI_IMPL_OPENGL_LOADER_GL3W

#include <memory>
#include <mutex>
#include <atomic>
#include <vector>

#include <gui.h>
#include <gpuimage.h>

#define SIMULATION_WIDTH  4096
#define SIMULATION_HEIGHT 4096
#define CHANNELS 3

struct PrecomputeData{
  unsigned int firstIdx;
  unsigned int secondIdx;
  unsigned int thirdIdx;
  unsigned int centerIndex;
};

template<int T_CHANNELS>
struct PackOfThreePixels {
  Pixel<T_CHANNELS> first;
  Pixel<T_CHANNELS> second;
  Pixel<T_CHANNELS> third;
};

class GameOfLife : public Gui {
public:
  GameOfLife();
  
  virtual ~GameOfLife() = default;
  
  virtual void render() override;

protected:
  
  virtual void ui() override;
  
  virtual void initTextures() override;
  
  virtual void producer();
  
private:
  
  static int computeThreads_;
  
  void computePartOfTheGameBoard(int fromRow, int toRow, int fromCol, int toCol);
  void computePartOfTheGameBoardIndexed(int fromIndex, int toIndex);
  
  virtual void simulate(int row, int col);
  std::mutex tex_data_lock_;
  
  std::atomic<bool> finish_request_{false};
  
  std::unique_ptr<GpuImage<SIMULATION_HEIGHT, SIMULATION_WIDTH, CHANNELS>> renderImage_;
  std::unique_ptr<GpuImage<SIMULATION_HEIGHT, SIMULATION_WIDTH, CHANNELS>> modelImage_;
  
  std::vector<std::vector<PrecomputeData>> precomputedOffsets_;
};


#endif //NXPCARINTERFACE_NXPCARINTERFACE_H