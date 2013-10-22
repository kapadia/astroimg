AstroImg
--------

Processing large FITS images for EPO purposes can be memory intensive.  When the original FITS image is of a large resolution, software programs such as [FITS Liberator](http://www.spacetelescope.org/projects/fits_liberator/) run slow due to placing a large subset of the image in memory.  This program is a command line utility that can work in conjunction with FITS Liberator to apply a scale and stretch on large FITS images and convert the image to a TIFF for processing in programs such as Photoshop, GIMP or ImageMagick.

Installation
============
This utility depends on two libraries:

* [`cfitsio`](http://heasarc.gsfc.nasa.gov/fitsio/)
* [`libtiff`](http://www.libtiff.org/)

Installation of `cfitsio` is explained at [http://heasarc.gsfc.nasa.gov/docs/software/fitsio/quick/node3.html](http://heasarc.gsfc.nasa.gov/docs/software/fitsio/quick/node3.html)

Installation of `libtiff` may be more complicated.  If on OS X there is an issue resolving a preprocessor variable regarding OpenGL.  Here's the solution:

Edit the file `tools/tiffgt.c` by changing

    #if HAVE_APPLE_OPENGL_FRAMEWORK
    # include <OpenGL/gl.h>
    # include <GLUT/glut.h>
    #else
    # include <GL/gl.h>
    # include <GL/glut.h>
    #endif
    
to

    # include <OpenGL/gl.h>
    # include <GLUT/glut.h>

Run `./configure`, `make`, `make install`.

Now compile `astroimg` using the `Makefile`.  Run `make`.

Usage
=====

Processing E/PO images with FITS Liberator means manually tweaking scale and stretch parameters to provide a subjective interpretation of an image's high dynamic range.

Processing large files can be difficult since FITS Liberator attempts to store a large portion of the image in memory.  One solution is to use a software such as [Montage](http://montage.ipac.caltech.edu/) to create color images, however, Montage does not provide all the parameters that FITS Liberator offers.  Another solution is to use Montage, FITS Liberator and this application in conjunction.

* Downsample the FITS image using Montage's mShrink utility.
* Import the downsampled image into FITS Liberator.
* Record the scale and stretch parameters (background level, scaled background level, peak level, scaled peak level, black level, white level)
* Use `astroimg` to apply the scale and stretch on the original resolution image.

Color composites can be done using Photoshop, GIMP or ImageMagick.  If the resolution of the images are extremely large, it might be worthwhile to downsample the TIFFs, apply the color in Photoshop, then use ImageMagick to create the full resolution composite using the colors found in Photoshop.

    ./astroimg [stretch] [background level] [peak level] [scaled peak level] [black level] [white level] filepath

## Note

Only two stretches are supported at the moment `arcsinh` and double arcsinh (`arcsinh2`).

Example
=======

On the todo list.
  