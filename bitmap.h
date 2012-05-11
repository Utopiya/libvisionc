/* 
 * File:   bitmap.h
 * Author: montero85
 *
 * Created on 27 aprile 2012, 16.59
 */

#ifndef _BITMAP_H
#define	_BITMAP_H

#include"types.h"

//Separators for the functions to save and load immages to a txt file
#define RGBSEPARATOR '.'
#define PIXELSEPARATOR ' '
#define ROWSEPARATOR '#'

typedef enum {RGB, GRAY, BW} IMGTYPE;

//The first two structures are used as an help to deal with the file

//Structure to contain the Bmp File Header (first part of a bmp file)
typedef struct {
  WORD filetp; //The file type
  DWORD filesz; //The size in bytes of the bitmap file
  DWORD creator; //reserved
  DWORD bmp_offset; //Indicates the offset among the beginning of the file and where the bitmap data are stored
} BMPFILE_HEADER;

//Structure to contain the DIP Header (second part of a bmp file)
typedef struct {
  DWORD header_sz; //size of this header
  int width; //bitmap width in pixel
  int height; //bitmap height in pixel
  WORD nplanes; //Number of color planes being used (must be 1)
  WORD bitspp; //the number of bits per pixel, which is the color depth of the image
  DWORD compress_type; //The compression method being used (0=none)
  DWORD bmp_bytesz; //the image size. This is the size of the raw bitmap data (don't confuse with the file size)
  int hres; //the horizontal resolution of the image. (pixel per meter, signed integer)
  int vres; //the vertical resolution of the image. (pixel per meter, signed integer)
  DWORD ncolors; //the number of colors in the color palette, or 0 to default to 2^n.
  DWORD nimpcolors; //the number of important colors used, or 0 when every color is important; generally ignored.
} BITMAPINFOHEADER;

//This is the actual structure used to handle the images
typedef struct{
int width; //Width in pixels of the image
int height; //Height in pixels of the image
IMGTYPE type; //Specifies whether the image is an RGB or a GRAYSCALE
unsigned char *data; //Pointer to where the data structure is stored (used to handle the actual data)
}IMG;

IMG loadBmp(char fileName[]);
int saveBmp(IMG, char []);
int bmp2File(IMG, char []); //Save a bmp img (not a .bmp file)into a txt file.
#endif	/* _BITMAP_H */

