/***************************************************************************
   NAME
     sphere.h
   PURPOSE
     compute the spherical approximation of the underlying object
   LAST MODIFIED
     05.26.98 L.K.Dale - cleanup

***************************************************************************/
#ifndef include_sphere

/*-------------------------------------------------------------------------
Prototypes
-------------------------------------------------------------------------*/
void
Init_Spherical_Approx(int nvert,float *r);

void
Compute_Part_Leaf_Sphere(part_data *part);

int
Create_Part_Binary_Tree(part_data *part);

#define include_sphere
#endif

