//
// Created by zvone on 22-Nov-19.
//

#include <thread>
#include <random>
#include <iostream>
#include <future>
#include "gameoflife.h"


using namespace std::chrono_literals;

static Pixel<3> DEAD = {0, 0, 0};
static Pixel<3> ALIVE = {255, 255, 255};

static float producerTime;

std::mt19937_64 engine(
    static_cast<uint64_t> (std::chrono::system_clock::to_time_t(std::chrono::system_clock::now())));
std::uniform_real_distribution<double> rngDistribution(0.0, 1.0);

int GameOfLife::computeThreads_ = std::thread::hardware_concurrency() * 4;

#define MULTITHREAD 1

void GameOfLife::ui() {
  static float imgScale = 1.f;
  {
    ImGui::Begin("Info & Control");
    ImGui::SliderFloat("Image scale", &imgScale, 0.2f, 10.0f);
    ImGui::SliderInt("Compute threads", &computeThreads_, 1, 20);
    
    ImGui::Separator();
    
    ImGui::Text("Rendering average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
                ImGui::GetIO().Framerate);
    ImGui::Text("Computing time %.3f ms/frame (%.1f FPS)", producerTime * 1000.f,
                1000.f / (producerTime * 1000.f));
    ImGui::End();
  }
  
  {
    std::lock_guard<std::mutex> lock(tex_data_lock_);
    renderImage_->render(imgScale);
  }
}

void GameOfLife::initTextures() {
  Gui::initTextures();
  
  renderImage_ = std::make_unique<GpuImage<SIMULATION_HEIGHT, SIMULATION_WIDTH, CHANNELS>>("Game Of Life Render");
  modelImage_ = std::make_unique<GpuImage<SIMULATION_HEIGHT, SIMULATION_WIDTH, CHANNELS>>("Game Of Life Render");
  
  const unsigned int cols = renderImage_->cols;
  const unsigned int rows = renderImage_->cols;
  
  
  #if MULTITHREAD
    #pragma omp parallel for default(none) num_threads(computeThreads_) shared(modelImage_, renderImage_, precomputedOffsets_, DEAD, ALIVE, cols, rows, rngDistribution, engine)
  #endif
  for (int row = 0; row < rows; row++) {
    for (int col = 0; col < cols; col++) {
      float randomNumber = 0.f; //implicitly OMP private
      
      if (row == 0 || col == 0 || row == renderImage_->rows - 1 || col == renderImage_->cols - 1) {
        modelImage_->at(row, col) = DEAD;
        renderImage_->at(row, col) = DEAD;
      } else {
        PrecomputeData precomputeData;
        
        precomputeData.firstIdx = idx2dTo1d(row - 1, col - 1, cols) * renderImage_->channels;
        precomputeData.secondIdx = idx2dTo1d(row, col - 1, cols) * renderImage_->channels;
        precomputeData.thirdIdx = idx2dTo1d(row + 1, col - 1, cols) * renderImage_->channels;
        precomputeData.centerIndex = idx2dTo1d(row, col, cols);
        
        #ifndef NDEBUG
        precomputedOffsets_.at(col).at(row) = precomputeData;
        #else
        precomputedOffsets_[col][row] = precomputeData;
        #endif
        
        #pragma omp critical
        {
          randomNumber = rngDistribution(engine);
        }
        
        if (randomNumber > 0.5) {
          modelImage_->at(row, col) = ALIVE;
          renderImage_->at(row, col) = ALIVE;
        } else {
          modelImage_->at(row, col) = DEAD;
          renderImage_->at(row, col) = DEAD;
        }
      }
    }
  }
}

void GameOfLife::render() {
  //initialize rendering
  preRenderInit();
  
  std::thread producer_thread(&GameOfLife::producer, this);
  renderLoop();
  finish_request_.store(true, std::memory_order_release);
  producer_thread.join();
}

void GameOfLife::computePartOfTheGameBoardIndexed(int fromIndex, int toIndex) {
  for (int idx = fromIndex; idx < toIndex - 1; idx++) {
    int col = idx % renderImage_->cols;
    int row = idx / renderImage_->cols;
    simulate(row, col);
  }
}

void GameOfLife::computePartOfTheGameBoard(int fromRow, int toRow, int fromCol, int toCol) {
  for (int row = fromRow; row < toRow - 1; row++) {
    for (int col = fromCol; col < toCol - 1; col++) {
      simulate(row, col);
    }
  }
}

void GameOfLife::producer() {
  float t = 0.0f; // time
  auto t0 = std::chrono::high_resolution_clock::now();
  
  while (!finish_request_.load(std::memory_order_acquire)) {
    auto t1 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> dt = t1 - t0;
    producerTime = dt.count();
//    t += dt.count();
    t += producerTime;
    t0 = t1;
    #if MULTITHREAD
      #pragma omp parallel for default(none) num_threads(computeThreads_)  shared(modelImage_, renderImage_, precomputedOffsets_)
    #endif
    for (int row = 1; row < modelImage_->rows - 1; row++) {
      for (int col = 1; col < modelImage_->cols - 1; col++) {
        simulate(row, col);
      }
    }
    
    {
      //Lock image for swap
      std::lock_guard<std::mutex> lock(tex_data_lock_);
      //Swap buffers
      std::swap(modelImage_, renderImage_);
    }
  }
  
  std::cout << "Producer thread stop\n";
}


void GameOfLife::simulate(int row, int col) {
  #ifndef NDEBUG
  const PrecomputeData precomputeData = precomputedOffsets_.at(row).at(col);
  #else
  const PrecomputeData precomputeData = precomputedOffsets_[row][col];
  #endif
  
  
  PackOfThreePixels<3> firstRow = reinterpret_cast<PackOfThreePixels<3> &>(renderImage_->atRaw(
      precomputeData.firstIdx));
  PackOfThreePixels<3> secondRow = reinterpret_cast<PackOfThreePixels<3> &>(renderImage_->atRaw(
      precomputeData.secondIdx));
  PackOfThreePixels<3> thirdRow = reinterpret_cast<PackOfThreePixels<3> &>(renderImage_->atRaw(
      precomputeData.thirdIdx));
  
  int aliveNeighbors = firstRow.first.r +
                       firstRow.second.r +
                       firstRow.third.r +
                       secondRow.first.r +
                       secondRow.third.r +
                       thirdRow.first.r +
                       thirdRow.second.r +
                       thirdRow.third.r;
  aliveNeighbors /= ALIVE.r;
  
  if (secondRow.second.r == DEAD.r) {
    if (aliveNeighbors == 3) {
      secondRow.second = ALIVE;
    }
  } else {
    if (aliveNeighbors < 2 || aliveNeighbors > 3) {
      secondRow.second = DEAD;
    }
    
    if (aliveNeighbors == 2 || aliveNeighbors == 3) {
      secondRow.second = ALIVE;
    }
  }
  modelImage_->at(precomputeData.centerIndex) = secondRow.second;
}

GameOfLife::GameOfLife() : Gui("Game of life") {
  for (int row = 0; row < SIMULATION_HEIGHT; row++) {
    precomputedOffsets_.emplace_back(std::vector<PrecomputeData>());
    for (int col = 0; col < SIMULATION_WIDTH; col++) {
      precomputedOffsets_[row].emplace_back(); //Just allocate, insert empty
    }
  }
}
