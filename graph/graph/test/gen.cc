#include <stdlib.h>
#include <iostream.h>
#include <unistd.h>

int main(int argn, char** argv)
{

  srandom(getpid());

	int n = atoi(argv[1]);

	//cout<<"Enter the number of vertice (per split) : "<<n<<endl;
	//	cin>>n;

	int m= atoi(argv[2]);
	//cout<<"Enter the number of splits : ";
	//cin>>m;

	int p= atoi(argv[3]);;
	//cout<<"Enter number of edges per split : ";
	//cin>>p;
	
	cout<<"GRAPHSTART"<<endl;

	int nv=0;

	//Generating Vertices
	for(int i=0; i<n; ++i)
		for(int j=0; j<m; ++j)
		{
			cout<<nv<<endl;
			nv++;
		}
	cout<<"-1"<<endl;

	//Generating Edges

	int start=0;	
	int end=0;
	for(int j=0; j<m; ++j)
	{
		
		for(int i=0; i<n; ++i)
		{
			end++;
		}
		end--;
		
//		cout<<"Start : "<<start<<"   End : "<<end<<endl;
		
		
		for(int i=0; i<p; ++i)
		{
			int a=random()%n;
			int b=random()%n;
			
			if(a!=b)
			cout<<start+a<<" "<<start+b<<endl;
		}
		end++;
		start=end;
	
	}

	cout<<"-1"<<endl;


	cout<<"GRAPHSTOP"<<endl;


}

