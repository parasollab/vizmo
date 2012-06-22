#include "gui/GUI.h"
#include "vizmo2.h"

int main( int argc, char ** argv )
{    
  if(argc >1){
    cout<<"vizmo++ doesn't take arguments..."<<endl;
    return 1;
  }
  else{
    //create gui
    if( VizGUI_Init(argc,argv)==false ) return 1;
    return 0;
  }
}
