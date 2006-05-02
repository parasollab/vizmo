#ifndef _GOAL_POSITION_H
#define _GOAL_POSITION_H

// **************** goalPosition *****************
// All configurations of x,y,z,r,p,y can use this 
// ***********************************************
#include "vizIncl.h"
#define MAX_INTERMEDIATE_GOALS 5

class goalPosition
{
	public:
		char name[MAX_ANY_NAME];
		float pos[6];
		void setInfo(float a, float b,
			float c,float d,float e,float f )
		{
			pos[0] = a;
			pos[1] = b;
			pos[2] = c;
			pos[3] = d;
			pos[4] = e;
			pos[5] = f;
		}
		goalPosition( char *title, float a, float b,
			float c,float d,float e,float f )
		{
			strncpy(name,title,MAX_ANY_NAME-1);
			setInfo(a,b, c,d, e, f);
		}
		~goalPosition()
		{}

};

#endif
