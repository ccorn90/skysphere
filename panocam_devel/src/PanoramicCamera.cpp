// PanoramicCamera.cpp
// @author Chris Cornelius, Ian McGinnis, Charles Nye
// Created 01/06/2012
// ATP 2012, Skysphere Project, St. Olaf College
// Licensed under Creative Commons Attribution-ShareAlike
// http://creativecommons.org/licenses/by-sa/3.0/

// Member definitions for PanoramicCamera class.

#include "PanoramicCamera.hpp"

bool PanoramicCamera::init() // Opens required connections and preps for captureing
{
  //ic.openConnection(); //TODO: reinstate once ImageCapture can open and close around grabbing frames
  cc.openSerial();
}

bool PanoramicCamera::close() // Closes all connections and preps for destruction
{
  //ic.closeConnection(); //TODO: reinstate once ImageCapture can open and close around grabbing frames
  if(cc.isopen()) cc.closeSerial();
}

bool PanoramicCamera::good()  // Returns true if the camera is ready to take images
{
  if(!cc.isopen()) return false;
  //  if(!ic.good())   return false;  // TODO: implement in ImageCapture
  
  return true;
}



// Captures a panoramic image spanning the given section of the sky, and
// saves the ppm files into the given directory
int PanoramicCamera::grabPano(string destinationDirectory, double ThetaMin, double ThetaMax,
			      double PhiMin, double PhiMax, double overlap, int zoomRatio, int picsPerLocation,
			      bool verbose, string panofilename)
{
  // TODO: build an empty PanoImage object
  // loop through all center points of images we're taking, take an image at each, storing to disk with proper name, etc
  
  if(ThetaMax < ThetaMin)   // check for inverted range and flip if so
    {
      double temp = ThetaMax;
      ThetaMax = ThetaMin;
      ThetaMin = temp;
    }
  double ThetaRange = ThetaMax - ThetaMin;
  if(PhiMax < PhiMin)   // check for inverted range and flip if so
    {
      double temp = PhiMax;
      PhiMax = PhiMin;
      PhiMin = temp;
    }
  double PhiRange = PhiMax - PhiMin;
  
  
  // how many images do we need to take in the pan range, and what is the spacing between them?
  // compute by dividing the range we're scanning by the field of view of the camera, with the overlap accounted for
  
  unsigned imagesPan  = 1 + (int) (ThetaRange / (libviscaBRC300::zoom_ratio_to_degreesTheta(zoomRatio)*(1-overlap)) );
  unsigned imagesTilt = 1 + (int) (PhiRange / (libviscaBRC300::zoom_ratio_to_degreesPhi(zoomRatio)*(1-overlap)) );
  double resolutionPan  = ThetaRange / ((double) imagesPan); 
  double resolutionTilt = PhiRange   / ((double) imagesTilt);
  
  // TODO: remove these printouts
  if(verbose) std::cerr << "Taking pano to directory " << destinationDirectory << std::endl;
  if(verbose) std::cerr << "Pan  = " << ThetaMin << " to " << ThetaMax << " => " << ThetaRange << " degrees for " << imagesPan << " captures spaced at " << resolutionPan << " degrees"<< std::endl;
  if(verbose) std::cerr << "Tilt = " << PhiMin << " to " << PhiMax << " => " << PhiRange << " degrees for " << imagesTilt << " captures spaced at " << resolutionTilt << " degrees" <<std::endl;
  
  // WORKAROUND - loops seem to run forever if resolution is set to zero... we want to jump over the bound so only one image is taken
  if(resolutionPan  == 0.0) resolutionPan  = 0.1;
  if(resolutionTilt == 0.0) resolutionTilt = 0.1;
  
  // set up zoom
  cc.setZoom(zoomRatio);
  if(verbose) std::cerr << "Set zoom: " << zoomRatio << std::endl;
  
  // keep track of how many frames we've written
  unsigned framecounter = 0;

  // stream to keep track of what we'll write to the file
  std::stringstream filebuff;
  filebuff << "PANOIMAGE\n"; // set up file header
  
  // a big for loop to take all of the images
  // note: it was interesting to swap theta and phi order for the loops and see which took longer to scan
  for(double tlt = PhiMin; tlt <= PhiMax; tlt+= resolutionTilt)
  {
      for(double pan = ThetaMin; pan <= ThetaMax; pan += resolutionPan)      
	{
	  if(verbose) std::cerr << "\tCamera seeking..." << std::endl;
	  if (cc.setPanTilt(pan,tlt))
	    if(verbose) std::cerr << "\tSet pan and tilt: " << pan << " " << tlt << std::endl;
	  else
	    if(verbose) std::cerr << "\t Error setting pan and tilt!" << std::endl;
	  
	  
	  // Take multiple pictures at this location... name all sequentially
	  for(int framenum = 0; framenum < picsPerLocation; framenum++)
	    {
	      if(verbose) std::cerr << "\tCapturing image at " << pan << " " << tlt << " : ";  std::cerr.flush();
	      
	      // set up the filename for this object -- use camera name, then theta and phi (truncated)
	      char fname [256];
	      sprintf(fname,"%s_%04u_%03.0f_%03.0f.ppm",_name.c_str(),framecounter,pan,tlt);
	      
	      stringstream fnamestream;  fnamestream << destinationDirectory << "/" << fname;
	      
	      // capture image
	      ImageCapture* IC = new ImageCapture(ic.device(),2);  // build a new ImageCapture object
	      IC->openConnection();
	      if (IC->grabImageToPPM(fnamestream.str().c_str()) != 0) std::cerr << "Error!" << std::endl;
	      IC->closeConnection();
	      
	      if(verbose) std::cerr << fnamestream.str() << std::endl;
	  
	      // insert a line into the file buffer
	      //////////////////////////////////////////////////////////////////////
	      // WORKAROUND Chris Cornelius 1/13/12 ////////////////////////////////
	      // Cloned in PanoElement::write() method under a different name to write info about an image file to a pano file.
	      // TODO: Remove and replace with construction of an actual PanoImage in PanoramicCamera::grabPano
	      filebuff << fname << "\t" << pan << "\t" << tlt << "\t"  // filename, pan and tilt pos 
		       << libviscaBRC300::zoom_ratio_to_degreesTheta(zoomRatio) << "\t"  // theta width
		       << libviscaBRC300::zoom_ratio_to_degreesPhi(zoomRatio)   << "\t"  // phi width
		   << "\n";
	      
	      // increase to the next file number
	      framecounter++;	
	    } // end picsPerLocation loop
	} // end theta loop
  } // end phi loop


  if(verbose) {
    std::cerr << "Done!" << std::endl;
    std::cerr << "----- .pano file ----------------------------------" << std::endl;
    std::cerr << filebuff.str();
    std::cerr << "---------------- ----------------------------------" << std::endl;
  }
  
  // make the .pano file in the directory specified
  stringstream panofilenamestream; // construct the name of our output file
  panofilenamestream << destinationDirectory << "/" << panofilename << ".pano";
  
  // print to the output file
  std::fstream panofile;
  panofile.open(panofilenamestream.str().c_str(), fstream::out);
  panofile << filebuff.str();  // put the contents of the file
  panofile.close();
  
  if(verbose) std::cerr << "Wrote pano file " << panofilenamestream.str() << std::endl;
  
  // TODO: return a PanoImage object instead and use the PanoImage write() method to write the file
  
  return 0;
}












// Captures one frame from the camera and saves it to disk
int PanoramicCamera::grabImageToPPM(string filename, double theta, double phi, int zoomRatio)
{
  // check to see if this device is good
  if( !good() ) return 1;
  
  // move camera to proper parameters
  if( !cc.setPanTilt(theta,phi) )
    return -21;
  
  if( !cc.setZoom(zoomRatio) )
    return -22;

  // capture image with proper filename
  //if( ic.grabImageToPPM(filename.c_str()) != 0) //TODO: reinstate
  //  return -10; //TODO: reinstate

  
  // workaround to try to capture an image, any image //

  ImageCapture* IC = new ImageCapture(ic.device(),2);  // build a new ImageCapture object
  IC->openConnection();
  if (IC->grabImageToPPM(filename.c_str()) != 0) return -50;
  IC->closeConnection();
  
  // end workaround (see lines commented out above) //

  delete IC;
  
  // assert: all okay
  return 0;
}


// Captures one frame from the camera and saves it to disk
int PanoramicCamera::grabImageToPPM(string filename)
{
  // check to see if this device is good
  if( !good() ) return 1;

  // capture image with proper filename
  //if( ic.grabImageToPPM(filename.c_str()) != 0) //TODO: reinstate
  //  return -10; //TODO: reinstate

  
  // workaround to try to capture an image, any image //

  ImageCapture* IC = new ImageCapture(ic.device(),2);  // build a new ImageCapture object
  IC->openConnection();
  if (IC->grabImageToPPM(filename.c_str()) != 0) return -50;
  IC->closeConnection();
  
  // end workaround (see lines commented out above) //

  delete IC;
  
  // assert: all okay
  return 0;
}
