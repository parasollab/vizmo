#ifndef _VIZINCL_H
#define _VIZINCL_H

//***************************************************************
// Vizmo 3D - A Visualization Tool for Motion Modelling
// Author: Renu Isaac
// Filename : vizIncl.h
// Headers for includes and prototypes used by all classes
//***************************************************************

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <tcl.h>
#include <tk.h>

#define TRUE 1
#define FALSE 0

#define MAX_FILENAME 200
#define MAX_ACTORNAME_LENGTH 20
#define MAX_NUMACTORS 200
#define MAX_ANY_NAME 200

#define ENVIRONMENT 0
#define ROADMAP 1
#define QUERY 2
#define PATH 3

/* possible status message id's */
#define READY 0
#define READING_ENVIRONMENT_FILE 1
#define READING_PATH_FILE 2
#define READING_QUERY_FILE 3
#define READING_OBJECT_FILE 4
#define GENERATING_MAP_FILE 5
#define GENERATING_PATH_FILE 6

/* possible cursor shapes */
#define WAIT 1
#define NORMAL 0

/* possible error messages to be sent */
#define INVALID_FILE 1
#define NO_WRITE_PERMISSION 2

#define TCL_ERROR 1



bool CheckValidFile(char *filename) ;
int updateStatus(int messageNo);
int updateCursor(int cursorType);
int sendUserMessage(int messageNo);

#endif
