/* 
 * File:   main.c
 * Author: montero85
 *
 * Created on 27 aprile 2012, 17.26
 */

#include <stdio.h>
#include <stdlib.h>
#include "bitmap.h"
/*
 * 
 */
BMPFILE_HEADER bmp_img_header;
BITMAPINFOHEADER bmp_info_header;
IMG imm;

int main(int argc, char** argv) {
    int y,x;
    char tmp;
    FILE *filePtr;
    imm=loadBmp("testR1.bmp");
    saveBmp(imm,"testSave.bmp");
    printf("START\n");
    
    return (EXIT_SUCCESS);
}

