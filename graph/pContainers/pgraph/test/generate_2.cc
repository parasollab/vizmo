#include <iostream.h>
#include <fstream.h>
#include <iomanip.h>
#include <stdlib.h>
#include <unistd.h>
#include <vector>
#include <map>

int main(int n, char** argv){
  int i;
//  map<int,int> m;
//  map<int,int> mit;
 // srand(getpid());
cout<<"pid "<< getpid() <<endl;

  int sz = atoi(argv[1]);
  int sparsity = atoi(argv[2]);
  int probability;
  ofstream  myofstream("out");
  if (!myofstream) {
    cout << "\nInWriteGraph: can't open outfile: " ; 
  }
  //m.resize(sz);
  //for(int i=0;i < sz;i++) m[i].resize(sz,0);
  
  myofstream<<"GRAPHSTART"<<endl;
  for(int i=0;i < sz;i++) myofstream<< i <<endl;
  myofstream<<"-1"<<endl;
  int k,t;
    for(int i=0;i < sz;i++){
      for(int j=0;j< sz;j++){
	if(i != j) {
	  int probability=rand()%sparsity;
	  if(probability==0) myofstream<<i<<" "<<j<<endl;
        }  
      }
    }
  myofstream<<"-1"<<endl;
  myofstream<<"GRAPHSTOP"<<endl;
  //write file to disk
  myofstream.close();
}
