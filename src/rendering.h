#ifndef RENDERING_H
#define RENDERING_H
#include <JPEGDecoder.h>
#include <FS.h>
#include <Adafruit_ILI9341.h>

// Return the minimum of two values a and b
#define minimum(a,b)     (((a) < (b)) ? (a) : (b))

void drawJpeg(const char *filename, int xpos, int ypos, Adafruit_ILI9341* tft);
void jpegRender(int xpos, int ypos, Adafruit_ILI9341* tft);

#endif
