/*
compile command:
cc -o slice slice.c `pkg-config --cflags --libs MagickWand`

run command:
./slice descriptor_id x_dim y_xim
*/

#include <stdio.h>
#include <stdlib.h>
#include <wand/MagickWand.h>
#include <string.h>


#define PPU 128
#define tanAngle 0.46868965165

void cropCenterTile (char *descriptorId, MagickWand *magick_wand, double center,
                double image_width, double image_height);
void cropXTiles (char *descriptorId, MagickWand *magick_wand, double center,
                double image_width, double image_height);
void cropYTiles (char *descriptorId, MagickWand *magick_wand, double center,
                double image_width, double image_height);

void writeImage (char *descriptorId, MagickWand *crop_wand, int x, int y);

double findCenter (double image_width, int x, int y);

int main(int argc,char **argv)
{
#define ThrowWandException(wand) \
{ \
  char \
    *description; \
 \
  ExceptionType \
    severity; \
 \
  description=MagickGetException(wand,&severity); \
  (void) fprintf(stderr,"%s %s %lu %s\n",GetMagickModule(),description); \
  description=(char *) MagickRelinquishMemory(description); \
  exit(-1); \
}

  MagickBooleanType
    status;

  MagickWand
    *magick_wand;

  char descriptorId[80];
  char inputFilename[84];

  if (argc != 4)
    {
      (void) fprintf(stdout,"Usage: %s descriptor_id x_dim y_dim\n",argv[0]);
      exit(0);
    }
  
  int x = atoi(argv[2]);
  int y = atoi(argv[3]);

  strcpy(descriptorId, argv[1]);
  strcpy(inputFilename, descriptorId);
  strcat(inputFilename, ".png");

  printf("Input File: %s\n", inputFilename);

  /*
    Read an image.
  */
  MagickWandGenesis();
  magick_wand=NewMagickWand();
  status=MagickReadImage(magick_wand,inputFilename);
  if (status == MagickFalse)
    ThrowWandException(magick_wand);

  double image_width = MagickGetImageWidth(magick_wand);;
  double image_height = MagickGetImageHeight(magick_wand);

  printf("Input File Dimensions: (width=%f height=%f)\n", image_width, image_height);

  double center = findCenter (image_width, x, y);

  cropCenterTile(descriptorId, magick_wand, center, image_width, image_height);
  cropXTiles(descriptorId, magick_wand, center, image_width, image_height);
  cropYTiles(descriptorId, magick_wand, center, image_width, image_height);

  magick_wand=DestroyMagickWand(magick_wand);
  MagickWandTerminus();
  return(0);
}

double findCenter (double image_width, int x, int y) {
  return y * (PPU/2);
}

  /*
    Crop/write the image then destroy it.
  */
void cropCenterTile (char *descriptorId, MagickWand *magick_wand, double center,
                double image_width, double image_height)
{
  MagickWand 
    *crop_wand;

  double left = center - PPU/2;
  crop_wand = MagickGetImageRegion(magick_wand, PPU, image_height, left, 0);
  writeImage (descriptorId, crop_wand, 0, 0);
}

void cropXTiles (char *descriptorId, MagickWand *magick_wand, double center,
                double image_width, double image_height)

{
  int x = 1;
  int y = 0;
  double bottomOffset;
  double left;

  int i;
  int incrementWidth = PPU/2;
  for (i = (center + incrementWidth); i < image_width; i += incrementWidth) {
    bottomOffset = (x * incrementWidth) * tanAngle - 1;
    left = i - 2;

    MagickWand 
      *crop_wand;

    double height = image_height - bottomOffset;
    crop_wand = MagickGetImageRegion(magick_wand, incrementWidth, height, left, 0);
    PixelWand *pixel = NewPixelWand();
    PixelSetAlpha(pixel, 0);
    MagickSetImageBackgroundColor(crop_wand, pixel);
    MagickExtentImage(crop_wand, PPU, height, -(PPU/2), 0);


    writeImage (descriptorId, crop_wand, x, y);
    x++;
  }
}

void cropYTiles (char *descriptorId, MagickWand *magick_wand, double center,
                double image_width, double image_height)

{
  int x = 0;
  int y = 1;
  double bottomOffset;
  double left;

  int i;
  int incrementWidth = PPU/2;
  for (i = (center - PPU); i >= 0; i -= incrementWidth) {
    bottomOffset = (y * incrementWidth) * tanAngle - 1;
    left = i + 2;

    MagickWand 
      *crop_wand;

    double height = image_height - bottomOffset;
    crop_wand = MagickGetImageRegion(magick_wand, incrementWidth, height, left, 0);
    PixelWand *pixel = NewPixelWand();
    PixelSetAlpha(pixel, 0);
    MagickSetImageBackgroundColor(crop_wand, pixel);
    MagickExtentImage(crop_wand, PPU, height, 0, 0);


    writeImage (descriptorId, crop_wand, x, y);
    y++;
  }
}

void writeImage (char *descriptorId, MagickWand *crop_wand, int x, int y) 
{
  MagickBooleanType
    status;

  char outputFilename[90];
  sprintf(outputFilename, "%s=%d_%d.png", descriptorId, x, y);
    status=MagickWriteImages(crop_wand,outputFilename,MagickTrue);
  if (status == MagickFalse)
    ThrowWandException(crop_wand);
  crop_wand=DestroyMagickWand(crop_wand);
}
