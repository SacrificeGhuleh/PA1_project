//
// Created by zvone on 22-Nov-19.
//
#ifndef NXPCARINTERFACE_NXPCARINTERFACE_H
#define NXPCARINTERFACE_NXPCARINTERFACE_H

#define IMGUI_IMPL_OPENGL_LOADER_GL3W

#include <memory>
#include <mutex>
#include <atomic>

#include <gui.h>
#include <gpuimage.h>

#define SIMULATION_WIDTH  4096
#define SIMULATION_HEIGHT 4096
#define CHANNELS 3

class GameOfLife : public Gui {
public:
  GameOfLife() : Gui("Game of life") {};
  
  virtual ~GameOfLife() = default;
  
  virtual void render() override;

protected:
  
  virtual void ui() override;
  
  virtual void initTextures() override;
  
  virtual void producer();
  
  void computePartOfTheGameBoard(int fromRow, int toRow, int fromCol, int toCol);
  void computePartOfTheGameBoardIndexed(int fromIndex, int toIndex);

private:
  std::mutex tex_data_lock_;
  
  std::atomic<bool> finish_request_{false};
  
  std::unique_ptr<GpuImage<SIMULATION_HEIGHT, SIMULATION_WIDTH, CHANNELS>> renderImage_;
  std::unique_ptr<GpuImage<SIMULATION_HEIGHT, SIMULATION_WIDTH, CHANNELS>> modelImage_;
};


#endif //NXPCARINTERFACE_NXPCARINTERFACE_H