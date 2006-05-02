#include "gui/gui.h"
#include "vizmo2.h"

int main( int argc, char ** argv )
{    
    //create gui
    if( VizGUI_Init(argc,argv)==false ) return 1;
    return 0;
}
