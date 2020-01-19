#include "gameoflife.h"
#include <omp.h>

#ifndef NDEBUG

#include <iostream>
#include <mat.h>

#endif //NDEBUG

// Main code
int main(int, char **) {
#ifndef NDEBUG
  testMats();
  
  std::cout << "Available threads : " << omp_get_max_threads() << "\n";

#endif //NDEBUG
  
  
  GameOfLife gui;
  
  gui.render();
  
  return 0;
}
