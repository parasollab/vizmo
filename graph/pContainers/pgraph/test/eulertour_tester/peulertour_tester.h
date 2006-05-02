/* Tester for parallel Euler Tour algo.
 * inheritted from ETTester
 */
#include "commontester.h"
//#include "pContainers/pgraph/pET.h"
//#include <vector>

using namespace stapl;

/* a BasePObject to do collection work 
 * which means collecting all parts for the pList (eulertour)
 * on all threads to thread0  - needed for correctness checking
 */
class comm_collect: public BasePObject {
  vector<euleredge>* totaltour;
public:
  comm_collect(vector<euleredge>* _totaltour) {
    totaltour = _totaltour;
    register_this(this);
  }
  ~comm_collect() {
    rmiHandle handle = this->getHandle();
    if (handle>-1) stapl::unregister_rmi_object(handle);
  }
  void SendMyEntriesToZero(vector<euleredge>& myes) {
    stapl::async_rmi(0, this->getHandle(), &comm_collect::GetEntries, myes);
  }
  void GetEntries(vector<euleredge> ines) {
    for (vector<euleredge>::iterator it=ines.begin(); it!=ines.end(); ++it) {
      totaltour->push_back(*it);
    }
  }
};

/*
bool euleredge_compare(const euleredge& e1, const euleredge& e2)
{
  int k1 = e1.GetRank(); int k2 = e2.GetRank();
  return (k1>k2);
}
*/

template <class PTREE>
class Tester_pET : public ETTester<PTREE> {
  int v_num;
  VID startvid;

  void collect(pList<euleredge>& originaltour, vector<euleredge>* totaltour) {
    for (pList<euleredge>::iterator it=originaltour.local_begin(); it!=originaltour.local_end(); ++it)  
      totaltour->push_back(*it);

    comm_collect collector(totaltour);
    stapl::rmi_fence();
    if (stapl::get_thread_id() != 0) 
      collector.SendMyEntriesToZero(*totaltour);
    stapl::rmi_fence();
    if (stapl::get_thread_id() == 0) {
      sort(totaltour->begin(), totaltour->end(), euleredge_compare);
    }
  }

  void GenLineTree(int n, PTREE& g) {
    //n = number of vertices
    //generated graph is 0-1-2-3-...-(n-1)
    Task data;
    int nprocs = stapl::get_num_threads();
    int myid = stapl::get_thread_id();
    int each = n/nprocs; //assume that there's no remain

    stapl::rmi_fence();
    for (int i=0; i<n/nprocs; i++) {
      data.SetWeight(myid*each+i);
      g.AddVertex(data, myid*each+i, myid); 
    }
    stapl::rmi_fence();
    //g.pDisplayGraph();
    //stapl::rmi_fence();
    VID u,v;
    for (int i=0; i<each-1; i++) {
      u=myid*each+i; v=u+1;
      g.AddEdge(u,v,u*v);
    }
    int i = each-1;
    if (myid != nprocs -1) {  //I'm not the last vertex
      u=myid*each+i; v=u+1;
      g.AddEdge(u,v,u*v);
    }
    stapl::rmi_fence();
    //g.pDisplayGraph();
    //stapl::rmi_fence();
  }

  bool correctcheck(PTREE& ptree, vector<euleredge> totaltour, int v_num, VID startvid) {
    //checking 1) #edges in the tour == 2 * #edges in the graph = 2 * (#vertices -1)
    if (totaltour.size() != 2*(ptree.size()-1) ) {
	  return false;
    }
    //cout<<"Number of euleredges is correct:"<<totaltour.size()<<endl;

    //checking 2) the first vertex in the tour should be startvid, if startvid is provided
    vector<euleredge>::iterator it, nit;
    VID firstvid;
    it=totaltour.begin();
    firstvid = (*it).GetFirstVid();
    if (startvid!=-1) {
      if (firstvid != startvid) {
 	cout<<" The first vertex "<<firstvid<<" is not startvid"<<startvid<<endl;
	return false;
      }
      //cout<<"Starting vertex of the tour is correct:"<<startvid<<endl;
    }
    
    //checking 3) the ranks of each edge in the tour should be: 2M-1, 2M-2, ..., 1,0
    //where 2M = totaltour.size() == 2 * #edges in the graph
    int edgecounter = totaltour.size()-1;
    while (it!=totaltour.end()) {
      if ((*it).GetRank() != edgecounter) {
	cout<<" euleredge "<<*it<<" rank should be "<<edgecounter<<" but not..."<<endl; 
	return false;
      }
    //checking 4) the last edge in the tour has ending point "lastvid" equal to the first vid
      if (edgecounter==0) {
        VID lastvid = (*it).GetSecondVid();
	if (lastvid != firstvid) {
	  cout<<" lastvid "<<lastvid<< "does not return to firstvid "<<firstvid<<endl;
	  return false;
	} //else cout<<"Last euleredge has lastvid "<<lastvid<<" matches firstvid"<<endl;
      }
      VID this2 = (*it).GetSecondVid();
      nit = it; nit++;
      if (nit==totaltour.end() ) break;
      VID next1 = (*nit).GetFirstVid();
    //checking 5) for each edge, check whether it's connected to next edge in the tour
      if (this2 != next1) {
	cout<<" 2 euleredges: "<<*it<<" and "<<*nit<<" are not connected"<<endl;
	return false;
      }
      it++; edgecounter--;
    }

    //checking 6) for each edge in the graph, find it in the eulertour
    //there should be 1 and only 1 for either direction 
    VID u,v;
    for (u=0; u<v_num-1; u++) {
      v = u+1;
      if (u>v) continue; //only consider each undirected edge once, but look for both directions
      euleredge tmpe(u,v);
      it=find(totaltour.begin(), totaltour.end(), tmpe);
      if (it==totaltour.end()) {
	cout<<" Cannot find "<<u<<" -> "<<v<<endl;
	return false;
      } else {
 	  //cout<<" find "<<vi->vid<<" -> "<<ei->vertex2id<<"  at rank "<<(*it).GetRank()<<endl;		
	  (*it).SetVids(-1, -1);
      }
      euleredge tmpe1(v,u);
      it=find(totaltour.begin(), totaltour.end(), tmpe1);
      if (it==totaltour.end()) {
	cout<<" Cannot find "<<v<<" -> "<<u<<endl;
	return false;
      } else {
 	//cout<<" find "<<ei->vertex2id<<" -> "<<vi->vid<<"  at rank "<<(*it).GetRank()<<endl;		
	(*it).SetVids(-1, -1);
      }
    }

    //checking 7) there should be no edges left in the totaltour
    //since each matching edges have been set to be invalid in last step
    for(it=totaltour.begin(); it!=totaltour.end(); ++it) {
      if (!(*it == euleredge(-1, -1) )) {
	cout<<" Remaining euleredge:"<<*it<<" not changed to -1,-1"<<endl;
	return false;
      }
    }

    //finish checking
    stapl_print("Complete checking!\n");
    return true;
  }

public:
  Tester_pET() { flag='p'; v_num=0; startvid=-1; }
  Tester_pET(int _v_num) { flag='p'; v_num=_v_num; startvid=-1;  }
  Tester_pET(int _v_num, VID _startvid) { flag='p'; v_num=_v_num; startvid=_startvid;}
 
  char *GetName() { return "Tester_pET"; }

  void ShowUsage() {
    stapl_print("\n"); stapl_print(GetName()); stapl_print(" p|c|b"); stapl_print(" #vertices startvid\n"); stapl_print("Notice: #vertices % #threads is assumed to be 0\n");
  }
  
  ETTester<PTREE> *CreateCopy() { Tester_pET *t = new Tester_pET; t->flag=this->flag; t->v_num=this->v_num; t->startvid=this->startvid; return t; }

  void ParseParams(char *s) {
    char *space=" "; char *p;
    int nprocs = stapl::get_num_threads();
    if (! (p=strtok(s,space) ) ) {
      ShowUsage(); exit(1);
    } else flag = p[0];
    if (! (flag=='p' || flag=='c' || flag=='b') ) {
      ShowUsage(); exit(1);
    }
    if ( p = strtok(NULL,space) )
      v_num = atoi(p);
    if (v_num<0 || v_num%nprocs!=0) {
      ShowUsage(); exit(1);
    }
    if ( p = strtok(NULL,space) )
      startvid = atoi(p);
    else startvid = -1;
    if ( (startvid !=-1 && startvid <0) || (startvid>=v_num) ) {
      ShowUsage(); exit(1);
    }
    return;
  }

  void SetTestNo() {
    testno = 1;
    ElapsedTimes = *(new vector<vector<double> > (testno) );
  }

  void Test(PTREE& ptree) {
    counter = 0;

    MethodNames.push_back("pET_Tester");
    GenLineTree(v_num, ptree);
    stapl::rmi_fence();
    //ptree.pDisplayGraph();
    //stapl::rmi_fence();

    pList<euleredge> eulertour;
    stapl::rmi_fence();
   
    if_want_perf {starttimer; }
    pET<PTREE>(ptree, eulertour, startvid);
    stapl::rmi_fence();
    if_want_perf {newstoptimer; }

    if_want_corr {
      vector<euleredge> totaltour;
      collect(eulertour, &totaltour);
      stapl::rmi_fence();
      if (stapl::get_thread_id() == 0) {
        for (vector<euleredge>::iterator it=totaltour.begin(); it!=totaltour.end(); ++it) {
            cout<<*it<<endl;
        }
        TREE tree;
        bool correctflag = correctcheck(ptree, totaltour, v_num, startvid);
        if (correctflag) cout<<"Succeed!"<<endl;
        else cout<<"Fail!"<<endl;
      } 
      stapl::rmi_fence();
    }
  }
  
};
