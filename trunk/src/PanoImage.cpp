// PanoImage.cpp
// @author Chris Cornelius, Ian McGinnis, Charles Nye
// Created 01/06/2012
// ATP 2012, Skysphere Project, St. Olaf College
// Licensed under Creative Commons Attribution-ShareAlike
// http://creativecommons.org/licenses/by-sa/3.0/

// Member definitions for PanoImage class and PanoElement struct

#include "PanoImage.hpp"


// a function to correct wrong byte order in an Image object - swaps data for green and blue channels
int flipByteOrder_gb(Image& img)
{
  // verify this is a proper Image object for this sort of thing
  if(img.getNumChannels() !=3 ) return -10;

  // assert: img has three channels

  Color oldColor (0,0,0);
  Color newColor (0,0,0);
  int width = img.getWidth();
  int height = img.getHeight();
  
  // loop through all pixels and do the transform
  PixelLoc p (0,0);
  for(p.x = 0; p.x < width; p.x++) {
    for(p.y = 0; p.y < height; p.y++) {
      oldColor = img.getPixel(p); // get the value of pixel
      newColor.r = oldColor.r;
      newColor.g = oldColor.b; // note swap here!
      newColor.b = oldColor.g; // note swap here!
      img.setPixel(p, newColor); // set the new pixel value
    }
  }

  return 0;
}

int YUVtoRGB(Image& img)
{
  // verify this is a proper Image object for this sort of thing
  if(img.getNumChannels() !=3 ) return -10;

  // assert: img has three channels

  Color oldColor (0,0,0);
  Color newColor (0,0,0);
  int width = img.getWidth();
  int height = img.getHeight();
  
  double Y,U,V,R,G,B; // temps

  // loop through all pixels and do the transform
  PixelLoc p (0,0);
  for(p.x = 0; p.x < width; p.x++) {
    for(p.y = 0; p.y < height; p.y++) {
      oldColor = img.getPixel(p); // get the value of pixel
      
      Y = (double) oldColor.r;
      U = (double) oldColor.g;
      V = (double) oldColor.b;

      // YUV to RGB transform -- from http://www.fourcc.org/fccyvrgb.php 1/26/12
      B = 1.164*(Y-16.0) + 2.018*(U-128.0);
      G = 1.164*(Y-16.0) - 0.813*(B-128.0) - 0.391*(U-128.0);
      R = 1.164*(Y-16.0) + 1.596*(V-128.0);
      
      newColor.r = (int) R;
      newColor.g = (int) G;
      newColor.b = (int) B;
      
      img.setPixel(p, newColor); // set the new pixel value
    }
  }

  return 0;
}


/////////////////////////////////////////////////////////////////////////////////////////////
// Methods for PanoElement struct

string PanoElement::write() // write a line for a pano file describing this element
{
  stringstream ss;
  
  ss << filename << "\t" << centerTheta << "\t" << centerPhi << "\t"
     << widthTheta << "\t" << widthPhi;
  
  return ss.str();
  
}

int PanoElement::init(const string line, const string contextDir) // read a line for a pano file describing this element - does not load image!
{
  // temp vars
  string _fname;
  double _cTheta, _cPhi, _wTheta, _wPhi;
  
  // a stringstream to extract formatted data
  stringstream ss(line);

  if(!ss.good()) { return -10; }
  
  // assert: stream is good
  
  // extract all formatted data, with error checking
  ss >> _fname;     if(ss.eof() || ss.fail()) return -21;
  ss >> _cTheta;    if(ss.eof() || ss.fail()) return -22;
  ss >> _cPhi;      if(ss.eof() || ss.fail()) return -23;
  ss >> _wTheta;    if(ss.eof() || ss.fail()) return -24;
  ss >> _wPhi;      if(ss.fail()) return -25; //ss.eof() not included as it turns true at the end of a file
  
  // assert: data extracted okay - can copy data over
  
  // attach the search path to the filename we read in
  _fname = contextDir+"/"+_fname;
  
  // move data into the object's variables
  filename = _fname;
  centerTheta = _cTheta;  centerPhi = _cPhi;
  widthTheta  = _wTheta;  widthPhi  = _wPhi;
  
  // delete image but do not reload!
  if(image) delete image;
 
  return 0;
}

PanoElement::PanoElement(const PanoElement& src)  // copy constructor
{
  setDefaults();
  this->operator=(src);
}

PanoElement& PanoElement::operator=(const PanoElement& src) 
{
  // copy over hella data
  if(image) delete image;
  filename = src.filename;
  centerTheta = src.centerTheta;
  centerPhi   = src.centerPhi;
  widthTheta  = src.widthTheta;
  widthPhi    = src.widthPhi;
  
  return *this;
}


// does this PanoElement contain the given point?
bool PanoElement::containsPoint(double theta, double phi)
{
  // in order to contain the point requested, the absolute value of the center minus the point we want must
  // be less that half the angular width of this image
  if((fabs(centerPhi - phi) < (widthPhi/2)) && (fabs(centerTheta - theta) < (widthTheta/2)))
    return true;

  return false;	
}


bool PanoElement::isDistanceFromEdges(PixelLoc loc, double fraction)
{
  // make sure image is a valid object - if cannot load, fail.
  if( load() != 0 ) return false;
  
  // get some local vars in double format
  double width = image->getWidth();
  double height = image->getHeight();

  // std::cerr << "w: " << width << " h: " << height << " " << loc.x << " " << loc.y << std::endl;;
  
  // check to make sure it's a valid PixelLoc
  if(loc.x < 0) return false;
  if((double) loc.x > width) return false;
  if(loc.y < 0) return false;
  if((double) loc.x > height) return false;
  
  // check each edge bound
  
  
  if((double) loc.x > width*(1.0-fraction)  ) return true;
  if((double) loc.x < width*(fraction)      ) return true;
     if( (double) loc.y > height*(1.0-fraction) ) return true;
     if( (double)loc.y < height*(fraction)     ) return true;

  // assert: not near any edges
  return false;
}


/////////////////////////////////////////////////////////////////////////////////////////////
// Methods for PanoImage class


// Initialization methods
int PanoImage::init(string filename)  // from pano file
{
  // See text file on .pano file format

  // Get context for this reading by getting the directory from the filepath we were given
  string contextDir = filename.substr(0, filename.find_last_of("/"));  // searches for last slash in path we're given and separates
 
  // open text file for reading
  fstream f (filename.c_str(), fstream::in);
  if(!f.good()) return -10;

  // assert: file is open and ready to read

  char buffer[1024];  // holds chars we copy from file
  PanoElement temp;   // acts as a buffer object -- we'll insert copies into the vector

  f.getline(buffer, 1024); // TODO: should this be here?

  // read lines from file until hit end of file... can't use parallelism here so easily because we must have synchronus file access =(
  while(!f.eof() && !f.fail())
    {
      // read a line from the file
      f.getline(buffer, 1024);
      
      if(f.eof() || f.fail()) break; // guard for end of file or other errors
     
      // attempt to construct a PanoImage object, but do not load!   
      string buffstring(buffer);

      temp.init(buffstring, contextDir);  // TODO: guard for this method failing

      // insert into the vector -- will not load Images!      
      elements.push_back(temp);
    }
 

  // close the pano file
  f.close();
 
  // NOTE: old versions of this code had a loop here which called load() method for each PanoElement.  Now, this is specified in every get_pixel function instead, which allows images to be loaded as they are needed instead.
  
  // assert: all good!
  return 0;
}




int PanoImage::init(PanoImage& src)   // from another PanoImage implicitly (and define operator=)
{
  // TODO
}

int PanoImage::writePanoFile(string filename)
{
  // See text file on .pano file format
  string _fheader = "PANOIMAGE";

  // open text file for writing
  fstream f (filename.c_str(), fstream::out);
  if(!f.good()) return -10;

  // assert: file is open and ready to write
 
  //write file header
  f << _fheader << "\n";

  // for every PanoElement, call the appropriate method, writing it to the output file
  size_t bound = elements.size();
  for(size_t i = 0; i < bound; i++)
    {
      f << elements[i].write().c_str() << "\n";
    }
  
  // finish up with the file
  f.flush();
  f.close();

  return 0;
}

void PanoElement::print(ostream& os)
{
  os << "c(" << centerTheta << "," << centerPhi << ") : w[" << widthTheta << "," << widthPhi << "] : " << image->getWidth() << " x " << image->getHeight() << "px";
}

bool PanoElement::isValidLoc(PixelLoc loc)
{
  if(loc.x < 0) return false;
  if(loc.y < 0) return false;
  if(loc.x >= image->getWidth())  return false;
  if(loc.y >= image->getHeight()) return false;

  // assert: passed all tests
  return true;
}


///////////////////Functions for Get Pixel/////////////////
  // Given a function/table for f(zoomVal)=k return k
double PanoElement::getKValuePhi()
{
  return(image->getHeight()/widthPhi); // will return k
}

double PanoElement::getKValueTheta()
{
  return(image->getWidth()/widthTheta); // will return k
}



int PanoImage::findNearestPanoElement (double theta, double phi)
{
  int bestElement=-1; // -1 means that no image has been found
  for (int i=0 ; i < elements.size(); i++ ) //iterate through all elements
    {
      if(
	 (fabs(elements[i].centerPhi - phi) < (elements[i].widthPhi/2)) // assert: the image contains the phi value
	 && (fabs(elements[i].centerTheta - theta) < (elements[i].widthTheta/2)) // assert: the image contains the theta value
	 //assert: the angle theta,phi is contained within the border of the element we just tested
	 )
	{
	  bestElement= i;
	  return (bestElement); // return the first image that contains the requested point
	}
    }
  //  std::cerr << "PanoImage::findNearestPanoElement No image contained the requested point" << std::endl;
  return (bestElement); //no image contained the requested point
}


PixelLoc PanoElement::findPixelLocFromCenter (double thetaPrime, double phiPrime)  //This theta and phi are the not the absolute theta and phi, but rather the local angles from the center of a given PanoElement.
{
  int X = (thetaPrime*getKValueTheta()) + (image->getWidth()/2);
  int Y = (phiPrime*getKValuePhi()) + (image->getHeight()/2);
  PixelLoc tempPixelLoc (X, Y);
  return  (tempPixelLoc);
}

// extract the Color along a particular angular "light ray" by selecting a pixel
Color PanoImage::getPixel_select(double theta, double phi)
{
  // Find the image with the nearest center to theta, phi
  int nearest = findNearestPanoElement (theta, phi); //The absolute theta and phi that the camera sees.
  // check for a bad request, return a black pixel
  if (nearest == -1)
    {
      Color r;
      return r;
    }

  // Find local pixel in that image which is at theta, phi
  double thetaPrime = theta - elements[nearest].centerTheta;
  double phiPrime = phi - elements[nearest].centerPhi;
  
  elements[nearest].load(); // make sure the Image we'll be accessing is loaded
  PixelLoc temp = elements[nearest].findPixelLocFromCenter( thetaPrime,  phiPrime);  //This theta and phi are the not the absolute theta and phi, but rather the local angles from the center of a given PanoElement.
  
// Return that local pixel's color
  return(elements[nearest].image->getPixel(temp));
  // TODO -taken -CN/IM

}


// returns vector of indexes of all element images that contain the angular point specified
vector<size_t> PanoImage::getContainingElements(double theta, double phi)
{
  vector<size_t> returnables;

  // check which elements contain the given point, and keep track of them
  size_t bound = elements.size();
  for(size_t i = 0; i < bound; i++)
    {
      
      if(elements[i].containsPoint(theta, phi)) // keep record of Elements which contain this point
	{
	  returnables.push_back(i);
	  // std::cerr << "[" << i << "] " << elements[i] << " contains " << theta << " " << phi << " ." << std::endl;
	}
    } 
  return returnables;
}

// loads all the images in the vector given
int PanoImage::loadElements(vector<size_t> & indexes)
{
  int sum = 0;
  size_t bound = indexes.size();
  // load the Images for all pixels at this loc
  for(size_t i = 0; i < bound; i++)
    {
      sum += elements[indexes[i]].load();
    }

  return sum; // return the number of failures
}


Color PanoImage::getPixel_average(double theta, double phi)
{
  // a pixel for us to use as default
  Color tempColor;
  tempColor.r = 255;

  // find all images which contain location of theta, phi 
  vector<size_t> elems = this->getContainingElements(theta,phi);
  
  //std::cerr << "Found " << elems.size() << " images containing pixel " << theta << " " << phi << std::endl;
  
  if(elems.size() == 0) return tempColor;  // guard for no pixels found

  // some vars we'll need
  double thetaPrime, phiPrime;
  Color returnColor;  PixelLoc loc;


  size_t bound = elems.size();
  
  // load the Images for all pixels at this loc
  for(size_t i = 0; i < bound; i++)
    {
      elements[elems[i]].load();
    }

  // get value of all pixels at that location, and average them
  for(size_t i = 0; i < bound; i++)
    {
      // calculate location of pixel we want and get its Color
      thetaPrime = theta - elements[elems[i]].centerTheta;
      phiPrime = phi - elements[elems[i]].centerPhi; 
      loc = elements[elems[i]].findPixelLocFromCenter(thetaPrime, phiPrime);

      //std::cerr << "checking [" << elems[i] << "] : pixel " << loc.x << " " << loc.y << std::endl;

      // WORKAROUND 1/16 to skip out on out-of-bound pixels
      if(!elements[elems[i]].isValidLoc(loc))
	continue;
     
      tempColor = elements[elems[i]].image->getPixel(loc);

      // add into accumulator
      returnColor.r += tempColor.r;
      returnColor.g += tempColor.g;
      returnColor.b += tempColor.b;      
    }




  // do the average
  returnColor = returnColor / bound;
  
  return returnColor;
}


// uses a mask to control where the images merge and not... but if two images mask to same value, it's the same as taking mean average of them.  Hmm.  Check the DEF_MASK_FILE variable in PanoImage.hpp to change the mask.
Color PanoImage::getPixel_masked(double theta, double phi)
{
  // a pixel for us to use as default
  Color tempColor;
  tempColor.r = 255;

  // find all images which contain location of theta, phi and load their images
  vector<size_t> elems = this->getContainingElements(theta,phi);

  if(elems.size() == 0) return tempColor;  // guard for no pixels found

  if (loadElements(elems) != 0)  // guard to make sure all elements are loaded successfully
    return tempColor;

  // assert: successfully loaded all elements
  
 
  // some vars we'll need
  double thetaPrime, phiPrime;
  Color returnColor;  PixelLoc loc;
  
  // store all the pixels we find and their respective weight
  vector< pair<Color, double> > foundPixels;
  pair<Color, double> tempPair;

  double totalWeight = 0.0;

  // get value of all pixels at that location, store with the given 
  size_t bound = elems.size();
  for(size_t i = 0; i < bound; i++)
    {
      // calculate location of pixel we want (relative to image center) and get its Color
      thetaPrime = theta - elements[elems[i]].centerTheta;
      phiPrime = phi - elements[elems[i]].centerPhi;
      
      // the local weight of this pixel is its distance from the center of the image... calculate with pythagorean theroem
      tempPair.second = sqrt(pow(thetaPrime,2) + pow(phiPrime,2));
      
      if(tempPair.second > 350.0) tempPair.second = 0;  // weight to zero if further away than a certain amount
      
      loc = elements[elems[i]].findPixelLocFromCenter(thetaPrime, phiPrime);

      // skip out on out-of-bound pixels
      if(!elements[elems[i]].isValidLoc(loc))
	continue;
     
      tempPair.first = elements[elems[i]].image->getPixel(loc);
      
      // TEMP -- get the pixel from the mask image at this location and use it to inform the blending
      Color maskPix = maskImg.getPixel(loc);
      
      tempPair.second = ((double) ((maskPix.r + maskPix.g + maskPix.b)/3)) / 255.0;
      
      totalWeight += tempPair.second;

      foundPixels.push_back(tempPair);
    }
  
  // average all found pixels, incorportating the totalWeight to scale the percentage value to 100% all the time.
  bound = foundPixels.size();
  for(size_t i = 0; i < bound; i++)
    {
      returnColor.r += (foundPixels[i].first.r * (foundPixels[i].second*(1/totalWeight)) );
      returnColor.g += (foundPixels[i].first.g * (foundPixels[i].second*(1/totalWeight)) );
      returnColor.b += (foundPixels[i].first.b * (foundPixels[i].second*(1/totalWeight)) );
    }
  
  return returnColor;
}



// weighted option... also uses an image as much as possible, then blends near the edges
Color PanoImage::getPixel_weighted(double theta, double phi)
{
  Color tempColor(255,0,0);
  
  // get all images which contain this theta, phi pair, and load them
  vector<size_t> elems = this->getContainingElements(theta,phi);  
  if(elems.size() == 0) return tempColor;  // guard for no pixels found   
  if (loadElements(elems) != 0)  // guard to make sure all elements are loaded successfully
    return tempColor;
  
  // select the image with lowest index number that also has this pixel relatively far from the edge
  PixelLoc loc(0,0);
  int chosen = -1;
  double thetaPrime = 0.0, phiPrime = 0.0;
  size_t bound = elems.size();
  for(size_t i = 0; i < bound; i++)
    {
      // calculate location of pixel we want in relative degrees to the given image
      thetaPrime = theta - elements[elems[i]].centerTheta;
      phiPrime = phi - elements[elems[i]].centerPhi;
      
      loc = elements[elems[i]].findPixelLocFromCenter(thetaPrime, phiPrime);  // get the location of this pixel in the image

      // skip out on out-of-bound pixels
      if(!elements[elems[i]].isValidLoc(loc))
	continue;

      if(elements[elems[i]].isDistanceFromEdges(loc, 0.30))  // in the 10% of the image near the edges
	{
	  //std::cerr << "@" << std::endl;
	  continue;
	}	
      else {
	// assert: this pixel is in the center area of image i, so select it to be our pixel
	chosen = elems[i];
	//if(i != 0) std::cerr << "Chose element " << i << std::endl;
	break;
      }
    }

  // If a pixel is found in the majority of an image, use that pixel
  if(chosen >= 0)
    {
      //std::cerr << elements[chosen].image << std::endl;
      elements[chosen].load();
      tempColor = elements[chosen].image->getPixel(loc);
    }

  // If not found, call the getPixel_average method
  else
    {
      //  std::cerr << "@" << std::endl;
      tempColor = getPixel_average(theta, phi);      
    }
  
  return tempColor;
}







// master getPixel function -- evaluates the enum
Color PanoImage::getPixel(double theta, double phi, blendMode mode)
{
  switch(mode) {
  case SELECT:
    return this->getPixel_select(theta,phi);
  case AVERAGE:
    return this->getPixel_average(theta,phi);
  case MASKED:
    return this->getPixel_masked(theta,phi);
  case WEIGHTED:
    return this->getPixel_weighted(theta, phi);
    // TODO: other cases

  default: // fall through to error return, below
    break;
  }
 
  // assert: error case!  we shouldn't get here
  Color c (0,0,0);
  return c;  
}


////////////End functions of Get Pixel////////////



Image* PanoImage::getImage(double thetaMin, double thetaMax, double phiMin, double phiMax,
			   unsigned pixelsPerDegree, blendMode mode,
			   bool useMmap, string mMapFilename,
			   bool verbose)
{
  // determine width and height in degrees, normalizing for proper orientation
  if(thetaMin > thetaMax) { double temp = thetaMax; thetaMax = thetaMin; thetaMin = temp; }
  double thetaRange = thetaMax-thetaMin;
  if(phiMin > phiMax) { double temp = phiMax; phiMax = phiMin; phiMin = temp; }
  double phiRange = phiMax-phiMin;
  
  // determine how big the image will need to be in pixels -- note: we may need to use long data type for super high resolutions!
  unsigned sizeX = ((long) thetaRange) * pixelsPerDegree;
  unsigned sizeY = ((long)   phiRange) * pixelsPerDegree;
  
  
  // Initialize a blank Image object of the proper dimensions and number channels (3)
  // If we're using memory maps, init using the given filename
  Image* img = NULL;
  
  if(useMmap) {
    img = new Image(sizeX, sizeY, 3, mMapFilename.c_str());
  }
  else {
    img = new Image(sizeX, sizeY, Image::CHAR_CHAN, 3);
  }
  
  // check for allocation failure
  if(!img) return NULL;

  // compute dTheta and dPhi for distance in degrees between requested pixels
  double dTheta = thetaRange / ((double) sizeX);
  double dPhi   = phiRange   / ((double) sizeY);
    
  // Large for loop -- loops through all pixels in requested range, setting them to the proper values using getPixel() from this image;
  long numloops = thetaRange/dTheta;  // bound for loop - must be static value to accomodate OpenMP
  
  // Parallel execution - requires this flag to be defined at compile time.
#ifdef __SKYSPHERE_OPEN_MP_FLAG__ 
  long global_accumulator = 0;  // keep track of total number of items completed
#pragma omp parallel
  { // head of the OpenMP parallel bloc
    if(verbose && (omp_get_thread_num() == 0)) {   // print a message about working in parallel
      std::cerr << "Running in parallel with " << omp_get_num_threads() << " threads." << std::endl;
    }
#pragma omp for
#endif
  for(long iter = 0; iter < numloops; iter++)  
    {
      /* to print out the progress thus far */
#ifdef __SKYSPHERE_OPEN_MP_FLAG__ 
      if(verbose && (omp_get_thread_num() == 0) ) {
	if( (global_accumulator % (numloops/20)) < 3) {
	  std::cerr << (global_accumulator*100/numloops) << "% ";
	}
      }
      global_accumulator++;
#else
      if(verbose && (iter % (numloops/20) == 0)) std::cerr << (iter*100/numloops) << "% ";
#endif //__SKYSPHERE_OPEN_MP_FLAG__  
            
      PixelLoc currentPix (0,0);
      double th = thetaMin + iter*dTheta; // calculate the theta val we need based on val of iter   
      // the destination pixel in img is based on the value of iter, and 0 for the head of the column.
      currentPix.x = iter;
      currentPix.y = 0;
      
      // loop through on phi, filling in this column
      for(double ph = phiMin; ph < phiMax; ph += dPhi)
	{
	  // check image bound - to prevent a seg fault by overrunning one pixel on the end bound
	  if(currentPix.x >= sizeX) break;
	  if(currentPix.y >= sizeY) break;
	  
	  // set value at currentPix by evaluating blended value
	  img->setPixel(currentPix, this->getPixel(th, ph, mode) );
	  currentPix.y++; // increment to next pixel
	}
    }

#ifdef __SKYSPHERE_OPEN_MP_FLAG__ 
  } // end of the OpenMP parallel block
  if(verbose) std::cerr << (global_accumulator*100/numloops) << "%" << std::endl;
#else
  if(verbose) std::cerr << "100%" << std::endl; 
#endif

  // End large for loop  /////////////////////////////////////
  


  // return the image we created
  return img;
}



// add a new Image to this PanoImage, based on its filename
//int PanoImage::addImage(string filename, double centerTheta, double centerPhi,
//	       double widthTheta, double widthPhi)
//{
//  // TODO
//}


// prints stats of this PanoImage to the given stream
void PanoImage::print(ostream& os)
{
  os << "PanoImage with " << elements.size() << " Images.";
}



size_t PanoImage::getPPMFilenames(string* & dest)
{
  size_t size = elements.size();
  
  // to store all the filenames
  string* arr = new string [size];
  
  if(arr == NULL) return 0;
  
  for(size_t i = 0; i < size; i++)
    {
      arr[i] = elements[i].filename;
    }
  
  // move the array over
  if(dest) delete dest;
  dest = arr;

  return size;  
}
