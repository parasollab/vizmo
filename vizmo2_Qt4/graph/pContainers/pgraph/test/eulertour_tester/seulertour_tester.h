/* Tester classes for basic methods of sequential Euler Tour algo.
 * inheritted from ETTester
 */
#include "commontester.h"

using namespace stapl;

template <class TREE>
class Tester_sET : public ETTester<TREE> {
  int v_num;
  VID startvid;

  void sET(TREE& tree, vector<euleredge>& totaltour) {
    typename TREE::VI vi, vj;
    typename TREE::EI ei, ej;
    VID u,v,w;

    int edgecounter = 2 * (v_num -1) - 1;  //the max. rank
    euleredge startedge, endedge;
    euleredge e, nexte;

    //decide the startedge
    if (startvid != -1) {
      if (tree.IsVertex(startvid, &vi) ) {
	ei = vi->edgelist.begin();
	v = ei->vertex2id;
	startedge.SetVids(startvid, v);
	startedge.SetRank(edgecounter);
      }
    } else {
      vi = tree.begin();
      u = vi->vid;
      ei = vi->edgelist.begin();
      v = ei->vertex2id;
      startedge.SetVids(u, v);
      startedge.SetRank(edgecounter);
    }

    //decide the endedge - the pred. of startedge
    //since startedge is always the leftmost in startvid's edgelist
    //endedge is always the rightmost one
    u = startedge.GetFirstVid();
    if (tree.IsVertex(u, &vj) ) {
      ej = vj->edgelist.end(); ej--;
      w = ej->vertex2id;
    }
    endedge.SetVids(w,u);
    endedge.SetRank(0);

    //then starting form the ending point of startedge
    e = startedge;
    while(1) {
      u = e.GetFirstVid();
      v = e.GetSecondVid();
      if (e == endedge) { 
        totaltour.push_back(endedge); break; 
      }
      if (! (tree.IsVertex(v, &vi) )  )
        ;
      for (ei=vi->edgelist.begin(); ei!=vi->edgelist.end(); ++ei) {
        if (ei->vertex2id == u) break;
      }
      if ((++ei) != vi->edgelist.end() ) //not the last 
	w = ei->vertex2id; //then the right neighbor
      else  //the last
	w = vi->edgelist.begin()->vertex2id; //then the first
      nexte.SetVids(v, w);
      e.SetVids(u, v);
      e.SetRank(edgecounter);
      totaltour.push_back(e);
      e = nexte;
      edgecounter--;
    } 
  }

  void GenLineTree(int n, TREE& g) {
    //n = number of vertices
    //generated graph is 0-1-2-3-...-(n-1)
    Task data;
    /* for sET, nprocs must be 1.
     */
    for (int i=0; i<n; i++) {
      data.SetWeight(i);
      g.AddVertex(data,i);
    }
    //g.DisplayGraph();
    VID u,v;
    for (int i=0; i<n-1; i++) {
      u=i; v=u+1;
      g.AddEdge(u,v,u*v);
    }
    stapl::rmi_fence();
    //g.DisplayGraph();
  }

  bool correctcheck(TREE& ptree, vector<euleredge> totaltour, int v_num, VID startvid) {
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
    //stapl_print("Complete checking!\n");
    return true;
  }
public:
  Tester_sET() {flag='p'; v_num=0; startvid=-1;}
  Tester_sET(int _v_num) {flag='p'; v_num=_v_num; startvid=-1;}
  Tester_sET(int _v_num, VID _startvid) {flag='p'; v_num=_v_num; startvid=_startvid;}

  char* GetName() { return "Tester_sET";}

  void ShowUsage() {
    stapl_print("\n"); stapl_print(GetName()); stapl_print(" p|c|b"); stapl_print(" #vertices startvid\n");
  }

  ETTester<TREE> *CreateCopy() { Tester_sET *t = new Tester_sET; t->flag=this->flag; t->v_num=this->v_num; t->startvid=this->startvid; return t; }

  void ParseParams(char* s) {
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
    ElapsedTimes = *(new vector<vector<double> >(testno));
  }

  void Test(TREE& tree) {
    counter = 0;

    MethodNames.push_back("sET_Tester");
    GenLineTree(v_num, tree);
    
    vector<euleredge> totaltour;

    if_want_perf {starttimer;}
    sET(tree,totaltour);
    if_want_perf {newstoptimer;}

    if_want_corr {
      bool correctflag = correctcheck(tree, totaltour, v_num, startvid); 
      if (correctflag) ;//cout<<"Succeed!"<<endl;
      else cout<<"Fail!"<<endl;
    } 
    return;
  }

};



