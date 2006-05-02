#include <stdlib.h>
#include "timing.h"

#include "Graph.h"
#include "GraphAlgo.h"
#include <iostream>

void generate(char * file_name,int nr_vertices, int step ){

 
int i, j;
 
char * actual_file_name=new char[100];
     sprintf(actual_file_name,"%s.scc",file_name);
     printf("writing scc to file [%s]\n",actual_file_name);
     ofstream _myistream(actual_file_name);

 if(!_myistream.is_open()){
       cout<<"Error opening output file "<<actual_file_name<<endl;
       return ;
     }

  _myistream<< "GRAPHSTART"<<endl;
 
 for( i=0; i< nr_vertices ; i++)
   _myistream<< i <<endl;

 _myistream<< -1 <<endl;
   
 for(  i=0; i< (nr_vertices)/2; i++)
      _myistream<< i <<"  "<< i*2 <<endl;


for(  i=0; i< (nr_vertices-step); i++)
      _myistream<< i <<"  "<< i+ step <<endl;



 for(  i=0; i< nr_vertices-2*step ; i++)
      _myistream<< i <<"  "<< i+ 2*step <<endl; 

 for ( i = rand()%nr_vertices ;  i< nr_vertices/4 ; i= i+5*step )
     _myistream<< i <<"  "<< i- 2*step <<endl;

 _myistream<< -1 <<endl;
      
  _myistream<< "GRAPHSTOP"<<endl;

}


 int main(int argc , char ** argv ){

  int nr_vertices = atoi(argv[2]);
  int step  = atoi(argv[3]);
  generate( argv[1] , nr_vertices, step );
 

  }
