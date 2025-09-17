// C implementations of image processing functions

#include <stdlib.h>
#include <assert.h>
#include "imgproc.h"

// TODO: define your helper functions here
uint8_t emboss_gray(uint32_t pixel, uint32_t pixel_upperleft)
{
  //compute differences: diffr = nr-r, diffg = ng - g, diffb = nb - b
  int32_t r = (pixel >> 24) & 0xFF;
  int32_t g = (pixel >> 16) & 0xFF;
  int32_t b = (pixel >> 8) & 0xFF;

  int32_t nr = (pixel_upperleft >> 24) & 0xFF;
  int32_t ng = (pixel_upperleft >> 16) & 0xFF;
  int32_t nb = (pixel_upperleft >> 8) & 0xFF;

  int32_t diffr = nr - r;
  int32_t diffg = ng - g;
  int32_t diffb = nb - b;

  //priority - red,green,blue
  int32_t diff = diffr;
  if (abs(diffg) > abs(diff) || (abs(diffg) == abs(diff) && diff == diffr))
  {
    diff = diffg;
  }
  if (abs(diffb) > abs(diff) ||
      (abs(diffb) == abs(diff) && diff != diffr && diff == diffg))
  {
    diff = diffb;
  }

  //gray is 128 + diff
  int32_t gray = 128 + diff;
  //must be clamped within range of 0-255
  if (gray < 0)
    gray = 0;
  if (gray > 255)
    gray = 255;

  return (uint8_t)gray;
}

//! Transform the color component values in each input pixel
//! by applying the bitwise complement operation. I.e., each bit
//! in the color component information should be inverted
//! (1 becomes 0, 0 becomes 1.) The alpha value of each pixel should
//! be left unchanged.
//!
//! @param input_img pointer to the input Image
//! @param output_img pointer to the output Image (in which the
//!                   transformed pixels should be stored)
void imgproc_complement(struct Image *input_img, struct Image *output_img)
{
  uint32_t totalPixels = input_img->width * input_img->height;

  // parse through total pixels
  for (int i = 0; i < totalPixels; i++)
  {
    uint32_t pixel = input_img->data[i];

    // isolate all the colors
    uint8_t red = (pixel >> 24);
    uint8_t green = (pixel >> 16);
    uint8_t blue = (pixel >> 8);
    uint8_t alpha = pixel & 0xFF;

    red = ~red;
    blue = ~blue;
    green = ~green;

    // create output
    output_img->data[i] = (((uint32_t)red << 24) | ((uint32_t)green << 16) |
                           ((uint32_t)blue << 8) | (uint32_t)alpha);
  }
}

//! Transform the input image by swapping the row and column
//! of each source pixel when copying it to the output image.
//! E.g., a pixel at row i and column j of the input image
//! should be copied to row j and column i of the output image.
//! Note that this transformation can only be applied to square
//! images (where the width and height are identical.)
//!
//! @param input_img pointer to the input Image
//! @param output_img pointer to the output Image (in which the
//!                   transformed pixels should be stored)
//!
//! @return 1 if the transformation succeeded, or 0 if the
//!         transformation can't be applied because the image
//!         width and height are not the same
int imgproc_transpose(struct Image *input_img, struct Image *output_img)
{
  int height = input_img->height;
  int width = input_img->width;

  if (height != width)
  {
    return 0;
  }

  output_img->width  = width;
  output_img->height = height;

  for (int i = 0; i < height; i++)
  {
    for (int j = 0; j < width; j++)
    {
      int inputIdx = i * width + j;
      int outputIdx = j * height + i;
      output_img->data[outputIdx] = input_img->data[inputIdx];
    }
  }

  return 1;
}

//! Transform the input image by copying only those pixels that are
//! within an ellipse centered within the bounds of the image.
//! Pixels not in the ellipse should be left unmodified, which will
//! make them opaque black.
//!
//! Let w represent the width of the image and h represent the
//! height of the image. Let a=floor(w/2) and b=floor(h/2).
//! Consider the pixel at row b and column a is being at the
//! center of the image. When considering whether a specific pixel
//! is in the ellipse, x is the horizontal distance to the center
//! of the image and y is the vertical distance to the center of
//! the image. The pixel at the coordinates described by x and y
//! is in the ellipse if the following inequality is true:
//!
//!   floor( (10000*x*x) / (a*a) ) + floor( (10000*y*y) / (b*b) ) <= 10000
//!
//! @param input_img pointer to the input Image
//! @param output_img pointer to the output Image (in which the
//!                   transformed pixels should be stored)
void imgproc_ellipse(struct Image *input_img, struct Image *output_img)
{
  int height = input_img->height;
  int width = input_img->width;
  int a = width / 2;
  int b = height / 2;

  for (int i = 0; i < height; i++)
  {
    for (int j = 0; j < width; j++)
    {
      int x = j - a;
      int y = i - b;

      if (((10000 * x * x) / (a * a)) + ((10000 * y * y) / (b * b)) <= 10000)
      {
        output_img->data[i * width + j] = input_img->data[i * width + j];
      }
      else
      {
        output_img->data[i * width + j] = 0x000000FF;
      }
    }
  }
}

//! Transform the input image using an "emboss" effect. The pixels
//! of the source image are transformed as follows.
//!
//! The top row and left column of pixels are transformed so that their
//! red, green, and blue color component values are all set to 128,
//! and their alpha values are not modified.
//!
//! For all other pixels, we consider the pixel's color component
//! values r, g, and b, and also the pixel's upper-left neighbor's
//! color component values nr, ng, and nb. In comparing the color
//! component values of the pixel and its upper-left neighbor,
//! we consider the differences (nr-r), (ng-g), and (nb-b).
//! Whichever of these differences has the largest absolute value
//! we refer to as diff. (Note that in the case that more than one
//! difference has the same absolute value, the red difference has
//! priority over green and blue, and the green difference has priority
//! over blue.)
//!
//! From the value diff, compute the value gray as 128 + diff.
//! However, gray should be clamped so that it is in the range
//! 0..255. I.e., if it's negative, it should become 0, and if
//! it is greater than 255, it should become 255.
//!
//! For all pixels not in the top or left row, the pixel's red, green,
//! and blue color component values should be set to gray, and the
//! alpha value should be left unmodified.
//!
//! @param input_img pointer to the input Image
//! @param output_img pointer to the output Image (in which the
//!                   transformed pixels should be stored)
void imgproc_emboss(struct Image *input_img, struct Image *output_img)
{
  // TODO: implement
  int height = input_img->height;
  int width = input_img->width;

  for (int row = 0; row < height; row++)
  {
    for (int col = 0; col < width; col++)
    {
      //calculate index from flattened array
      int idx = row * width + col;
      uint32_t pixel = input_img->data[idx];
      uint32_t a = pixel & 0xFF;

      //checks for top row or left column
      if (row == 0 || col == 0)
      {
        //make pixel gray
        uint32_t gray = 128;
        output_img->data[idx] = (gray << 24) | (gray << 16) | (gray << 8) | a;
      }
      else
      {
        //find top left neighboring index using flattened array calculation
        int neighbor = (row - 1) * width + (col - 1);
        uint32_t neighborpixel = input_img->data[neighbor];

        //use helper function to find how much brighter/darker pixel should be compared to neighbor
        uint8_t gray = emboss_gray(pixel, neighborpixel);
        //create pixel with gray color from emboss_gray function
        output_img->data[idx] = (gray << 24) | (gray << 16) | (gray << 8) | a;
      }
    }
  }
}
