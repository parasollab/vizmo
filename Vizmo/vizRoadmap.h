#ifndef _vizRoadmap_h
#define _vizRoadmap_h
#include "OBPRM.h"
#include "Roadmap.h"
#include "vtkActor.h"
#include "vtkRenderWindow.h"
#include "vtkCamera.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderer.h"
#include "vtkBYUReader.h"
#include "vtkTclUtil.h"
#include "vtkPolyData.h"
#include "vtkSphereSource.h"
#include "vtkPolyDataMapper.h"
#include "vtkMatrix4x4.h"
#include "vtkTransform.h"
#include "vtkCellArray.h"
#include "vtkFloatPoints.h"           

#include <stdio.h>
class vizRoadmap: Roadmap{
    public:
    vizRoadmap();
    vtkActor* CreateCCEdgeActor(vector <pair <Cfg,Cfg> >edges);
    void displayCC(vtkRenderer *ren,vtkActor *act);                     
    void hideCC(vtkRenderer *ren,vtkActor *act);      
    int Display(char *file,char *Rend,Tcl_Interp *interp);
    int Hide(char *Rend,Tcl_Interp *interp)  ;
    void CreateCCEdges(); 
    int ReadRoadmap( char *file);
    int isRead();
    private:
    int roadmapRead;
    int numCC;
    float *project(Cfg);
    vector<vtkActor*> ccEdges;

};
#endif

