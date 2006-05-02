#include <iostream.h>
#include <fstream.h>
#include <iomanip.h>
#include <stdlib.h>
#include <unistd.h>
#include <vector>
#include <map>

int main(int n, char** argv){
  int i;
  map<int,int> m;
  map<int,int> mit;
  srandom(getpid());

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
  t=0;
  while(1){
    int ne = rand()%20+10;
    if((t+ne) >= sz) break;
    for(int i=t;i < t+ne ;i++){
      //addVertex(i);
      for(int j=i;j<t+ne;j++){
	//randomly decide if there will be an edge between i, j
	//if((m[i][j] == 1) || (m[j][i] == 1)) 
	//mit = m.find(i);
	//if(mit == m.end())
	//if(mit->second == j)
	//  continue;
	//mit = m.find(j);
	//if(mit->second == i)
	//  continue;
	
	if(i != j) {
	  int probability=rand()%sparsity;
	  if(probability==0) myofstream<<i<<" "<<j<<endl;
	  //m[i][j]=m[j][i]=1;
	}
      }
    }
    t+= ne;
    if(t >= sz) break;
  }
  myofstream<<"-1"<<endl;
  myofstream<<"GRAPHSTOP"<<endl;
  //write file to disk
  myofstream.close();
}
