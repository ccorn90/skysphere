// PanoImage.cpp
// @author Chris Cornelius, Ian McGinnis, Charles Nye
// Created 01/06/2012
// ATP 2012, Skysphere Project, St. Olaf College
// Licensed under Creative Commons Attribution-ShareAlike
// http://creativecommons.org/licenses/by-sa/3.0/

// Member definitions for PanoImage class and PanoElement struct

#include "PanoImage.hpp"

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

  f.getline(buffer, 1024);

  // read lines from file until hit end of file... can't use parallelism here so easily because we must have synchronus file access =(
  while(!f.eof() && !f.fail())
    {
      // read a line from the file
      f.getline(buffer, 1024);
      
      if(f.eof() || f.fail()) break; // guard for end of file or other errors
     
      // attempt to construct a PanoImage object, but do not load!   
      string buffstring(buffer);

      temp.init(buffstring, contextDir); 

      // insert into the vector -- will not load Images!      
      elements.push_back(temp);
    }
 

  // close the pano file
  f.close();
  
  // load Image file for all PanoElements in the vector - run in parallel with OpenMP
  size_t bound = elements.size(), i = 0;
#ifdef __SKYSPHERE_OPEN_MP_FLAG__ 
#pragma omp parallel for private(i)
#endif
  for(i = 0; i < bound; i++)
    {
      //if( (i % (bound/10)) == 0) std::cerr << (bound*100/i) << "% loaded." << std::endl; 
      elements[i].load();
    }
  
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



  /*vector<int>::iterator it;
  for (it=elements.begin() ; it < elements.end(); it++ ) //iterate through all elements
    {
      if(
	 (*it.centerPhi - phi) < (*it.widthPhi/2) // assert: the image contains the phi value
	 && (*it.centerTheta - theta) <(*it.widthTheta/2); // assert: the image contains the theta value
	 //assert: the angle theta,phi is contained within the border of the element we just tested
	 )
	{
	  bestElement= *iti;
	  return (bestElement) // return the first image that contains the requested point
	}
    }
  return (bestElement); //no image contained the requested point
  }
  */
  /* //Take Closest Image Function
   containingElements.push_back *it; //assert: containingElements holds all good canidate elements
   //use pythagorean theorem to find element with closest center
   temp = sqrt (pow ((*it.centerPhi - phi), 2) + (pow ((*it.centerTheta - theta), 2)) //pythagorean theorem
   containingElementsDistance.push_back temp; //assert containingElementsDistance holds all pathygoren distances
 */

PixelLoc PanoElement::findPixelLocFromCenter (double thetaPrime, double phiPrime, double zoom)  //This theta and phi are the not the absolute theta and phi, but rather the local angles from the center of a given PanoElement.
{
  int X = -1; //temporay init
  int Y = -1; //temporay init
  X = (thetaPrime*getKValueTheta()) + (image->getWidth()/2);
  Y = (phiPrime*getKValuePhi()) + (image->getHeight()/2);
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
  double zoom = 1; //TODO zoom must be added to inputs
  PixelLoc temp = elements[nearest].findPixelLocFromCenter( thetaPrime,  phiPrime, zoom);  //This theta and phi are the not the absolute theta and phi, but rather the local angles from the center of a given PanoElement.
  
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

  // get value of all pixels at that location, and average them
  size_t bound = elems.size();
  for(size_t i = 0; i < bound; i++)
    {
      // calculate location of pixel we want and get its Color
      thetaPrime = theta - elements[elems[i]].centerTheta;
      phiPrime = phi - elements[elems[i]].centerPhi; 
      loc = elements[elems[i]].findPixelLocFromCenter(thetaPrime,  phiPrime, 0);

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


// gets a pixel using a weighted average... weighting source pixels heavier if they are closer to the center of the image
Color PanoImage::getPixel_weighted(double theta, double phi)
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
      
      loc = elements[elems[i]].findPixelLocFromCenter(thetaPrime,  phiPrime, 0);

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




// master getPixel function -- evaluates the enum
Color PanoImage::getPixel(double theta, double phi, blendMode mode)
{
  switch(mode) {
  case SELECT:
    return this->getPixel_select(theta,phi);
  case AVERAGE:
    return this->getPixel_average(theta,phi);
  case WEIGHTED:
    return this->getPixel_weighted(theta,phi);
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
  //long iter = 0; // openMP only uses int var types, so must be an int
  
  // Parallel execution - requires this flag to be defined at compile time.
#ifdef __SKYSPHERE_OPEN_MP_FLAG__ 
  std::cerr << "OpenMP flag on." << std::endl;
  bool ompflag_local = true;
#pragma omp parallel for
#endif
  for(long iter = 0; iter < numloops; iter++)  
    {
      if(verbose) { if( (iter % (numloops/10)) == 0) std::cerr << (iter*100/numloops) << "% "; }
      
#ifdef __SKYSPHERE_OPEN_MP_FLAG__ 
      if(verbose && ompflag_local) { std::cerr << omp_get_num_threads() << " threads." << std::endl;}
#endif
      
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
  // End large for loop  /////////////////////////////////////

  if(verbose) std::cerr << std::endl;

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
