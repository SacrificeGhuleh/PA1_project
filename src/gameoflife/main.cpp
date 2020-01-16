// dear imgui - standalone example application for OpenGL
// If you are new to dear imgui, see examples/README.txt and documentation at the top of imgui.cpp.

#include "gameoflife.h"
#include <thread>
#include <iostream>

#ifndef NDEBUG
#include <iostream>
#include <mat.h>

#endif //NDEBUG

// Main code
int main(int, char **) {
#ifndef NDEBUG
  testMats();
#endif //NDEBUG
  
  unsigned int n = std::thread::hardware_concurrency();
  std::cout << n << " concurrent threads are supported.\n";
  
  GameOfLife gui;
  
  gui.render();
  
  return 0;
}
