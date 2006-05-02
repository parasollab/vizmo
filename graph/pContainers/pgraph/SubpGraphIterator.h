#ifndef _Sub_pGraph_Iterator_h_
#define _Sub_pGraph_Iterator_h_

#include <vector>
#include <list>
#include <map>
#include <iterator>
#include <algorithm>
#include <pGraph.h>
#include <runtime.h>
#define WHITE 0
#define GRAY 1
#define BLACK 2

template<class SubPGRAPH>
class SubpGraphIterator 
  : public iterator<forward_iterator_tag, 
                         typename SubPGRAPH::PGRAPH_TYPE::value_type>
{
private:
  SubPGRAPH *spg;
  VID vid;
  bool _END;

public:

  typedef iterator<forward_iterator_tag,
                        typename SubPGRAPH::PGRAPH_TYPE::value_type> base_type;

  typedef typename base_type::iterator_category iterator_category;
  typedef typename base_type::value_type        value_type;
  typedef typename base_type::difference_type   difference_type;
  typedef typename base_type::pointer           pointer;
  typedef typename base_type::reference         reference;

  void define_type(stapl::typer& t) {
    t.dynamic(spg);
    t.local(vid);
    t.local(_END);
  }

  SubpGraphIterator()
  {  spg=NULL;  vid=0;  _END=false;  }
  SubpGraphIterator(SubPGRAPH *_spg)
  {  spg=_spg;  vid=0;  _END=false;  }

  void SetVID(VID _v)  {  vid=_v;  }
  void SetEnd(bool _end)  { _END=_end; }

  
  bool GetEnd()  { return _END; } 
  VID GetVID()  {  return vid; }

  SubpGraphIterator<SubPGRAPH> operator++()  {
    *this=spg->_next_vertex_(); 
    return *this;  
  }
  
  SubpGraphIterator<SubPGRAPH> operator++(int _dummy_) {
    SubpGraphIterator<SubPGRAPH> tmp = *this;
    ++*this;
    return tmp;
  }

  // VID operator *() { return vid; }

  inline
  bool operator == (SubpGraphIterator<SubPGRAPH> second) const
  {  return ( (spg==second.spg) && (vid==second.vid) &&(_END==second._END) );}    
  inline
  bool operator != (SubpGraphIterator<SubPGRAPH> second) const
  {  return !( (spg==second.spg) && (vid==second.vid) &&(_END==second._END) );}

  inline
  reference operator*() const {
    typename SubPGRAPH::PGRAPH_TYPE::VI vit;
    spg->getGraph()->IsVertex(vid, &vit);
    return vit->data;
  }

  inline
  pointer operator->() const {
    typename SubPGRAPH::PGRAPH_TYPE::VI vit;
    spg->getGraph()->IsVertex(vid, &vit);
    return &(vit->data);
  }

};



template<class PGRAPH>
class SubpGraph
{
private:
  PGRAPH * pg; //Pgraph corresponding to this iterator
  
//  vector<pair<VID,VID> > boundary;   //Boundary information of the pGraph.
                //Set of all edges that cross the boundary of the subgraph of pGraph
  hash_multimap<VID, VID, hash<VID> > boundary;


  hash_map<VID, int, hash<VID> > color;  //Used in doing a BFS in the SubGraph;
  list<VID> q; 
  bool processing_edge_list;
  VID v1id;
    typename PGRAPH::VI v1;
    typename PGRAPH::EI ei;

  VID startVID;  //The start VID where the BFS execution is started

  VID _dummy_vid_;  
public:

  void define_type(stapl::typer& t) {
    t.dynamic(pg);
    t.local(boundary);
    t.local(color);
    t.local(q);
    t.local(processing_edge_list);
    t.local(v1id);
    t.local(startVID);
    t.local(_dummy_vid_);
  }

  typedef PGRAPH PGRAPH_TYPE;

  SubpGraph()
    : color(), q(), _dummy_vid_(-100), pg(NULL), ei(NULL),
      processing_edge_list(false) 
  {
    startVID = _dummy_vid_;
    v1id = _dummy_vid_;
    stapl_assert(1,"SubpGraph constructor called.\n");
  }

  SubpGraph(const SubpGraph<PGRAPH>& other) {
    pg = other.pg;
    boundary = other.boundary;
    color = other.color;
    q = other.q;
    processing_edge_list = other.processing_edge_list;
    v1id = other.v1id;
    v1 = other.v1;
    ei = other.ei;
    startVID = other.startVID;
    _dummy_vid_ = _dummy_vid_;
  }

  SubpGraph<PGRAPH>& operator =(const SubpGraph<PGRAPH>& other) {
    if (&other != this) {
      pg = other.pg;
      boundary = other.boundary;
      color = other.color;
      q = other.q;
      processing_edge_list = other.processing_edge_list;
      v1id = other.v1id;
      v1 = other.v1;
      ei = other.ei;
      startVID = other.startVID;
      _dummy_vid_ = _dummy_vid_;
      return *this;
    } else {
      return *this;
    }
  }

  SubpGraph(PGRAPH * _pg, const hash_multimap<VID, VID, hash<VID> >& _bdry,
            VID _stV)
    : color(), q(), boundary(_bdry), ei(NULL)
  {
    pg = _pg;
    startVID = _stV;
    processing_edge_list = false;
    _dummy_vid_ = -100;
    v1id = -100;
  }

  PGRAPH* getGraph() { return pg; }

  SubpGraphIterator< SubpGraph<PGRAPH> > begin()
  {
    if (!color.empty())
      color.clear();
    SubpGraphIterator< SubpGraph<PGRAPH> > element(this);
    element.SetVID(_begin_boundary_traversal());
    return element;
  }

  VID _begin_boundary_traversal()
  {  
    if (!q.empty())
      q.clear();

    if ((pg != NULL) && ( pg->IsVertex(startVID) ))
    {
      q.push_back(startVID);
      color[startVID] = GRAY;
      processing_edge_list=false;
      return startVID;
    } 
    else 
    {
      cout << "\nIn SubpGraphIterator: root vid=" << startVID << "not in pgraph";
      return -1; 
    }
  }

  SubpGraphIterator< SubpGraph<PGRAPH> > _next_vertex_()
  {
    SubpGraphIterator< SubpGraph<PGRAPH> > element(this);
    element.SetVID(_find_next_vid());
    if(q.empty())
        element.SetEnd(true);    
    return element;
  }
  
  VID _find_next_vid()
  {
    bool found=false;
    VID foundVID=_dummy_vid_;


    while((!found)&&(!q.empty()))
    {
      if(!processing_edge_list)
      {

        v1id=q.front();
        if(!(pg->IsVertex(v1id,&v1)))
        {  

          cout<<" In SubpGraphIterator[" << stapl::get_thread_id() << "]::Next v1id "<<v1id<<" not in pGraph \n";
          exit(-1);
        }
        ei=v1->edgelist.begin();
        processing_edge_list=true;

      }
    
      if ((ei!=v1->edgelist.end())  && !(ei == typename PGRAPH::EI(NULL)))
      {
      

        VID v2id=ei->vertex2id;


          hash_map<VID, int, hash<VID> >::iterator vertex_it = color.find(v2id);
          if (vertex_it == color.end()) {
            pair<hash_multimap<VID, VID, hash<VID> >::iterator,
                 hash_multimap<VID, VID, hash<VID> >::iterator> edges = boundary.equal_range(v1id);
            bool on_boundary = false;
            for (hash_multimap<VID, VID, hash<VID> >::iterator i = edges.first; i != edges.second; ++i) {
              if (i->second == v2id)
                on_boundary = true;
            }
            if(!on_boundary)
            {  //We have not hit the boundary
              q.push_back(v2id);
              color.insert(pair<VID, int>(v2id, GRAY));
              found=true;
              foundVID=v2id;
            }
            else
            {
              //Boundary edge dont do anything
            }
          
          }
          else if(vertex_it->second == GRAY)  
          {//back edge  
          }
          else if(vertex_it->second == BLACK)
          {//front edge
          }
          else
          {  cout<<" Unknown color in SubpGraphIterator::Next... exiting...\n";
            exit(-1);
          }
        ei++;
      }
      else
      {
        //Finished processing the edge list
        if (processing_edge_list)
          color[v1id]=BLACK;
        q.pop_front();
        processing_edge_list=false;
      }  
        
    }
  

    return foundVID;
  }

  SubpGraphIterator< SubpGraph<PGRAPH> > end()
  {
    SubpGraphIterator< SubpGraph<PGRAPH> > element(this);
                element.SetVID(_dummy_vid_);
    element.SetEnd(true);
                return element;
  }

};

#endif
