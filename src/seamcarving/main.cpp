#include "seamcarving.h"
#include <omp.h>

#ifndef NDEBUG

#include <iostream>
//#include <mat.h>

#endif //NDEBUG

// Main code
int main(int argc, char **argv) {
#ifndef NDEBUG
  //testMats();
  
  std::cout << "Available threads : " << omp_get_max_threads() << "\n";

#endif //NDEBUG
  const char *inputFileName;
  if (argc < 2) {
    inputFileName = "milka.jpg";
  } else {
    inputFileName = argv[1];
  }
  
  SeamCarving gui(inputFileName);
  
  gui.render();
  
  return 0;
}
