#include <iostream>
#include <stdint.h>
#include <math.h>
#include "fitsio.h"
#include "tiffio.h"

double arcsinh (double value) {
  return log(value + sqrt(1 + value * value));
}

std::string createFilename (const char* path) {
  std::string filename, filepath;
  filepath = std::string (path);
  size_t pos = filepath.find_last_of("/");
  
  if(pos != std::string::npos)
    filename.assign(filepath.begin() + pos + 1, filepath.end());
  else
    filename = filepath;
  return filename + ".tiff";
}


int main (int argc, const char * argv[])
{
    if (argc < 8) {
        std::cout << "stretch backgroundLevel scaledBackgroundLevel peakLevel scaledPeakLevel blackLevel whiteLevel bitdepth filepath" << std::endl;
        return 0;
    }
    
    // Capture the arguments
    const std::string stretch = argv[1];
    const float backgroundLevel = atof(argv[2]);
    const float scaledBackgroundLevel = atof(argv[3]);
    const float peakLevel = atof(argv[4]);
    const float scaledPeakLevel = atof(argv[5]);
    const float blackLevel = atof(argv[6]);
    const float whiteLevel = atof(argv[7]);
    const unsigned int bitdepth = atoi(argv[8]);
    const char* filepath = argv[9];
    
    // Define data type
    // #define BIT_DEPTH bitdepth
    // #if BIT_DEPTH <= 8
    // #define bitdepth_t uint8_t
    // #else
    // #define bitdepth_t uint16_t
    // #endif
    #define bitdepth_t uint16_t
    
    // Declare variables
    fitsfile *fptr;
    TIFF *image;
    std::string filename;
    float scaledRange, range, scaleRange;
    int i, naxis, bytesPerStrip, status = 0;
    long naxes[2], fpixel[2];
    double *original, computed;
    bitdepth_t *buffer;
    bitdepth_t bitdepthRange = pow(2.0, bitdepth) - 1;
    
    filename = createFilename(filepath);
    
    // Compute some values that are frequently used
    scaledRange = scaledPeakLevel - scaledBackgroundLevel;
    range = peakLevel - backgroundLevel;
    scaleRange = whiteLevel - blackLevel;
    
    // Open the FITS image and get dimension info.
    fits_open_image(&fptr, filepath, READONLY, &status);
    fits_get_img_dim(fptr, &naxis, &status);
    fits_get_img_size(fptr, 2, naxes, &status);
    
    // Initialize variables for writing TIFF
    if((image = TIFFOpen(filename.c_str(), "w")) == NULL){
      std::cout << "Could not open TIFF image for writing" << std::endl;
      exit(1);
    }
    // Set baseline tiff tags
    TIFFSetField(image, TIFFTAG_IMAGEWIDTH, naxes[0]);
    TIFFSetField(image, TIFFTAG_IMAGELENGTH, naxes[1]);
    TIFFSetField(image, TIFFTAG_BITSPERSAMPLE, bitdepth);
    TIFFSetField(image, TIFFTAG_SAMPLESPERPIXEL, 1);
    TIFFSetField(image, TIFFTAG_ROWSPERSTRIP, 1);
    
    TIFFSetField(image, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
    TIFFSetField(image, TIFFTAG_FILLORDER, FILLORDER_MSB2LSB);
    TIFFSetField(image, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
    TIFFSetField(image, TIFFTAG_COMPRESSION, COMPRESSION_LZW);
    TIFFSetField(image, TIFFTAG_SOFTWARE, "astroimg 0.0.1");
    
    // Allocate memory for one row of pixels for FITS and TIFF data
    bytesPerStrip = naxes[0] * sizeof(bitdepth_t);
    original = (double *) malloc(naxes[0] * sizeof(double));
    buffer = (bitdepth_t *) malloc(bytesPerStrip);
    if (original == NULL || buffer == NULL) {
      std::cout << "Memory allocation error" << std::endl;
      return(1);
    }
    
    std::cout << "Image Width:\t\t" << naxes[0] << std::endl;
    std::cout << "Image Height:\t\t" << naxes[1] << std::endl;
    std::cout << "Bit Depth:\t\t" << bitdepth << std::endl;
    std::cout << "Bytes Per Strip:\t" << bytesPerStrip << std::endl;
    
    // Read image one row at a time
    fpixel[0] = 1;
    for (fpixel[1] = 1; fpixel[1] <= naxes[1]; fpixel[1]++) {
      fits_read_pix(fptr, TDOUBLE, fpixel, naxes[0], 0, original, 0, &status);
      
      for (i = 0; i < naxes[0]; i++) {
        // Scale the pixel
        computed = scaledBackgroundLevel + scaledRange * (original[i] - backgroundLevel) / range;
        
        // Stretch the pixel using two applications of arcsinh
        computed = arcsinh(computed);
        computed = arcsinh(computed);
        
        // Scale to 16 bit integer
        computed = (computed - blackLevel) / scaleRange;
        if (computed > 1) {
          computed = 1;
        } else if (computed < 0) {
          computed = 0;
        }
        computed = bitdepthRange * computed;
        buffer[i] = floor(computed + 0.5);
      }
      
      // Write row to TIFF
      TIFFWriteEncodedStrip(image, naxes[1] - fpixel[1], buffer, bytesPerStrip);
    }
    
    // Close files and free memory
    TIFFClose(image);
    fits_close_file(fptr, &status);
    free(original);
    free(buffer);
    
    std::cout << "SHABAM!" << std::endl;
    return 0;
}