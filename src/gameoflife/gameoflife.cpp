//
// Created by zvone on 22-Nov-19.
//

#include <thread>
#include "gameoflife.h"

using namespace std::chrono_literals;

void GameOfLife::ui() {
  Gui::ui();
  {
    std::lock_guard<std::mutex> lock(tex_data_lock_);
    originalImage_->render();
  }
}

void GameOfLife::initTextures() {
  Gui::initTextures();

//  originalImage_ = new GpuImage<200,200,3>("Example random noise image");
  originalImage_ = std::make_unique<GpuImage<200, 200, 3>>("Example random noise image");
}

void GameOfLife::render() {
  preRenderInit();
  
  std::thread producer_thread(&GameOfLife::producer, this);
  renderLoop();
  finish_request_.store(true, std::memory_order_release);
  producer_thread.join();
}

void GameOfLife::producer() {
  while (!finish_request_.load(std::memory_order_acquire)) {
    //if (originalImage_ == nullptr) continue;
    for (int row = 0; row < originalImage_->rows; row++) {
      for (int col = 0; col < originalImage_->cols; col++) {
        Pixel<3> &pix = originalImage_->at(row, col);
        
        pix.r = rand() % 255;
        pix.g = rand() % 255;
        pix.b = rand() % 255;
      }
    }
  }
  std::cout << "Producer thread stop\n";
}
