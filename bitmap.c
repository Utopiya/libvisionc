#include"bitmap.h"
#include<stdio.h>

/*
 * Load a bitmap file into a img structure. Both non-palettized RGBs and GRAYSCALE are supported.
 * Pixel are stored in the IMG structure from top to bottom, left to right in the RGB order.
 * (This doesn't happen in bitmap files)
 * Alterations in the colors of grayscale immages may be experienced if the palette is non-standard.
 * TODO: introduce support to BW 1 bit depth images.
 * PARAM:
 * fileName: a string containing the name of the bmp file
 * return: BMPIMG structure
 */
IMG loadBmp(char fileName[])
{FILE *filePtr;
int tmp;
unsigned int x,y;
BMPFILE_HEADER imgHeader; //Structure to contain the Bitmap File Header
BITMAPINFOHEADER imgBmpHeader; //Structure to contain the Bitmap Information Header
IMG img; //The image itself
unsigned char *tmpImg;
unsigned int imgDataPtr, tmpImgPtr;
char bytesPerPixel;
char padding=0;
unsigned int scanlinebytes=0, paddedscanlbytes=0;


img.data=NULL;
//open filename in read binary mode
filePtr = fopen(fileName,"rb");
if (filePtr == NULL) return img; //In case of errors opening the file

//Read the Bitmap File Header.
//To avoid the use of #pragma pack(..) the structure must be read field by field
fread(&tmp,2,1,filePtr);
imgHeader.filetp=tmp;
fread(&tmp,4,1,filePtr);
imgHeader.filesz=tmp;
fread(&tmp,4,1,filePtr);
imgHeader.creator=tmp;
fread(&tmp,4,1,filePtr);
imgHeader.bmp_offset=tmp;

//verify that this is a bmp file by check bitmap id
if (imgHeader.filetp !=0x4D42)
{   
     fclose(filePtr);
    return img;
}

//read the bitmap info header
fread(&imgBmpHeader, sizeof(BITMAPINFOHEADER),1,filePtr);
//Store relevant infos into the IMG struct
img.height = imgBmpHeader.height;
img.width = imgBmpHeader.width;
img.type = (imgBmpHeader.bitspp == 8) ? GRAY : RGB; //Image type depends on the bits per pixel field
//Go to the beginning of the bmp data structure
fseek(filePtr, imgHeader.bmp_offset, SEEK_SET);

//allocate enough memory for the bitmap image data
tmpImg = (unsigned char*)malloc(imgBmpHeader.bmp_bytesz);

//verify memory allocation
if (!(tmpImg))
{
free(tmpImg);
fclose(filePtr);
return img;
}

//Read the image
fread(tmpImg, imgBmpHeader.bmp_bytesz,1, filePtr);


//make sure bitmap image data was read
if (tmpImg == NULL)
{
fclose(filePtr);
return img;
}

fclose(filePtr);
bytesPerPixel=imgBmpHeader.bitspp/8;
scanlinebytes=imgBmpHeader.width*bytesPerPixel;
//Calculates the number of bytes of the padding (refer to http://en.wikipedia.org/wiki/BMP_file_format#File_structure)
while ( ( scanlinebytes + padding ) % 4 != 0 )
		padding++;
//because of the padding the length of the row could not be the same as the product of the width and the pixel depth
paddedscanlbytes=scanlinebytes+padding;
//Reserve memory to store the image
img.data = (unsigned char*)malloc(imgBmpHeader.width*imgBmpHeader.height*bytesPerPixel);

//The actual image data are stored
for ( y = 0; y < imgBmpHeader.height; y++ )
		for ( x = 0; x < bytesPerPixel * imgBmpHeader.width; x+=bytesPerPixel )
		{//Two pointers are needed because pixel are stored from bottom to top, left to right, inside
                 // a bitmap file, while we want them to be stored from top to bottom inside the IMM structure.
			imgDataPtr = y * bytesPerPixel * imgBmpHeader.width + x;
			tmpImgPtr = ( imgBmpHeader.height - y - 1 ) * paddedscanlbytes + x;
                        
                        if(bytesPerPixel==1) //GRAYSCALE
                            *(img.data+imgDataPtr)=*(tmpImg+tmpImgPtr);
                        else //RGB
                            {// GBR order have to be exchanged with RGB one.
                             *(img.data+imgDataPtr)=*(tmpImg+tmpImgPtr+2);
                             *(img.data+imgDataPtr+1)=*(tmpImg+tmpImgPtr+1);
                             *(img.data+imgDataPtr+2)=*(tmpImg+tmpImgPtr);
                            }
		}


return img;
}
/*
 * Stores a imm structure into a txt file. Returns 0 if errors occurred.
 * FILE STRUCTURE:
 Height Width ColorDepth
 RED(1,1).GREEN(1,1).BLUE(1,1) RED(1,2).GREEN(1,2).BLUE(1,2)- .....#
 RED(2,1).GREEN(2,1).BLUE(2,1) RED(2,2).GREEN(2,2).BlUE(2,2) ....#
 ................................................................#
 */
int bmp2File(IMG imm, char fileName[]){
    FILE *filePtr;
    int x,y;
    int colorDepth;

//Set the color Depth
    if(imm.type==GRAY) colorDepth=8;
    else if (imm.type==RGB) colorDepth=24;
    else return 0;
//Open the file in binary write mode
    filePtr=fopen(fileName,"w");
    //The first file line contains infos
    fprintf(filePtr,"%d %d %d\n", imm.height, imm.width, colorDepth);

    //Images are stored depending on their type (that is their color depth)
    switch(colorDepth){
        case 8: //GRAYSCALE
                for(y=0; y<imm.height; y++)
                    {for(x=0; x<imm.width; x++)
                        fprintf(filePtr,"%d%c",*(imm.data+(y*imm.width+x)),PIXELSEPARATOR);
                     fprintf(filePtr,"%c",ROWSEPARATOR);}
                 break;
        case 24: //RGB
                for(y=0; y<imm.height; y++)
                    {for(x=0; x<3*imm.width;x+=3)
                        fprintf(filePtr,"%d%c%d%c%d%c",*(imm.data+(y*imm.width*3+x)),RGBSEPARATOR,
                        *(imm.data+(y*imm.width*3+x+1)),RGBSEPARATOR,
                        *(imm.data+(y*imm.width*3+x+2)),PIXELSEPARATOR);
                     fprintf(filePtr,"%c",ROWSEPARATOR);
                     }
                break;
        default: return 0;
    }
    fclose(filePtr);
    return -1;
}

/*
 * Saves an image structure as a .bmp file. If the image type is RGB, no palette is added.
 * If it's grayscale a standard palette is created. This might create problems with images taken
 * from non-standard palettized bitmaps that are then saved again.
 * PARAMETERS: structure containing the image data and the desired name for the output .bmp file
 * RETURNS: 0 in case of errors
 */
int saveBmp(IMG imm, char fileName[]){
    FILE *filePtr;
    BMPFILE_HEADER imgHeader; //Structure to contain the Bitmap File Header
    BITMAPINFOHEADER imgBmpHeader; //Structure to contain the Bitmap Information Header
    unsigned char *imgTmp;
    unsigned char GPalette[1024];
    int immPtr,imgTmpPtr;
    int x,y;
    int padding=0,scanlinebytes,paddedscanlbytes,paletteSize;
    char BytesPerPixel;
    short shortTmp;
    unsigned int uintTmp;
    filePtr=fopen(fileName,"wb");

    switch(imm.type){
        case RGB:   BytesPerPixel=3;
                    paletteSize=0;
                    break;
        case GRAY:
                    BytesPerPixel=1;
                    paletteSize=1024;
                    break;
        default:
                    return 0;
    }
    scanlinebytes = imm.width * BytesPerPixel;
	while ( ( scanlinebytes + padding ) % 4 != 0 )
		padding++;
    paddedscanlbytes = scanlinebytes + padding;

    imgTmp=(unsigned char*)malloc(imm.height*paddedscanlbytes);

    
    immPtr=0; imgTmpPtr=0;

    for (  y = 0; y < imm.height; y++ )
		for ( x = 0; x <  BytesPerPixel*imm.width; x+=BytesPerPixel)
		{
			immPtr = y * BytesPerPixel*imm.width + x;     // position in original buffer
			imgTmpPtr = ( imm.height - y - 1 ) * paddedscanlbytes + x; // position in padded buffer

                        if(BytesPerPixel==1) 
                            *(imgTmp+imgTmpPtr)=*(imm.data+immPtr);
                        else{
                            *(imgTmp+imgTmpPtr)=*(imm.data+immPtr+2);
                            *(imgTmp+imgTmpPtr+1)=*(imm.data+immPtr+1);
                            *(imgTmp+imgTmpPtr+2)=*(imm.data+immPtr);
                        }

		}

    imgHeader.filetp=0x4D42;
    imgHeader.filesz= sizeof(BMPFILE_HEADER) + sizeof(BITMAPINFOHEADER) +paletteSize+ paddedscanlbytes*imm.height;
    imgHeader.creator=0;
    imgHeader.bmp_offset=0x36+paletteSize;
    

    imgBmpHeader.header_sz=sizeof(BITMAPINFOHEADER);
    imgBmpHeader.width=imm.width;
    imgBmpHeader.height=imm.height;
    imgBmpHeader.nplanes=1;
    imgBmpHeader.bitspp=BytesPerPixel*8;
    imgBmpHeader.compress_type=0;
    imgBmpHeader.bmp_bytesz=paddedscanlbytes*imm.height;
    imgBmpHeader.vres=0x0ec4;
    imgBmpHeader.hres=0x0ec4;
    imgBmpHeader.ncolors=0;
    imgBmpHeader.nimpcolors=0;
    
   //fwrite(&imgHeader,sizeof(imgHeader),1,filePtr);
    shortTmp=imgHeader.filetp;
    fwrite(&shortTmp, sizeof(short),1,filePtr);
    uintTmp=imgHeader.filesz;
    fwrite(&uintTmp, sizeof(unsigned int),1,filePtr);
    uintTmp=imgHeader.creator;
    fwrite(&uintTmp, sizeof(unsigned int),1,filePtr);
    uintTmp=imgHeader.bmp_offset;
    fwrite(&uintTmp, sizeof(unsigned int),1,filePtr);
    fwrite(&imgBmpHeader,sizeof(imgBmpHeader),1,filePtr);
    
    switch(imm.type){
        case RGB: break;
        case GRAY:  createGrayPalette(GPalette);
                    fwrite(GPalette,paletteSize,1,filePtr);
                    break;
    }
    fwrite(imgTmp, paddedscanlbytes*imm.height,1,filePtr);

    
    fclose(filePtr);
    return -1; 
}

void createGrayPalette(char *palette){
    unsigned int i;
    for ( i = 0; i < 256; i++)
    {
        palette[i * 4 + 0] =(char) i; //blue
        palette[i * 4 + 1] = (char)i; //green
        palette[i * 4 + 2] = (char)i; //red
        palette[i * 4 + 3] =  (char)0; //padding
    }
}

/*TODO:
 * 1) RGB2Gray function
 * 2) Insert other image types like BW (1 bits black and white mask) and MASK with its own palette
 * 3) Add error handling to saveBmp
 * 4) Split functions and structures non strictly related to bitmaps on other files.
 *