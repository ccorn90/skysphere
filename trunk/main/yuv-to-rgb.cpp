// yuv-to-rgb.ppm
// processes PPM files, reformatting YUV to RGB
// TODO: make this well documented

#include <iostream>
#include <fstream>
#include <string.h>
#include "eriolHeader.h"
#include "PanoImage.hpp"

// for multithreading support
#ifdef __SKYSPHERE_OPEN_MP_FLAG__
#include <omp.h>
#endif // __SKYSPHERE_OPEN_MP_FLAG__

using namespace std;

void help(); // display usage info

// global vars and configuration vars
const static string DEFAULT_INPUT_FILE = "";
const static string DEFAULT_OUTPUT_DIR = "";
static string inputFile = DEFAULT_INPUT_FILE;
static string outputDir = DEFAULT_OUTPUT_DIR;
static bool panoMode = false;
static string path = "./";
// the images we'll process
static string* images = NULL;
static int bound = 0;


int flipByteOrder_gb(Image& img);

int main(int argc, char* argv[])
{

  // parse arguments
  if(argc > 1) {
    if(strcmp(argv[1], "--pano") == 0) {
      panoMode = true;
      if(argc > 2) {  // check for a second argument -- if so, it's the name of the pano file we want
	string t (argv[2]);
	inputFile = t;
      }	
    }
    else if((strcmp(argv[1], "--files") == 0) || (strcmp(argv[1],"-f") == 0))
      {
	// loop through args until we run out
	int argcount = 2;
	images = new string[argc-2];
	bound = argc - 2;
	while(argcount < argc)
	  {
	    string t(argv[argcount]);
	    images[argcount-2] = t;
	    argcount++ ;
	  }
      }
    else if(strcmp(argv[1], "--help") == 0)
      {
	help();
	return 0;
      }
    else { /* continue on */ }
  }

  cerr << "yuv-to-rgb" << endl;

  if( panoMode )
    {
      if(inputFile == DEFAULT_INPUT_FILE)  cerr << "Specify a source .pano file: ";  cin >> inputFile;      // load pano file
      PanoImage pano;
      cerr << "Loading pano file " << inputFile << endl;
      if( pano.init(inputFile) != 0) {
	cerr << "Error occured while loading pano file." << endl;
	return -100;
      }

      bound = pano.getPPMFilenames(images);
    }

 
  cerr << "Flipping B and G bytes on " << bound << " images..." << endl;
  
  // for each image, load it, swap it, save it to the output directory, and close it

// for multithreading
#ifdef __SKYSPHERE_OPEN_MP_FLAG__ 
  int global_accumulator = 0;
#pragma omp parallel
  {
    if(omp_get_thread_num() == 0)
      cerr << "Running in parallel with " << omp_get_num_threads() << " threads." << endl;
#pragma omp for
#endif // __SKYSPHERE_OPEN_MP_FLAG__
  for(size_t i = 0; i < bound; i++)
    {
      // print progress indicator in percent
#ifdef __SKYSPHERE_OPEN_MP_FLAG__
      if ((omp_get_thread_num() == 0))
	{ 
	  //	  if(global_accumulator % (bound/15) == 0)
	  //	  if(global_accumulator % (0) == 0)
	  //cerr << ((global_accumulator*100) / bound) << "% ";
	}
      global_accumulator++;
#else
      if( i % (bound/10) == 0)
	cerr << (i*100) / bound << "% ";
#endif // __SKYSPHERE_OPEN_MP_FLAG__

      fstream f(images[i].c_str(), fstream::in);
      if( ! f.good() ) {
      	cerr << "Unable to open file " << images[i] << endl;
      	continue;
       }
      Image* img = new Image(images[i].c_str());   
      if(img == NULL) {
	cerr << "Unable to open file " << images[i] << endl;
	continue;
      }
      
      if(YUVtoRGB(*img) != 0) {
	cerr << "Error exchanging YUV for RGB for image " << images[i] << endl;
      }
      else {
	// print the image out
	string path = outputDir + images[i];
	img->print(path.c_str());
      }

      // clean up this iteration
      if(img) delete img;
      
    }
#ifdef __SKYSPHERE_OPEN_MP_FLAG__
  } // end of omp parallel block
#endif //__SKYSPHERE_OPEN_MP_FLAG__

  cerr << "100%" << endl;
  cerr << "Done." << endl;
  
}


void help() {
  cerr << "" << endl;
  cerr << "The Skysphere Project, St. Olaf College." << endl;
  cerr << "yuv-to-rgb: converts PPM files formatted with YUV into PPM files formatted correctly as RGB" << endl;
  cerr << "usage: yuv-to-rgb [-p path] [--pano panofile]" << endl;
  cerr << "" << endl;  
}


