#include "vizRoadmap.h"
#include "vizIncl.h"
 

vizRoadmap::vizRoadmap()
{
  roadmapRead=0;
}
vtkActor* vizRoadmap::CreateCCEdgeActor(vector <pair <Cfg,Cfg> >edges)
{
   vtkActor *act;
  int i,pn[2];
  double coordinate[6];
  vtkFloatPoints *points = vtkFloatPoints::New();
  vtkCellArray *lines = vtkCellArray::New();
  for(i=0;i< edges.size(); i++)
  {
    float *f;
 
    f=project(edges[i].first);
 
    points->InsertNextPoint(f);
    pn[0]=2*i;
 
 
    f=project(edges[i].second);
    //pn[i]=cc[i];
     points->InsertNextPoint(f);
    pn[1]=2*i+1;
 
    lines->InsertNextCell( 2, pn);
   }
  vtkPolyDataMapper* map = vtkPolyDataMapper::New();
  vtkPolyData *polydata = vtkPolyData::New();
  polydata->SetPoints(points);
  polydata->SetLines(lines);
  points->Delete();
  lines->Delete();
 
  map->SetInput(polydata);
  act= vtkActor::New();
  act->SetMapper(map);
  float r= drand48(),g= drand48(),b=drand48();
  //cout << "Connected component color " << r << " " << g << " " <<b << endl;
  act->GetProperty()->SetColor( r,g,b); // sphere color blue
  act->GetProperty()->SetEdgeColor( r,g,b); // sphere color blue
  act->GetProperty()->EdgeVisibilityOn();
  act->GetProperty()->SetRepresentationToWireframe();
  return act;
                       
}
void vizRoadmap::CreateCCEdges() {
   //numCC = roadmap.ccstats.size();
   int ccnum = 1;
   int i;
  vector< vector< pair<Cfg,Cfg> > > ccList;
   ccList=roadmap.GetEdgesByCCVDataOnly();
    numCC==ccList.size();
  cout << "Creating actors for  " << numCC << endl;
  if(ccEdges.size())
  {
     for(i=0;i<ccEdges.size();i++)
          ccEdges[i]->Delete();
     ccEdges.clear();
 
  }
  for(i=0;i<ccList.size();i++) 
     ccEdges.push_back( CreateCCEdgeActor(ccList[i]));
 
}
float* vizRoadmap::project(Cfg coord)
{
  float *projection=new float[3];
  projection[0]=coord.GetSingleParam(0);
  projection[1]=coord.GetSingleParam(1);
  projection[2]=coord.GetSingleParam(2);  
  return projection;
 
}
 
                                            
 
int vizRoadmap::Display(char *file,char *Rend,Tcl_Interp *interp)
{
   int i;
   vtkRenderer *ren;
   int err;
   //Tcl_Obj *resultPtr
   if(roadmapRead) Hide(Rend,interp);
   cout << "Display: reading\n";
   if(ReadRoadmap(file)==TCL_ERROR) return TCL_OK;
   ren = (vtkRenderer *) vtkTclGetPointerFromObject
                (Rend, "vtkRenderer", interp,err);
   cout << "Adding actors for size " << ccEdges.size()<< endl;
   for(i=0;i<ccEdges.size();i++)
     ren->AddActor(ccEdges[i]);
   return TCL_OK;
 
}
 
int vizRoadmap::Hide(char *Rend,Tcl_Interp *interp)
{
   vtkRenderer *ren;
   int i;
   int err;
   if(!roadmapRead) return TCL_OK;
   ren = (vtkRenderer *) vtkTclGetPointerFromObject
                (Rend, "vtkRenderer", interp,err);
   for(i=0;i<ccEdges.size();i++) ren->RemoveActor(ccEdges[i]);
   roadmapRead=0;
   return TCL_OK;
}
                     
int vizRoadmap::ReadRoadmap( char *file)
{
   FILE *in;
   int number;
   int t1,t2;
   char line[256];
   //GraphNode node;
   int i,j;
 
   char tagstring[100];
 
  cout << "Reading roadmap\n";
/*
   ifstream  myifstream(file);
   if (!myifstream) {
                fprintf(stderr,"Could not open the map file %s\n",
                  file);
                sendUserMessage(INVALID_FILE);
                return TCL_ERROR;
 
                }
   do {
    myifstream >> tagstring;
    } while(!strstr(tagstring,"#####DMSTOP"));
   myifstream >> tagstring;
   cout << "Read " << tagstring << endl;
   typedef vector< pair<int,VID> >::const_iterator CCI;
   int maxCCprint; */
   //ReadGraph(myifstream);
   ReadRoadmapGRAPHONLY(file);
   roadmapRead=1; 
   CreateCCEdges();
  
  cout << "Read roadmap " << file <<"\n";
   return TCL_OK;
                                         
}
int vizRoadmap::isRead()
{
  return roadmapRead;
}
