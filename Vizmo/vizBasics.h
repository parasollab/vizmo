#ifndef _VIZBASICS_H
#define _VIZBASICS_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define FALSE		0
#define TRUE		1

#define EPSILON         0.00005

#define vizFloat  float
#define vizInt    int
#define vizChar   char
#define vizBool   int
#define ULONG     unsigned long
#define USHORT    unsigned short

// Bezier Splines sampling methods
#define VIZ_UNIFORM 1  

// Type definitions for parameters in vizFlatContour
#define VIZ_LEFT  0
#define VIZ_RIGHT 1
#define VIZ_BY    0
#define VIZ_YB    1
#define VIZ_GB    2
#define VIZ_BG    3
#define VIZ_GY    4
#define VIZ_YG    5

#endif 
