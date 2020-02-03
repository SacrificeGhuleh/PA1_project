//
// Created by zvone on 22-Nov-19.
//

#include <thread>
#include <random>
#include <iostream>
#include <future>

#include <omp.h>
#include "seamcarving.h"

#include <opencv2/core/mat.hpp>   //cv::Mat
#include <opencv2/highgui.hpp>    //cv::imshow, cv::waitKey
#include <opencv2/imgproc.hpp>  //cv::imread


#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

// About Desktop OpenGL function loaders:
//  Modern desktop OpenGL doesn't have a standard portable header file to load OpenGL function pointers.
//  Helper libraries are often used for this purpose! Here we are supporting a few common ones (gl3w, glew, glad).
//  You may use another loader/header of your choice (glext, glLoadGen, etc.), or chose to manually implement your own.
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
  
  #include <GL/gl3w.h>    // Initialize with gl3wInit()

#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
#include <GL/glew.h>    // Initialize with glewInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
  #include <glad/glad.h>  // Initialize with gladLoadGL()
#else

  #include IMGUI_IMPL_OPENGL_LOADER_CUSTOM

#endif

// Include glfw3.h after our OpenGL definitions

#include <GLFW/glfw3.h>


using namespace std::chrono_literals;

template<typename T>
inline T sqr(T val) {
  return val * val;
}

std::mt19937_64 engine(
    static_cast<uint64_t> (std::chrono::system_clock::to_time_t(std::chrono::system_clock::now())));

//std::uniform_real_distribution<double>(0.0, 1.0);

int SeamCarving::computeThreads_ = omp_get_max_threads() - 1; //1 thread is used for gui
float SeamCarving::producerTime_ = 0.f;

static float imgScale = 1.f;

#define MULTITHREAD 1

void GetDesktopResolution(int &horizontal, int &vertical) {
  RECT desktop;
  // Get a handle to the desktop window
  const HWND hDesktop = GetDesktopWindow();
  // Get the size of screen to the variable desktop
  GetWindowRect(hDesktop, &desktop);
  // The top left corner will have coordinates (0,0)
  // and the bottom right corner will have coordinates
  // (horizontal, vertical)
  horizontal = desktop.right;
  vertical = desktop.bottom;
}


void SeamCarving::ui() {
  {
    ImGui::Begin("Info & Control");
    ImGui::SliderFloat("Image scale", &imgScale, 0.2f, 10.0f);
    
    static bool experimentalThreads = false;
    ImGui::Checkbox("Unlimited threads", &experimentalThreads);
    
    if (experimentalThreads) {
      ImGui::Text("Unlimited threads, only for experiments");
      ImGui::SliderInt("Compute threads", &computeThreads_, 1, omp_get_max_threads() * omp_get_max_threads());
    } else {
      ImGui::SliderInt("Compute threads", &computeThreads_, 1, omp_get_max_threads());
    }
    
    ImGui::Separator();
    
    ImGui::Text("Rendering average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
                ImGui::GetIO().Framerate);
    ImGui::Text("Computing time %.3f ms/generation (%.1f Generations/s)", producerTime_ * 1000.f,
                1000.f / (producerTime_ * 1000.f));
    ImGui::Separator();
    
    ImGui::Text(" Generation: %d", generation_);
    ImGui::End();
  }
  
  {
    std::lock_guard<std::mutex> lock(tex_data_lock_);
    matRenderer_->render(imgScale);
    energyRenderer_->render(imgScale);
    seamsRenderer_->render(imgScale);
  }
}

void SeamCarving::initTextures() {
  Gui::initTextures();
  
  
  seamImage_ = cv::imread(filename_);
  seamImageMask_ = cv::Mat(seamImage_.rows, seamImage_.cols, CV_32F, 0.f);
  seamImageRenderHelperMat_ = cv::Mat(seamImage_.rows, seamImage_.cols, CV_8UC3, cv::Scalar(0, 0, 0));
  allSeamsMat_ = cv::Mat(seamImage_.rows, seamImage_.cols, CV_8UC3, cv::Scalar(0, 0, 0));
  
  matRenderer_ = std::make_unique<CvMatRenderer>(seamImage_.rows, seamImage_.cols, seamImage_.data, "Seam carving");
  energyRenderer_ = std::make_unique<CvMatRenderer>(seamImageRenderHelperMat_.rows, seamImageRenderHelperMat_.cols,
                                                    seamImageRenderHelperMat_.data,
                                                    "Energy");
  seamsRenderer_ = std::make_unique<CvMatRenderer>(allSeamsMat_.rows, allSeamsMat_.cols, allSeamsMat_.data,
                                                   "Seams");
  
  
  int desktopWidth, desktopHeight;
  
  GetDesktopResolution(desktopWidth, desktopHeight);
  
  imgScale = static_cast<float>(desktopWidth / 2.f) / static_cast<float>(seamImage_.cols);
}

void SeamCarving::render() {
  //initialize rendering
  preRenderInit();
  
  std::thread producer_thread(&SeamCarving::producer, this);
  renderLoop();
  finish_request_.store(true, std::memory_order_release);
  producer_thread.join();
}

void SeamCarving::producer() {
  float t = 0.0f; // time
  auto t0 = std::chrono::high_resolution_clock::now();
  
  while (!finish_request_.load(std::memory_order_acquire)) {
    auto t1 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> dt = t1 - t0;
    producerTime_ = dt.count();
    t += producerTime_;
    t0 = t1;
    
    const unsigned int rows = seamImage_.rows;
    const unsigned int cols = seamImage_.cols - generation_;
    
    
    cv::Mat grayscaleMat = cv::Mat(rows, cols, CV_8UC1, 0.f);
    
    #if MULTITHREAD
      #pragma omp parallel for default(none) num_threads(computeThreads_) shared(grayscaleMat, seamImage_)
    #endif
    for (int row = 0; row < seamImage_.rows; ++row) {
      for (int col = 0; col < seamImage_.cols; ++col) {
        cv::Vec3b pix = seamImage_.at<cv::Vec3b>(row, col);
        grayscaleMat.at<uint8_t>(row, col) = (pix.val[0] * 0.11f) + (pix.val[1] * 0.59) + (pix.val[2] * 0.3);
      }
    }
    
    //compute energy
    
    #if MULTITHREAD
      #pragma omp parallel for default(none) num_threads(computeThreads_)  shared(grayscaleMat, seamImageMask_, rows, cols)
    #endif
    for (int row = 1; row < rows - 1; row++) {
      for (int col = 1; col < cols - 1; col++) {
        
        /*
         * Sobel operator for energy
         * */
        uint8_t pix0 = grayscaleMat.at<uint8_t>(row - 1, col - 1);
        uint8_t pix1 = grayscaleMat.at<uint8_t>(row - 1, col);
        uint8_t pix2 = grayscaleMat.at<uint8_t>(row - 1, col + 1);
        uint8_t pix3 = grayscaleMat.at<uint8_t>(row, col - 1);
        uint8_t pix4 = grayscaleMat.at<uint8_t>(row, col);
        uint8_t pix5 = grayscaleMat.at<uint8_t>(row, col + 1);
        uint8_t pix6 = grayscaleMat.at<uint8_t>(row + 1, col - 1);
        uint8_t pix7 = grayscaleMat.at<uint8_t>(row + 1, col);
        uint8_t pix8 = grayscaleMat.at<uint8_t>(row + 1, col + 1);
        float val = 0;
        
        float hx = -pix0 + pix2 - 2 * pix3 + 2 * pix5 - pix6 + pix8;
        float hy = -pix0 - 2 * pix1 - pix2 + pix6 + 2 * pix7 + pix8;
        
        val = sqrt(sqr(hx) + sqr(hy));
        
        seamImageMask_.at<float>(row, col) = val / 8;
      }
    }
    
    //find vertical seam
    
    cv::Mat seams(rows, cols, CV_16SC1);
    
    
    #if MULTITHREAD
      #pragma omp parallel for default(none) num_threads(computeThreads_)  shared(seams, rows, cols)
    #endif
    for (int col = 1; col < cols - 1; col++) {
      int newCol = col;
      for (int row = 0; row < rows - 1; row++) {
        float possibleEnergy1 = seamImageMask_.at<float>(row + 1, newCol - 1);
        float possibleEnergy2 = seamImageMask_.at<float>(row + 1, newCol);
        float possibleEnergy3 = seamImageMask_.at<float>(row + 1, newCol + 1);
        
        if (possibleEnergy1 < possibleEnergy2 && possibleEnergy1 < possibleEnergy3) {
          newCol = newCol - 1;
        }
        
        if (possibleEnergy2 < possibleEnergy1 && possibleEnergy2 < possibleEnergy3) {
          newCol = newCol;
        }
        
        if (possibleEnergy3 < possibleEnergy1 && possibleEnergy3 < possibleEnergy2) {
          newCol = newCol + 1;
        }
        newCol = std::min<int>(std::max<int>(newCol, 0), cols - 1);
        seams.at<int16_t>(row, col) = newCol;
      }
    }
    
    cv::Mat cumulativeEnergyMat(1, cols, CV_16SC1);
    
    cv::Mat seam(rows, 1, CV_16SC1);
    
    
    #if MULTITHREAD
      #pragma omp parallel for default(none) num_threads(computeThreads_)  shared(seams, cumulativeEnergyMat, rows, cols)
    #endif
    for (int col = 5; col < cols - 5; col++) {
      int cumulativeEnergy = 0;
      for (int row = 0; row < rows - 1; row++) {
        int seamCol = seams.at<int16_t>(row, col);
        assert(seamImageMask_.at<float>(row, seamCol) >= 0.f);
        cumulativeEnergy += seamImageMask_.at<float>(row, seamCol);
      }
      cumulativeEnergyMat.at<int16_t>(col) = cumulativeEnergy;
    }
    
    int minCumulativeIndex = 1;
    int minCumulativeEnergy = std::numeric_limits<int>::max();
    
    
    #if MULTITHREAD
      #pragma omp parallel for default(none) num_threads(computeThreads_)  shared(seams, cumulativeEnergyMat, minCumulativeEnergy, minCumulativeIndex, rows, cols)
    #endif
    for (int col = 1; col < seams.cols - 1; col++) {
      
      if (cumulativeEnergyMat.at<int16_t>(col) < minCumulativeEnergy) {
        minCumulativeEnergy = cumulativeEnergyMat.at<int16_t>(col);
        minCumulativeIndex = col;
        continue;
      }
      
      if (cumulativeEnergyMat.at<int16_t>(col) == minCumulativeEnergy && (rand() % 2)) {
        minCumulativeEnergy = cumulativeEnergyMat.at<int16_t>(col);
        minCumulativeIndex = col;
      }
    }
    seam = seams.col(minCumulativeIndex);
    
    //remove seam
    #if MULTITHREAD
      #pragma omp parallel for default(none) num_threads(computeThreads_)  shared(seamImage_, seam, rows, cols)
    #endif
    for (int row = 0; row < rows; ++row) {
      for (int col = seam.at<int16_t>(row); col < cols - 1; ++col) {
        seamImage_.at<cv::Vec3b>(row, col) = seamImage_.at<cv::Vec3b>(row, col + 1);
      }
    }
    
    #if MULTITHREAD
      #pragma omp parallel for default(none) num_threads(computeThreads_)  shared(seamImage_, rows, cols)
    #endif
    for (int row = 0; row < rows; ++row) {
      seamImage_.at<cv::Vec3b>(row, cols - 1) = cv::Vec3b(0, 0, 0);
    }
    
    
    cv::normalize(seamImageMask_, seamImageMask_, 1, 0, cv::NORM_MINMAX);
    
    #if MULTITHREAD
      #pragma omp parallel for default(none) num_threads(computeThreads_) shared(seamImageRenderHelperMat_, seam, seamImageMask_, rows, cols, allSeamsMat_)
    #endif
    for (int row = 0; row < seamImageRenderHelperMat_.rows; row++) {
      for (int col = 0; col < seamImageRenderHelperMat_.cols; col++) {
        float loc_pix = seamImageMask_.at<float>(row, col);
        seamImageRenderHelperMat_.at<cv::Vec3b>(row, col) = cv::Vec3b(loc_pix * 255, loc_pix * 255, loc_pix * 255);
        allSeamsMat_.at<cv::Vec3b>(row, col) = cv::Vec3b(0, 0, 0);
      }
    }
    
    #if MULTITHREAD
      #pragma omp parallel for default(none) num_threads(computeThreads_)  shared(seamImageRenderHelperMat_, seam, rows, cols, seams, allSeamsMat_)
    #endif
    for (int row = 0; row < rows; row++) {
      seamImageRenderHelperMat_.at<cv::Vec3b>(row, seam.at<int16_t>(row)) = cv::Vec3b(0, 0, 255);
      seamImageRenderHelperMat_.at<cv::Vec3b>(row,
                                              std::min<int>(std::max<int>(seam.at<int16_t>(row) + 1, 0), cols - 1)) =
          cv::Vec3b(0, 0, 255);
      seamImageRenderHelperMat_.at<cv::Vec3b>(row,
                                              std::min<int>(std::max<int>(seam.at<int16_t>(row) + 2, 0), cols - 1)) =
          cv::Vec3b(0, 0, 255);
      seamImageRenderHelperMat_.at<cv::Vec3b>(row,
                                              std::min<int>(std::max<int>(seam.at<int16_t>(row) - 1, 0), cols - 1)) =
          cv::Vec3b(0, 0, 255);
      seamImageRenderHelperMat_.at<cv::Vec3b>(row,
                                              std::min<int>(std::max<int>(seam.at<int16_t>(row) - 2, 0), cols - 1)) =
          cv::Vec3b(0, 0, 255);
      
      for (int col = 0; col < cols; col++) {
        allSeamsMat_.at<cv::Vec3b>(row,std::min<int>(std::max<int>(seams.at<int16_t>(row, col) - 2, 0), cols - 1)) =
            cv::Vec3b(0, 0, 255);
      }
      
    }
    
    {
      std::lock_guard<std::mutex> lock(tex_data_lock_);
      generation_++;
    }
  }
  
  std::cout << "Producer thread stop\n";
}

SeamCarving::SeamCarving(const char *filename) :
    Gui("Game of life"),
    generation_{0},
    filename_{filename} {
}
