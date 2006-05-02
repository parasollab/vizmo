/***************************************************************************
   NAME
     sphere.c
   PURPOSE
     compute the spherical approximation of the underlying object
   NOTES
     There are two parts in this code:
     1. fill object boundaries with sphere leaf nodes.
     2. build a binary tree with these leaf nodes from bottom up
   LAST MODIFIED
     05.26.98 L.K.Dale - cleanup and removal of unused code

***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <memory.h>

//#include "const.h"
//#include "struct.h"
//#include "extern.h"
//#include "macro.h"
//#include "hash.h"
//#include "util.h"
//#include "distance.h"
//#include "sphere.h"

/**************************************************************************
Globally declared but used only by routines in this file
**************************************************************************/
static int level        =0;
static int deepest_level=0;

#define NODE_CHUNK      4096
#define TEMP_NODE_CHUNK 65536

static float min_leaf_dist;
static bt_node **G_node_ptr;
static int G_node_size;
static int G_node_idx;
static long total_nodes;
static long total_leaf_nodes;
static long total_leaf_ptr;
static float leaf_r;

/**************************************************************************
  Init_Spherical_Approx: Initialize the radius of the leaf sphere,
  hash table size, and collision precision
**************************************************************************/

void
Init_Spherical_Approx(int nvert,float *r) {

  V1(fprintf(logfd, "Number of vertices: %d\n", nvert));
  V1(fprintf(logfd, "Leaf sphere size: %f\n", *r));
  leaf_r = *r;
  min_leaf_dist= *r/sqrt(2);
  SetError(EPSILON5);            
  Init_Object_Data();
  Init_Hash_Table_Size(nvert);

}

/**************************************************************************
  Allocate_BT_Nodes: return a bt_node, allocate a chunk when it runs out
**************************************************************************/

bt_node *
Allocate_BT_Node(node_data *nodes) {

  if (nodes->node_idx>=nodes->node_size) {
    /* maintance a list of allocated memory */
    mem_list *tmp;
    tmp=nodes->mlist;
    nodes->mlist=(mem_list *)calloc(1,sizeof(mem_list));
    nodes->mlist->next=tmp;
    nodes->node_idx=0;
    nodes->node_size=NODE_CHUNK;
    nodes->node=(bt_node *)calloc(nodes->node_size,sizeof(bt_node));
    nodes->mlist->ptr=nodes->node;
    nodes->mlist->size=nodes->node_size*sizeof(bt_node);
    if (nodes->node==NULL) {
      NO_MORE_MEMORY("Allocate_BT_Node");
      return(NULL);
    }
    total_nodes+=nodes->node_size;
  }

  nodes->total_node++;
  return(&nodes->node[nodes->node_idx++]);
}

/**************************************************************************
  Check_BT_Node_Size: Allocate and fill in the bt_node structure with the 
  given formal arguments
**************************************************************************/

bt_node *
Create_BT_Node(node_data *nodes,fpoint3D c,float r,triangle_data *t,
	       bt_node *left,bt_node *right) {

  bt_node *ret;

  if ((ret=Allocate_BT_Node(nodes))!=NULL) {
    ret->c=c;
    ret->r=r;
    ret->tri=t;
    ret->left=left;
    ret->right=right;
  }

  return(ret);
}

/**************************************************************************
  Check_BT_Leaf_Node_Size: allocate leaf pointers to the the real bt_node
  memory
**************************************************************************/

int
Check_BT_Leaf_Node_Size(node_data *nodes) {

  bt_node **tmp;
  
  /* pointer to the bt_nodes, need to be continuous */
  if (nodes->nleaf>=nodes->leaf_size) {
    if (nodes->leaf_size==0) {
      nodes->leaf_size=NODE_CHUNK;
      nodes->leaf=(bt_node **)calloc(nodes->leaf_size,sizeof(bt_node *));
    } else {
      tmp=nodes->leaf;
      nodes->leaf_size+=NODE_CHUNK;
      nodes->leaf=(bt_node **)calloc(nodes->leaf_size,sizeof(bt_node *));
      BCOPY(tmp,nodes->leaf,(nodes->leaf_size-NODE_CHUNK)*sizeof(bt_node *));
      free(tmp);
    }
    if (nodes->leaf==NULL) {
      NO_MORE_MEMORY("Check_BT_Leaf_Node_Size");
      return(FALSE);
    }
    total_leaf_ptr += NODE_CHUNK;
  }

  return(TRUE);

}

/**************************************************************************
  Create_BT_Leaf_Node: create a bt_node and assign leaf data to it, 
  and record where it is. This info may be eliminated in the future.
**************************************************************************/

bt_node *
Create_BT_Leaf_Node(node_data *nodes,fpoint3D c,float r,triangle_data *t) {
  
  bt_node *ret;
  
  if (!Check_BT_Leaf_Node_Size(nodes)) {
    return(NULL);
  }
  ret=Create_BT_Node(nodes,c,r,t,NULL,NULL);
  nodes->leaf[nodes->nleaf++]=ret;
  total_leaf_nodes++;

  return(ret);

}

/**************************************************************************
  Compute_Line_Segment_Leaf_Sphere: Given a line segment (two points in space)
  fill it with spheres such that the retangular strip with thinkness 
  min_leaf_dist is covered.
**************************************************************************/

void 
Compute_Line_Segment_Leaf_Sphere(node_data *nodes,triangle_data *triangle,
				 fpoint3D p1,fpoint3D p2) {

  int i,j;
  float len,dist,len1,dd=min_leaf_dist*2.0;
  fpoint3D v21,adv,center;

  SUBST_VEC(p2,p1,v21);
  len=lengthof(v21);
  
  if (len<=min_leaf_dist) {
    /* special case where the strip can be covered with a sphere */
    MIDPOINT(p1,p2,center);
    Create_BT_Leaf_Node(nodes,center,leaf_r,triangle);
  } else if (len<=dd) {
    /* special case where the strip can be cover with two spheres */
    MIDPOINT(p1,p2,adv);
    MIDPOINT(p1,adv,center);
    Create_BT_Leaf_Node(nodes,center,leaf_r,triangle);
    MIDPOINT(p2,adv,center);
    Create_BT_Leaf_Node(nodes,center,leaf_r,triangle);
  } else {
    /* the other case, find the increment length, can create one by one */
    len1=min_leaf_dist/len;
    SCALE_VEC(v21,len1,adv);
    i=0;
    dist=dd;
    while(dist<len) {
      j=2*i+1;
      ADD_COEF1_VEC(p1,j,adv,center);
      Create_BT_Leaf_Node(nodes,center,leaf_r,triangle);
      dist += dd;
      i++;
    }
    /* special treatment for the last sphere */
    if (dist-len>EPSILON6) {
      ADD_COEF1_VEC(center,1,adv,center);
      MIDPOINT(center,p2,center);
      Create_BT_Leaf_Node(nodes,center,leaf_r,triangle);
    }
  }
  
  return;
}

/**************************************************************************
  Compute_Triangle_Leaf_Sphere
**************************************************************************/

void 
Compute_Triangle_Leaf_Sphere(node_data *nodes,triangle_data *triangle,
			     int always) {

  int i;
  float len,d2=min_leaf_dist*2,len1,len2,len3,d,h,r1,r2,curh,adv,leni;
  fpoint3D p1,p2,p3,start,end;
  fpoint3D v21,v31,v23,nv,normal;

  p1=triangle->vert[0]->lvert;
  p2=triangle->vert[1]->lvert;
  p3=triangle->vert[2]->lvert;
  if (triangle->radius<leaf_r) {
    /* special case where the triangles can be covered with one sphere */
    GET_GRAVITY_CENTER(p1,p2,p3,start);
    Create_BT_Leaf_Node(nodes,start,triangle->radius,triangle);
    return;
  } else if (triangle->radius<leaf_r*2.0) {
    /* special case where the triangles can be covered with three spheres */
    GET_GRAVITY_CENTER(p1,p2,p3,start);
    MIDPOINT(start,p1,end);
    Create_BT_Leaf_Node(nodes,end,triangle->radius*0.5,triangle);
    MIDPOINT(start,p2,end);
    Create_BT_Leaf_Node(nodes,end,triangle->radius*0.5,triangle);
    MIDPOINT(start,p3,end);
    Create_BT_Leaf_Node(nodes,end,triangle->radius*0.5,triangle);
    return;
  }
  SUBST_VEC(p1,p2,v21);
  SUBST_VEC(p1,p3,v31);
  SUBST_VEC(p3,p2,v23);
  
  len1=lengthof(v23);
  len2=lengthof(v31);
  len3=lengthof(v21);

  /* special case where the triangle is a long strip */
  if (len1<leaf_r) {
    MIDPOINT(p2,p3,start);
    Compute_Line_Segment_Leaf_Sphere(nodes,triangle,p1,start);
    return;
  } else if (len2<leaf_r) {
    MIDPOINT(p1,p3,start);
    Compute_Line_Segment_Leaf_Sphere(nodes,triangle,p2,start);
    return;
  } else if (len3<leaf_r) {
    MIDPOINT(p1,p2,start);
    Compute_Line_Segment_Leaf_Sphere(nodes,triangle,p3,start);
    return;
  }

  leni=1.0/len1;
  SCALE_VEC(v23,leni,v23);
  leni=1.0/len2;
  SCALE_VEC(v31,leni,v31);
  leni=1.0/len3;
  SCALE_VEC(v21,leni,v21);

  /* find nv: the normal vector on the triangle for the base edge v23 */
  CROSS_PROD(v23,v31,normal);
  CROSS_PROD(normal,v23,nv);
  len=lengthof(nv);
  /* special case where it is a colinear triangle, find the longest edge */
  if (len<EPSILON6) {
    if (len1>len2) {
      if (len1>len3) {
	start=p2;
	end=p3;
      } else {
	start=p1;
	end=p2;
      }
    } else {
      if (len2>len3) {
	start=p1;
	end=p3;
      } else {
	start=p1;
	end=p2;
      }
    }
    Compute_Line_Segment_Leaf_Sphere(nodes,triangle,start,end);
    return;
  }

  /* otherwise, fill the triangle with scanline algorithm */
  /* find the height of the triangle */
  leni=1.0/len;
  SCALE_VEC(nv,leni,nv);
  h=DOT_PROD(nv,v31)*len2;
  /* find the advance distances r1,r2 of the two side edge */
  r1=len3/h;
  r2=len2/h;
  /* first scan line */
  ADD_COEF1_VEC(p2,min_leaf_dist,nv,start);
  ADD_COEF1_VEC(p3,min_leaf_dist,nv,end);
  Compute_Line_Segment_Leaf_Sphere(nodes,triangle,start,end);
  i=1;
  curh=d2;
  while(curh<h) {
    adv=r1*d2*i;
    ADD_COEF1_VEC(p2,adv,v21,start);
    ADD_COEF1_VEC(start,min_leaf_dist,nv,start);
    adv=r2*d2*i;
    ADD_COEF1_VEC(p3,adv,v31,end);
    ADD_COEF1_VEC(end,min_leaf_dist,nv,end);
    Compute_Line_Segment_Leaf_Sphere(nodes,triangle,start,end);
    i++;
    curh+=d2;
  }
  /* see if the last scan line is needed */
  d=h-curh+d2;
  if (d>min_leaf_dist) {
    MIDPOINT(start,p1,start);
    MIDPOINT(end,p1,end);
    Compute_Line_Segment_Leaf_Sphere(nodes,triangle,start,end);
  }
  return;

}


/**************************************************************************
  Compute_Part_Leaf_Sphere: Compute leaf spheres for a part
**************************************************************************/

void 
Compute_Part_Leaf_Sphere(part_data *part) {

  register int i,j;
  polygon3D *poly;
  int always=(part->type==MOVING_PART);

  memset(&part->nodes,0,sizeof(node_data));
  for(i=0;i<part->npoly;i++) {
    poly = &part->poly3[i];
    for(j=0;j<poly->ntriangle;j++) {
      Compute_Triangle_Leaf_Sphere(&part->nodes,&poly->triangle[j],always);
    }
  }

}

/*************************************************************************
**************************************************************************
  Routines to compute the binary tree for the assembly
**************************************************************************
**************************************************************************/

/**************************************************************************
  Checkout_Temp_Node_Pointer_Idx: May ask for a chunk of memory at a time
  if the amount is greater than the reserved, allocate more for it and keep 
  the remains reserved.
**************************************************************************/

int
Checkout_Temp_Node_Pointer_Idx(int size) {

  int ret;
  bt_node **tmp;

  /* for self-protection */
  if (size<=0) return(-1);

  if (G_node_idx+size>=G_node_size) {
    /* not enough */
    if (G_node_size==0) {
      /* the first call */
      G_node_size=MAX(size*2,TEMP_NODE_CHUNK*2);
      G_node_ptr=(bt_node **)calloc(G_node_size,sizeof(bt_node *));
	  if (G_node_ptr==NULL) {
		NO_MORE_MEMORY("Checkout_Temp_Node_Pointer_Idx");
		return(-1);
	  }
    } else {
      /* the following calls */
      /* the memory chunk needs to be contiguous */
      tmp=G_node_ptr;
      G_node_size+=MAX(TEMP_NODE_CHUNK,G_node_idx+size-G_node_size);
      G_node_ptr=(bt_node **)calloc(G_node_size,sizeof(bt_node *));
	  if (G_node_ptr==NULL) {
		NO_MORE_MEMORY("Checkout_Temp_Node_Pointer_Idx");
		return(-1);
	  }
      BCOPY(tmp,G_node_ptr,G_node_idx*sizeof(bt_node *));
      free(tmp);
    }
  }

  /* for debugging purpose */
  if (G_node_idx+size>=G_node_size) {
    Warning_Bell();
    fprintf(logfd,"Warning: Memory allocation is not correct\n");
    return(-1);
  }
  ret=G_node_idx;
  G_node_idx+=size;
  return(ret);

}

/**************************************************************************
  Free_Temp_Node_Pointer: Don't really free it but just increase the reserved 
  size.
**************************************************************************/

void
Free_Temp_Node_Pointer(int size) {

  G_node_idx-=size;

}

/**************************************************************************
  Compute_Nodes_BBox: Given a list of nodes, computing their bounding box
**************************************************************************/

bbox3d
Compute_Nodes_BBox(int n,bt_node **node) {

  register int i;
  bbox3d bb;

  Reset_BBox(&bb);
  for(i=0;i<n;i++) {
    UPDATE_BBOX(node[i]->c,bb);
  }
  return(bb);
}

/**************************************************************************
  X_Partition: Partition a list of nodes into two according to the mid-plane
  along the x axis.
**************************************************************************/

void
X_Partition(float mx,int n_node,bt_node **in_node,int *ln,bt_node **l_node,
	    int *rn,bt_node **r_node) {

  register int i;

  for(i=0,*ln=0,*rn=0;i<n_node;i++) {
    if (in_node[i]->c.x<mx) {
      l_node[(*ln)++]=in_node[i];
    } else {
      r_node[(*rn)++]=in_node[i];
    }
  }
  return;
}

/**************************************************************************
  Y_Partition: Partition a list of nodes into two according to the mid-plane
  along the y axis.
**************************************************************************/

void
Y_Partition(float my,int n_node,bt_node **in_node,int *ln,bt_node **l_node,
	    int *rn,bt_node **r_node) {

  register int i;

  for(i=0,*ln=0,*rn=0;i<n_node;i++) {
    if (in_node[i]->c.y<my) {
      l_node[(*ln)++]=in_node[i];
    } else {
      r_node[(*rn)++]=in_node[i];
    }
  }
  return;
}

/**************************************************************************
  Z_Partition: Partition a list of nodes into two according to the mid-plane
  along the z axis.
**************************************************************************/

void
Z_Partition(float mz,int n_node,bt_node **in_node,int *ln,bt_node **l_node,
	    int *rn,bt_node **r_node) {

  register int i;

  for(i=0,*ln=0,*rn=0;i<n_node;i++) {
    if (in_node[i]->c.z<mz) {
      l_node[(*ln)++]=in_node[i];
    } else {
      r_node[(*rn)++]=in_node[i];
    }
  }
  return;
}

/**************************************************************************
  Blind_Partition: Partition a list of nodes blindly. Used when they are all
  the same point
**************************************************************************/

void
Blind_Partition(int n_node,bt_node **in_node,int *ln,bt_node **l_node,
		int *rn,bt_node **r_node) {
  
  register int i;
  int mn=n_node/2;

  for(i=0,*ln=0;i<mn;i++)
    l_node[(*ln)++]=in_node[i];

  for(i=mn,*rn=0;i<n_node;i++)
    r_node[(*rn)++]=in_node[i];

  return;
}

/**************************************************************************
  Find_Enclosing_Circle: Find the center and radius of the sphere that 
  enclose the two given spheres
**************************************************************************/

void
Find_Enclosing_Circle(fpoint3D c1,fpoint3D c2,float R1,float R2,
		      fpoint3D *c,float *R) {

  float cdist,ratio1,ratio2;

  cdist=sqrt(SQR(c1.x-c2.x)+SQR(c1.y-c2.y)+SQR(c1.z-c2.z));
  if (fabs(cdist)<EPSILON6) {
    (*R)=MAX(R1,R2);
    ratio1=ratio2=0.5;
  } else {
    (*R)=(cdist+R1+R2)*0.5;
    ratio1=((*R)-R1)/cdist;
    if (ratio1>1.0) ratio1=1.0;
    ratio2=1-ratio1;
  }
  c->x=c1.x*ratio1+c2.x*ratio2;
  c->y=c1.y*ratio1+c2.y*ratio2;
  c->z=c1.z*ratio1+c2.z*ratio2;
  return;
}

/**************************************************************************
  Create_Binary_Tree: the recursive function call to build a binary tree 
  from bottom up.
**************************************************************************/
bt_node *
Create_Binary_Tree(node_data *nodes,int n_node,bt_node **in_node) {

  int ln,rn,max_axis;
  int l_node_idx,r_node_idx;
  bt_node *left,*right;
  fpoint3D center;
  bbox3d bb;
  float mp,r,d[3];
  
  if (n_node==0) {
    return(NULL);
  } else if (n_node==1) {
    return(in_node[0]);
  } else if (n_node==2) {
    Find_Enclosing_Circle(in_node[0]->c,in_node[1]->c,
			  in_node[0]->r,in_node[1]->r,
			  &center,&r);
    return(Create_BT_Node(nodes,center,r,NULL,in_node[0],in_node[1]));
  }

  V1(level++;
     deepest_level=MAX(deepest_level,level));

  /* must be allocated in the reversed calling order */
  if ((r_node_idx = Checkout_Temp_Node_Pointer_Idx(n_node)) == -1) {
    NO_MORE_MEMORY("Create_Binary_Tree");
    return(NULL);
  }
  if ((l_node_idx = Checkout_Temp_Node_Pointer_Idx(n_node)) == -1) {
    NO_MORE_MEMORY("Create_Binary_Tree");
    return(NULL);
  }

  bb=Compute_Nodes_BBox(n_node,in_node);
  d[0]=bb.max.x-bb.min.x;
  d[1]=bb.max.y-bb.min.y;
  d[2]=bb.max.z-bb.min.z;

  max_axis= (d[0]>d[1]? (d[0]>d[2]?0:2):(d[1]>d[2]?1:2));

  switch(max_axis) {
  case 0:
    mp=(bb.max.x+bb.min.x)*0.5;
    X_Partition(mp,n_node,in_node,&ln,G_node_ptr+l_node_idx,
		&rn,G_node_ptr+r_node_idx);
    break;
  case 1:
    mp=(bb.max.y+bb.min.y)*0.5;
    Y_Partition(mp,n_node,in_node,&ln,G_node_ptr+l_node_idx,
		&rn,G_node_ptr+r_node_idx);
    break;
  case 2:
    mp=(bb.max.z+bb.min.z)*0.5;
    Z_Partition(mp,n_node,in_node,&ln,G_node_ptr+l_node_idx,
		&rn,G_node_ptr+r_node_idx);
    break;
  default:
    break;
  }
  if (!(ln&&rn)) {
    Warning_Bell();
    fprintf(logfd,"Warning: partition fail, continue.\n");
    Blind_Partition(n_node,in_node,&ln,G_node_ptr+l_node_idx,
		    &rn,G_node_ptr+r_node_idx);
  }
  
  /* notice the order of calls */
  /* left branch */
  left=Create_Binary_Tree(nodes,ln,G_node_ptr+l_node_idx);
  Free_Temp_Node_Pointer(n_node);
  /* right branch */
  right=Create_Binary_Tree(nodes,rn,G_node_ptr+r_node_idx);
  Free_Temp_Node_Pointer(n_node);

  V2(level--);
  if ((left!=NULL)&&(right!=NULL)) {
    /* create a node from two children */
    Find_Enclosing_Circle(left->c,right->c,left->r,right->r,
			  &center,&r);
    return(Create_BT_Node(nodes,center,r,NULL,left,right));
  } else if ((left==NULL)&&(right==NULL)) {
    return(NULL);
  } else if (left==NULL) {
    return(right);
  } else {
    return(left);
  }

}

/**************************************************************************
  Create_Part_Binary_Tree: Create a binary tree for a part (part=MOVIE.BYU)
  set up some data structure and then call the recursive function:
  Create_Binary_Tree
**************************************************************************/

int
Create_Part_Binary_Tree(part_data *part) {

  register int i;
  int n_node;
  bt_node **in_node;

  n_node=part->nodes.nleaf;
  V1(fprintf(logfd,"For part #%d, with %d leaves.\n",part->id, n_node));
  Vn(fprintf(logfd,"Temp node size before building the tree:%d\n",G_node_idx));
  deepest_level=0;
  if (n_node>0) {
    if ((in_node = (bt_node **)calloc(n_node,sizeof(bt_node *))) == NULL) {
      NO_MORE_MEMORY("Create_Part_Binary_Tree");
      return(FALSE);
    }
    
    for(i=0;i<n_node;i++) {
      in_node[i] = part->nodes.leaf[i];
    }
    if ((part->root=Create_Binary_Tree(&part->nodes,n_node,in_node))==NULL) {
      return(FALSE);
    }
  }
  Vn(fprintf(logfd,"Temp node size after building the tree:%d\n",G_node_idx));
  V1(fprintf(logfd,"Number of leaf nodes and total nodes:[%d,%d]\n",
	     part->nodes.nleaf,part->nodes.total_node));
  V1(fprintf(logfd,"Depths of the tree:%d\n",deepest_level));
  return(TRUE);
}
