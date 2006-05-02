/*!
  \file pMatrixBCD.h
  \author Gabriel Tanase	
  \date Mar. 23, 04
  \ingroup stapl
  \brief pMatrix class with block cyclic distribution

  Internal to stapl developer. 
*/
#ifndef PMATRIXBCD_H
#define PMATRIXBCD_H
#include <vector>
#include "BasePContainer.h"
#include "mtl/mtl.h"
#include "MatrixPart.h"
#include "../distribution/MatrixBlockCyclicDistribution.h"
#include "valarray"
#include "pMatrix.h"
template <class T>
class _MVMultiply_add{
 public:
  _MVMultiply_add(){}
  int operator()(T& data,T& arg){
    data += arg;
    return OK;
  }
  void define_type(stapl::typer &t){}
};
/**
 * @ingroup pContainers
 * @defgroup pmatrix Parallel Matrix
 * @{
 * 
 **/
namespace stapl {
  /**
   *pMatrixBCD class definition
   *pMatrixBCD implements the parallel matrix container. There are two templates that has 
   *to be specified at the instantiation: 
   * T the type of elements of the pMatrixBCD
   * MatrixOrientation the tag defining the orientation of the matrix.
   * \nosubgrouping 
   */
  template<class T,class MatrixOrientation = stapl_row_major>
    class pMatrixBCD : 
    public BasePContainer<MatrixPart<T,mtl::rectangle<>,mtl::dense<>,typename MatrixOrientation::stapl_matrix_orientation>, 
    MatrixBlockCyclicDistribution,no_trace, random_access_iterator_tag, stapl_element_tag > {

      public:
      //=======================================
      //types
      //=======================================
      ///The type of the elements of the pMatrixBCD.
      typedef T value_type;

      //required by mtl; not used right now by stapl
      typedef mtl::rectangle<> Shape;
      typedef mtl::dense<>     Storage;

      ///Matrix orientation.
      typedef typename MatrixOrientation::stapl_matrix_orientation Orientation;

      typedef typename MatrixPart<value_type,Shape,Storage,Orientation>::Element_Set_type Matrix_Range_type;
      ///Distribution defines
      typedef MatrixBlockCyclicDistribution Distribution;
  
      ///Matrix Part.
      typedef  MatrixPart<value_type,Shape,Storage,Orientation> pContainerPart_type;
      ///BasePContainer. The base class for pMatrixBCD.
      typedef BasePContainer<MatrixPart<value_type,Shape,Storage,Orientation>, Distribution,no_trace, random_access_iterator_tag, stapl_element_tag  > BasePContainer_type;

      //Iterator over the parts.
      typedef typename BasePContainer_type::parts_internal_iterator parts_internal_iterator;

      ///Matrix distribution info class.
      typedef typename Distribution::DistributionInfo_type MatrixDistributionInfo_type;

      ///Iterator over the elements of the matrix distribution
      typedef typename Distribution::DistributionInfo_iterator DistributionInfo_iterator;

      ///Part type.
      typedef MatrixPart<T,Shape,Storage,Orientation>     Part_type;
      typedef typename MatrixPart<T,Shape,Storage,Orientation>::iterator part_iterator;

      typedef pair<Location,Location>                     Partbdryinfo_type;
      typedef pMatrixBCD<T,MatrixOrientation>                this_type;

      ///Iterator over the rows/columns of the matrix depending on the orientation.
      typedef typename BasePContainer_type::iterator                iterator;
      ///Const iterator over the rows/columns of the matrix depending on the orientation.
      typedef typename BasePContainer_type::const_iterator          const_iterator;

      ///Iterator over the elements of a row or a column(depending on the orientation of the matrix).
      typedef typename Part_type::element_iterator        element_iterator;
      ///Const iterator over the elements of a row or a column(depending on the orientation of the matrix).
      typedef typename Part_type::const_element_iterator  const_element_iterator;

      ///Rows number.
      unsigned int M;

      ///Columns number.
      unsigned int N;

      private:
      //local parray used for matrix algorithms
      pArray<T> larray;
      bool done_flag;
      pair<unsigned int, unsigned int> layout;
      vector<bool> flagsA12,flagsA21;
      vector<Part_type*> A12,A21;

      public:
      //=======================================
      //constructors 
      //=======================================
       /**@name Constructors and Destructor */
      //@{
      /**@brief  Default Constructor
       */
      pMatrixBCD() {
        this->register_this(this);
        rmi_fence(); 
      }

      /**@brief  Specific Constructor; A default processor layout is chosen
       *@param unsigned int _m the number of rows.
       *@param unsigned int _n the number of columns.
       */
      pMatrixBCD(unsigned int _m, unsigned int _n, int nrp=-1, int ncp = -1){
	int i,j;
	int lnrp,lncp,partid=0;
	int bP=0,bQ=0;
	Location l;
	pair<unsigned int, unsigned int> _current;
	M=_m; N=_n;
	layout = MatrixOrientation().GetProcessorsLayout();
	
	if(nrp == -1){
	  nrp = _m / layout.first; //number rows per processor
	}
	if(ncp == -1){
	  ncp = _n / layout.second;//number columns per processor
	}
	stapl_assert(nrp <= _m,"pMatrix Constructor: ERROR while specifying nrp");
	stapl_assert(ncp <= _n,"pMatrix Constructor: ERROR while specifying ncp");
	//now all procs will allocate the blocks that are local to them
	for(i=0;i<this->pcontainer_parts.size();++i){
	  delete this->pcontainer_parts[i];
	}
	this->pcontainer_parts.clear();

	pid2ij(this->myid,_current);
	i=_current.first;
	while(i*nrp < _m){
	  ++bP;
	  lnrp = ((i+1)*nrp < _m)?nrp:(_m - i*nrp);
	  j=_current.second;
	  bQ = 0;
	  while(j*ncp < _n){
	    ++bQ;
	    lncp = ((j+1)*ncp < _n)?ncp:(_n - j*ncp);
	    //allocate block i,j
	    //cout<<"Allocate part:"<<endl;
	    //cout<<"   "<<lnrp<<" "<<lncp<<" "<<i*nrp<<" "<<j*ncp<<endl;
	    Part_type *p = new Part_type(lnrp,
					 lncp,
					 i*nrp,
					 j*ncp,
					 partid++);
	    this->pcontainer_parts.push_back(p);
	    j+=layout.second;
	  }
	  i+=layout.first;
	}
	this->register_this(this);
        this->dist.InitializeMatrixBlockCyclicDistribution(_m,_n,nrp,ncp,bP,bQ,layout);
        stapl::rmi_fence();
      }
      
      /**@brief  Constructor where the user can specify the processors layout.
       *@param _m the rows number.
       *@param _n the columns number.
       *@param _layout the processor layout.
       */
      pMatrixBCD(unsigned int _m, 
		 unsigned int _n,
		 const pair<unsigned int, unsigned int>& _layout,
		 int nrp=-1, int ncp = -1){
		   //....
      }
      
      /**@brief  Copy constructor 
       */
      pMatrixBCD(const pMatrixBCD& _other) : BasePContainer_type(_other) {
	M = _other.M;
	N = _other.N;
	layout = _other.layout;
        this->register_this(this);
        rmi_fence();
      }

      /**@brief  
       *Destructor
       */     
      ~pMatrixBCD(){
        stapl::unregister_rmi_object( this->getHandle());
      }

      /**@brief 
       * Initialize the data of the pMatrixBCD; The functor will be called for every index of
       * the parray and the result will be stored in the element corresponding
       * to the index e.g., {parray[index] = functor(index)}.
       *@param Function user specified function.
       */
      template<class Function>
      void initialize(Function gen_fun){
	unsigned int li,lj,I,J,nColumns,nRows;
	Part_type* part;
	part_iterator pi;
	//for all parts
	for (int i=0; i < this->GetPartsCount();++i){
	  I = this->GetPart(i)->GetI();
	  J = this->GetPart(i)->GetJ();
	  nRows = this->GetPart(i)->GetnRows();
	  nColumns = this->GetPart(i)->GetnColumns();
	  part = this->GetPart(i);
	  for (li=I;li<I+nRows;++li){
	    for (lj=J;lj<J+nColumns;++lj){
	      gen_fun((*part)(li-I,lj-J),li,lj);
	    }
          }
	}
      }


      void Display(){
	unsigned int li,lj,I,J,nColumns,nRows;
	Part_type* part;
	part_iterator pi;
	//for all parts
	for (int i=0; i < this->GetPartsCount();++i){
	  I = this->GetPart(i)->GetI();
	  J = this->GetPart(i)->GetJ();
	  nRows = this->GetPart(i)->GetnRows();
	  nColumns = this->GetPart(i)->GetnColumns();
	  part = this->GetPart(i);
	  cout<<"-"<<this->myid<<" "<<I<<" "<<J<<" "<<nRows<<" "<<nColumns<<" "<<part<<endl;
	  for (li=I;li<I+nRows;++li){
	    for (lj=J;lj<J+nColumns;++lj){
	      cout<<(*part)(li-I,lj-J)<<" ";
	    }
          }
	}
      }

      /**@brief 
       *Define type for pMatrixBCD.
       */
      void define_type(typer &t)  {
        cout<<"ERROR::Method define_type not yet implemented for pMatrixBCD"<<endl;
      }

      //@}

      //protected:
      inline 
      void pid2ij(unsigned int _pid, pair<unsigned int, unsigned int>& _p){
	_p.first = _pid / layout.second;
	_p.second = _pid % layout.second;
      }

      inline 
      void ij2pid(unsigned int& _pid, const pair<unsigned int, unsigned int>& _p){
	_pid = _p.first * layout.second + _p.second;
      }
      
      unsigned int getLeft(){
	unsigned int i,j;
	i = this->myid / layout.second;
	j = this->myid % layout.second;
	cout<<"["<<myid<<"|"<<i<<":"<<j<<"]"<<endl;
	j = (j + layout.second - 1) % layout.second;
	return i*layout.second + j;
      }
			    
      unsigned int getRight(){
	unsigned int i,j;
	i = this->myid / layout.second;
	j = this->myid % layout.second;
	cout<<"["<<myid<<"|"<<i<<":"<<j<<"]"<<endl;
	j = (j + 1) % layout.second;
	return i*layout.second + j;
      }

      unsigned int getUp(){
	unsigned int i,j;
	i = this->myid / layout.second;
	j = this->myid % layout.second;
	cout<<"["<<myid<<"|"<<i<<":"<<j<<"]"<<endl;
	i = (i + layout.first - 1) % layout.first;
	return i*layout.second + j;
      }

      unsigned int getDown(){
	unsigned int i,j;
	i = this->myid / layout.second;
	j = this->myid % layout.second;
	cout<<"["<<myid<<"|"<<i<<":"<<j<<"]"<<endl;
	i = (i + 1) % layout.first;
	return i*layout.second + j;
      }

      public:

      size_t max_size() {return size_t(-1);}
            
      /**@name Set/Get data */
      //@{

      /**@brief 
       *Assignment operator.
       */
      this_type& operator=(const this_type& _other) {
	M = _other.M;
	N = _other.N;
	layout = _other.layout;
	BasePContainer_type::operator=(_other);
	return *this;
      }
      
      //=======================================
      // Set/Get/Update methods;
      //=======================================      
      /**@brief Parenthesis operator should only be called for local elements if it is in right-hand-side.
       *if need to modify a remote element, use SetElement(GID,Data);
       *@param unsigned int _i row id.
       *@param unsigned int _j column id.
       *@return const value_type the value of the element (_i,_j).
      */
      const value_type operator()(unsigned int _i, unsigned int _j) {
        return this->GetElement(_i , _j);
      }
      
      protected:
      virtual const value_type _GetElement(_StaplTriple<unsigned int,unsigned int,PARTID> _args){
	Part_type  *p;
	if(_args.third != INVALID_PART) {
	  p = this->GetPart(this->dist.GlobalBlockIdToLocal(_args.third));
	  //p = this->GetPart(_args.third);
	  value_type t = (*p)(_args.first - p->GetI(), _args.second - p->GetJ());
	  return t;
	}
	else {
	  //throw Invalid_Part();
	  return value_type();
	}
      }

      public:

      /**@brief 
       *Get the value of an element of the matrix.
       *@param unsigned int _i row id.
       *@param unsigned int _j column id.
       *@return const value_type the value of the element (_i,_j).
      */      
      const value_type GetElement(unsigned int _i, unsigned int _j) {
	PARTID partid;
	Part_type  *p;
	if(this->dist.IsLocal(_i,_j, partid)){
	  p = GetPart(partid);
	  //translate _i, _j to local i local j
	  value_type t = (*p)(_i - p->GetI() ,_j - p->GetJ());
	  return t;
	}
	else{
	  value_type rtn;		
	  Location loc = this->dist.Lookup(_i,_j);
	  stapl_assert(loc.ValidLocation(),"Invalid location for get element");    
	  _StaplTriple<unsigned int,unsigned int,PARTID> args(_i,_j,loc.partid());
	  rtn=stapl::sync_rmi(loc.locpid(),getHandle(),
			      &this_type::_GetElement,args);
	  //printf("\n Get element %d location (%d,%d) return %d", _gid,loc.part_id,loc.pid,rtn);
	  return rtn;
	}
      }
      
      
      protected:
    
      void _SetElement(const _StaplQuad<unsigned int, unsigned int,PARTID, value_type>& _args) {
	Part_type  *p;
	if(_args.third != INVALID_PART) {
	  p = GetPart(this->dist.GlobalBlockIdToLocal(_args.third));
	  //p = GetPart(_args.third);
	  (*p)(_args.first - p->GetI() ,_args.second - p->GetJ()) = _args.fourth;
	  //return OK; 
	}
	//return ERROR;
      }
    
      public:

      /**@brief 
       *Set the value of an element of the matrix.
       *@param unsigned int _i row id.
       *@param unsigned int _j column id.
       *@return const value_type the value of the element (_i,_j).
      */          
      void SetElement(unsigned int _i, unsigned int _j, const value_type& _t){
	PARTID partid;
	if(this->dist.IsLocal(_i,_j,partid)){
	  Part_type  *p = GetPart(partid);
	  (*p)(_i - p->GetI(), _j - p->GetJ()) = _t;
	}
	else{			
	  Location loc = dist.Lookup(_i,_j);
	  stapl_assert(loc.ValidLocation(),"Invalid location for get element");    
	  stapl::async_rmi(loc.locpid(),getHandle(),
			  &this_type::_SetElement,
			  _StaplQuad<unsigned int, unsigned int,PARTID, value_type>(_i,_j,loc.partid(),_t));
	}
      }
      //@}

      //=========================================================
      //algorithms provided by the pMatrixBCD Class
      //first simple operators
      //=========================================================      
      /**@name Numeric algorithms */
      //@{
      
      /**@brief 
       *Add a second matrix to the current one.
       *@param this_type& reference to the second matrix.
       */
      void Add(this_type& B){
	//add this to B and save it in B
	for(int i=0;i<this->pcontainer_parts.size();++i){
	  mtl::add(B.pcontainer_parts[i]->part_data,this->pcontainer_parts[i]->part_data);
	}
      }

      /**@brief 
       *Subtract a second matrix from the current one.
       *@param this_type& reference to the second matrix.
       */
      void Subtract(this_type& B){
	//add this to B and save it in B
	for(int i=0;i<this->pcontainer_parts.size();++i){
	  mtl::add(scaled(B.pcontainer_parts[i]->part_data,-1),this->pcontainer_parts[i]->part_data);
	}
      }
      
      /**@brief 
       *Scale all the elements of the matrix with a value T
       *@param T& reference to the element to which all the elements 
       *of the matrix will be multiplied.
       */
      void Scale(T& _t){
	for(int i=0;i<this->pcontainer_parts.size();++i){
	  mtl::scale(this->pcontainer_parts[i]->part_data,_t);
	}
      }

      protected:
      void _shiftBufferMatrixVectorMultiply(typename pArray<T>::Part_type& _temp){
	larray.SetPart(0 , _temp);
	done_flag = true;
      }

      public:
      /**@brief 
       *Matrix array multiplication;
       *@param pArray<T>& _source source pArray.
       *@param pArray<T>& _dest destination pArray.
       */
      void Multiply(pArray<T>& _source, pArray<T>& _dest){
	typename pArray<T>::Part_type::iterator ai, di;
	typename this_type::iterator mi, mbegin = this->local_begin();
	typename this_type::iterator mend = this->local_end();
	element_iterator eit,eiend;
	T tmp;

	//thread id to whom I sent and from which I receive
	int from,to;
	to = (this->myid + 1) % this->nprocs;
	if(this->myid == 0) from = this->nprocs - 1;
	else from = this->myid - 1; 

	//local index start and local index stop
	int listart,listop;
	int iter_count = this->nprocs;

	while (iter_count > 0){
	  // cout<<"iteration"<<iter_count<<endl;
	  //decide from where to where inside pmatrix to multiply
	  listart = _source.GetPart(0)->start_index;
	  listop = listart + _source.GetPart(0)->size();

	  //multiply
	  
	  //di = _dest.local_begin();
	  di = _dest.GetPart(0)->begin();
	  for(mi = mbegin;mi != mend;++mi,++di){
	    ai = _source.GetPart(0)->begin();;
	    eiend = (*mi).begin() + listop;
	    tmp = *di;
	    for(eit = (*mi).begin()+listart;eit != eiend; ++eit,++ai){
	      tmp += (*eit) * (*ai);
	    }
	    *di = tmp;
	  }
	  //shift the source array 
	  done_flag = false;
	  stapl::async_rmi(to,
			   getHandle(),
			   &this_type::_shiftBufferMatrixVectorMultiply,*(_source.GetPart(0)));
	  stapl::rmi_fence();

	  while(done_flag == false) stapl::rmi_poll();
	  _source.SetPart(0 , *(larray.GetPart(0))); 
	  
	  //stapl::rmi_fence();
	  iter_count--;
	}
      }


      protected:

      void _shiftBufferMatrixVectorMultiplyBCD(typename pArray<T>::Part_type& _temp){
	//larray.SetPart(0 , _temp);
	larray.GetPart(0)->SetArrayPart(_temp.part_data,0,_temp.start_index);
	done_flag = true;
      }

      public:
      /**@brief 
       *Matrix array multiplication;
       *@param pArray<T>& _source source pArray.
       *@param pArray<T>& _dest destination pArray.
       */
      void MultiplyBCD(pArray<T>& _source, pArray<T>& _dest){
	typedef typename Part_type::Container_type DenseMatrix;
	typedef valarray<T> ARRAY;

	typename pArray<T>::Part_type::iterator ai, di;
	typename Part_type::iterator mi,mbegin , mend;

	//typename this_type::iterator mi, mbegin = this->local_begin();
	//typename this_type::iterator mend = this->local_end();

	element_iterator eit,eiend;
	T tmp;

	//thread id to whom I sent and from which I receive
	int from,to,lineno=0;
	to = (this->myid + 1) % this->nprocs;
	if(this->myid == 0) from = this->nprocs - 1;
	else from = this->myid - 1; 
	
	ARRAY temp_dest(T(0),this->M);
	//local index start and local index stop
	int k,i_start,i_stop,minj,maxj,l_i_start,l_i_stop;
	int iter_count = this->nprocs;

	while (iter_count > 0){
	  // cout<<"iteration"<<iter_count<<endl;
	  //decide from where to where inside pmatrix to multiply
	  i_start = l_i_start = _source.GetPart(0)->start_index;
	  i_stop  = l_i_stop = l_i_start + _source.GetPart(0)->size();
	  //multiply
	  //for all parts
	  for(parts_internal_iterator it = pcontainer_parts.begin(); 
	      it !=pcontainer_parts.end(); 
	      ++it) {
	    
	    Part_type* lp = *it;
	    i_start = l_i_start;
	    i_stop  = l_i_stop;
	    //if this columns intersects the pArray compute the intersection
	    minj = lp->GetJ();
	    maxj = minj + lp->GetnColumns();
	    if(minj > l_i_start) i_start = minj;//intersection start
	    if(maxj < l_i_stop) i_stop = maxj;//intersection stop
	    if(i_start > l_i_stop || i_stop <= l_i_start) continue;//the intersection is null
	    if(i_start > maxj  || i_stop <= minj) continue;//the intersection is null
	    //here the intersection was not null	    

	    mbegin = lp->part_data.begin();
	    mend = lp->part_data.end();
	    k=0;
	    for(mi = mbegin;mi != mend;++mi,++k){
	      tmp = temp_dest[lp->GetI() + k];
	      ai = _source.GetPart(0)->begin() + i_start - l_i_start;
	      eiend = (*mi).begin() + i_stop - minj;
	      //tmp = *di;
	      for(eit = (*mi).begin()+i_start-minj;eit != eiend; ++eit,++ai){
		tmp += (*eit) * (*ai);
	      }
	      temp_dest[lp->GetI() + k] = tmp;
	    }
	    /*
	    for (typename pArray<T>::iterator itd=_dest.local_begin(); itd != _dest.local_end(); ++itd){
	      cout<<*itd<<" ";
	    }
	    cout<<endl;  
	    */

	  }
	  //shift the source array 
	  done_flag = false;
	  stapl::async_rmi(to,
			   getHandle(),
			   &this_type::_shiftBufferMatrixVectorMultiplyBCD, *(_source.GetPart(0)));
	  stapl::rmi_fence();

	  while(done_flag == false) stapl::rmi_poll();
	  _source.GetPart(0)->SetArrayPart(larray.GetPart(0)->part_data,0,larray.GetPart(0)->start_index);
	    //_source.SetPart(0 , *(larray.GetPart(0))); 
	  
	  //stapl::rmi_fence();
	  iter_count--;
	}
	//here we have to reduce temp_dest into _dest
	/*
	_MVMultiply_add<T> adder;
	l_i_start = _dest.GetPart(0)->start_index;
	l_i_stop = l_i_start + _dest.GetPart(0)->part_data.size();
	di = _dest.GetPart(0)->part_data.begin(); 
	for (k=l_i_start;k < l_i_stop->M;++k){
	  *di += temp_dest[]
	}
	*/
      }
      ///////////////////////////////////////////////////////////////////////////
      //   MATRIX MATRIX multiply
      ///////////////////////////////////////////////////////////////////////////

      protected:
      void _shiftBufferMatrixMatrixMultiply(_StaplQuad<unsigned int, unsigned int, unsigned int, unsigned int>& _args, vector<T>& _temp){
	
	Part_type* p = new Part_type(_args.first, _args.second, _args.third, _args.fourth,0);
	p->SetPart(_temp);
	delete (this->GetPart(0)) ;
	this->SetPart(0,p);
	done_flag = true;
      }

      public: 
      void SimpleLeftShift(){
	//thread id to whom I sent and from which I receive
	int from,to;
	Part_type *p = this->GetPart(0);
	to = (this->myid + this->nprocs + 1) % this->nprocs;
	//local index start and local index stop
	//cout<<this->myid<<"sends to "<< to <<endl;	
	vector<T> temp = *(p->GetVectorData());
	_StaplQuad<unsigned int, unsigned int, unsigned int, unsigned int> _args(p->GetnRows(),p->GetnColumns(),p->GetI(),p->GetJ());
	stapl::rmi_fence();
	stapl::async_rmi(to,getHandle(),&this_type::_shiftBufferMatrixMatrixMultiply,_args,temp);
	stapl::rmi_fence();
      }

      public:
      /**@brief 
       *Matrix matrix multiplication; C = this * B
       *The matrix B has to be column oriented while this and B has to be 
       *row oriented.
       *@param TB& Matrix B
       *@param this_type& Matrix C
       */
      template <class TB>
      void Multiply(TB& _BB, this_type& _CC){
	
	int iter_count = this->nprocs;
	unsigned int row_from, row_to, col_from, col_to;
	Part_type *pA,*pC;
	typename TB::Part_type *pB;
	pA = this->GetPart(0);
	pC = _CC.GetPart(0);
	while (iter_count > 0){
	  //cout<<"iteration"<<iter_count<<endl;
	  pB = _BB.GetPart(0);
	  row_from = pB->GetI();
	  row_to = row_from + pB->GetnRows();
	  col_from = pB->GetJ();
	  col_to = col_from + pB->GetnColumns();

	  //if(this->myid == 2){
	  //cout<<"iteration"<<iter_count<<"cols from:"<<col_from<<"to"<<col_to<<endl;
	  //mtl::print_all_matrix(pB->part_data);
	  //}

	  mtl::mult(pA->part_data,pB->part_data,pC->part_data.sub_matrix(row_from,row_to,col_from,col_to));
	  //shift matrix B to the left
	  if(this->nprocs > 1)_BB.SimpleLeftShift();
	  iter_count--;

	  //stapl::rmi_fence();
	  //if(this->myid == 0){
	  //for (int i = 0; i < 10; ++i) {
	  //  for (int j = 0; j < 10; ++j) {
	  //cout<<_CC.GetElement(i,j)<<" ";
	  //  }
	  //  cout<<endl;
	  //}
	  //}
	  //stapl::rmi_fence();
	}
      }      


      ///////////////////////////////////////////////////////////////////////////
      //   MATRIX MATRIX multiply for 2D layouts
      ///////////////////////////////////////////////////////////////////////////
      public:
      void _shiftBufferMatrixMatrixMultiply2D(_StaplQuad<unsigned int, unsigned int, unsigned int, unsigned int>& _args, vector<T>& _temp){
	Part_type* p = new Part_type(_args.first, _args.second, _args.third, _args.fourth,0);
	p->SetPart(_temp);
	delete (this->GetPart(0)) ;
	this->SetPart(0,p);
	done_flag = true;
      }
     
      public: 
      template <class TB>
      void ShiftThisMatrixLeftwardAndBUpward(TB& B){
	vector<T> temp;
	vector<T> temp_b;
	pair<unsigned int, unsigned int> ij, ij_b, layout_b;
	unsigned int to;
	unsigned int to_b;
	pid2ij(this->myid, ij);
	ij_b = ij;
	layout_b = B.GetLayout();
	Part_type *p = this->GetPart(0);
	if(ij.first > 0){
	  ij.second = (ij.second + layout.second - ij.first)%layout.second;
	  temp = *(p->GetVectorData());
	  ij2pid(to,ij);
	}
	typename TB::Part_type *pb = B.GetPart(0);
	if(ij_b.second > 0){
	  ij_b.first = (ij_b.first + layout_b.first - ij_b.second) % layout_b.first;
	  temp_b = *(pb->GetVectorData());
	  ij2pid(to_b,ij_b);
	}
	_StaplQuad<unsigned int, unsigned int, unsigned int, unsigned int> _args(p->GetnRows(),p->GetnColumns(),p->GetI(),p->GetJ());
	_StaplQuad<unsigned int, unsigned int, unsigned int, unsigned int> _args_b(pb->GetnRows(),pb->GetnColumns(),pb->GetI(),pb->GetJ());
	stapl::rmi_fence();
	if(ij.first > 0){
	  stapl::async_rmi(to,getHandle(),&this_type::_shiftBufferMatrixMatrixMultiply2D,_args,temp);
	}
	if(ij_b.second > 0){
	  stapl::async_rmi(to_b,B.getHandle(),&TB::_shiftBufferMatrixMatrixMultiply2D,_args_b,temp_b);
	}
	temp.clear();
	temp_b.clear();
	stapl::rmi_fence();
      }


      template <class TB>
      void UndoShiftThisMatrixLeftwardAndBUpward(TB& B){
	vector<T> temp;
	vector<T> temp_b;
	pair<unsigned int, unsigned int> ij, ij_b, layout_b;
	unsigned int to;
	unsigned int to_b;
	pid2ij(this->myid, ij);
	ij_b = ij;
	layout_b = B.GetLayout();
	Part_type *p = this->GetPart(0);
	if(ij.first > 0){
	  ij.second = (ij.second + ij.first)%layout.second;
	  temp = *(p->GetVectorData());
	  ij2pid(to,ij);
	}
	typename TB::Part_type *pb = B.GetPart(0);
	if(ij_b.second > 0){
	  ij_b.first = (ij_b.first + ij_b.second) % layout_b.first;
	  temp_b = *(pb->GetVectorData());
	  ij2pid(to_b,ij_b);
	}
	_StaplQuad<unsigned int, unsigned int, unsigned int, unsigned int> _args(p->GetnRows(),p->GetnColumns(),p->GetI(),p->GetJ());
	_StaplQuad<unsigned int, unsigned int, unsigned int, unsigned int> _args_b(pb->GetnRows(),pb->GetnColumns(),pb->GetI(),pb->GetJ());
	stapl::rmi_fence();
	if(ij.first > 0){
	  stapl::async_rmi(to,getHandle(),&this_type::_shiftBufferMatrixMatrixMultiply2D,_args,temp);
	}
	if(ij_b.second > 0){
	  stapl::async_rmi(to_b,B.getHandle(),&TB::_shiftBufferMatrixMatrixMultiply2D,_args_b,temp_b);
	}
	temp.clear();
	temp_b.clear();
	stapl::rmi_fence();
      }

      void ShiftMatrixUpward(){	
	vector<T> temp;
	pair<unsigned int, unsigned int> ij;
	unsigned int to;

	pid2ij(this->myid, ij);
	Part_type *p = this->GetPart(0);
	if(ij.second > 0){
	  ij.first = (ij.first + layout.first - ij.second) % layout.first;
	  temp = *(p->GetVectorData());
	  ij2pid(to,ij);
	  //cout<<this->myid<<"sends to "<< to <<endl;
	}

	_StaplQuad<unsigned int, unsigned int, unsigned int, unsigned int> _args(p->GetnRows(),p->GetnColumns(),p->GetI(),p->GetJ());
	//local index start and local index stop
	stapl::rmi_fence();
	if(ij.second > 0){
	  stapl::async_rmi(to,getHandle(),&this_type::_shiftBufferMatrixMatrixMultiply2D,_args,temp);
	}
	stapl::rmi_fence();
      }


      void ShiftMatrixLeftward(){
	vector<T> temp;
	pair<unsigned int, unsigned int> ij;
	unsigned int to;

	pid2ij(this->myid, ij);
	cout<<"["<<ij.first<<"|"<<ij.second<<"]"<<endl;
	Part_type *p = this->GetPart(0);
	if(ij.first > 0){
	  ij.second = (ij.second + layout.second - ij.first)%layout.second;
	  temp = *(p->GetVectorData());
	  ij2pid(to,ij);
	}

	_StaplQuad<unsigned int, unsigned int, unsigned int, unsigned int> _args(p->GetnRows(),p->GetnColumns(),p->GetI(),p->GetJ());
	//local index start and local index stop
	stapl::rmi_fence();
	if(ij.first > 0){
	  stapl::async_rmi(to,getHandle(),&this_type::_shiftBufferMatrixMatrixMultiply2D,_args,temp);
	}
	stapl::rmi_fence();
      }

      void LeftShift2D(){
	vector<T> temp;
	pair<unsigned int, unsigned int> ij;
	unsigned int to;
	
	pid2ij(this->myid, ij);
	//cout<<"["<<ij.first<<"|"<<ij.second<<"]"<<endl;
	Part_type *p = this->GetPart(0);
	ij.second = (ij.second + layout.second - 1)%layout.second;
	//cout<<"-["<<ij.first<<"|"<<ij.second<<"]"<<endl;
	temp = *(p->GetVectorData());
	ij2pid(to,ij);
	//cout<<this->myid<<"sends to "<< to <<endl;
	_StaplQuad<unsigned int, unsigned int, unsigned int, unsigned int> _args(p->GetnRows(),p->GetnColumns(),p->GetI(),p->GetJ());
	//local index start and local index stop
	stapl::rmi_fence();
	stapl::async_rmi(to,getHandle(),&this_type::_shiftBufferMatrixMatrixMultiply2D,_args,temp);
	stapl::rmi_fence();
      }

      void UpShift2D(){
	vector<T> temp;
	pair<unsigned int, unsigned int> ij;
	unsigned int to;
	
	pid2ij(this->myid, ij);
	Part_type *p = this->GetPart(0);
	ij.first = (ij.first + layout.first - 1) % layout.first;
	temp = *(p->GetVectorData());
	ij2pid(to,ij);
	//cout<<this->myid<<"sends to "<< to <<endl;
	
	_StaplQuad<unsigned int, unsigned int, unsigned int, unsigned int> _args(p->GetnRows(),p->GetnColumns(),p->GetI(),p->GetJ());
	//local index start and local index stop
	stapl::rmi_fence();
	stapl::async_rmi(to,getHandle(),&this_type::_shiftBufferMatrixMatrixMultiply2D,_args,temp);
	stapl::rmi_fence();
      }
      
      public:
      /**@brief 
       *Matrix matrix multiplication; C = this * B
       *The matrix B has to be column oriented while this and B has to be 
       *row oriented.
       *@param TB& Matrix B
       *@param this_type& Matrix C
       */
      template <class TB>
      int Multiply2D(TB& _BB, this_type& _CC){
	int s;
	Part_type *pA,*pC;
	typename TB::Part_type *pB;
	pC = _CC.GetPart(0);
	if(layout.first != layout.second){
	  cout<<"Multiply2D works for sqrt(p) ~ integer"<<endl;
	  return -1;
	}	
	this->ShiftThisMatrixLeftwardAndBUpward(_BB);
	pB = _BB.GetPart(0);
	for(s=0;s<layout.first;++s){
	  pA = this->GetPart(0);
	  pB = _BB.GetPart(0);
	  mtl::mult(pA->part_data,pB->part_data,pC->part_data);
	  this->LeftShift2D();
	  _BB.UpShift2D();
	}
	this->UndoShiftThisMatrixLeftwardAndBUpward(_BB);
	return 1;
      }    

      ///////////////////////////////////////////////////////////////////////////
      //   MATRIX MATRIX multiply with BMR
      ///////////////////////////////////////////////////////////////////////////
      void broadcastMBR(int k, this_type& _TT){
	pair<unsigned int, unsigned int> ij, toij;
	unsigned int to;
	Part_type *p = this->GetPart(0);
	pid2ij(this->myid, ij);
	if(ij.second == (ij.first + k) % layout.first){
	  toij = ij;
	  for(int s =0;s < layout.first; ++s){
	    if(s != ij.second){
	      toij.second = s;
	      ij2pid(to,toij);
	      _StaplQuad<unsigned int, 
		unsigned int, 
		unsigned int, 
		unsigned int> _args(p->GetnRows(),
				    p->GetnColumns(),
				    p->GetI(),p->GetJ());
	      stapl::async_rmi(to,_TT.getHandle(),
			       &this_type::_shiftBufferMatrixMatrixMultiply2D,
			       _args,*(p->GetVectorData()));
	    }
	  }
	}
	stapl::rmi_fence();
      }

      template <class TB>
      int Multiply2DBMR(TB& _BB, this_type& _CC){
	int s;
	pair<unsigned int, unsigned int> ij;
	Part_type *pA, *pT,*pC;
	typename TB::Part_type *pB;
	pC = _CC.GetPart(0);	
	pA = this->GetPart(0);
	if(layout.first != layout.second){
	  cout<<"Multiply2DBMR works for sqrt(p) ~ integer"<<endl;
	  return -1;
	}
	pid2ij(this->myid, ij);
	this_type TT(*this);
	for(s=0;s<layout.first;++s){
	  broadcastMBR(s,TT);
	  pT = TT.GetPart(0);
	  pB = _BB.GetPart(0);
	  if(ij.second == (ij.first + s) % layout.first)
	    mtl::mult(pA->part_data,pB->part_data,pC->part_data);
	  else
	    mtl::mult(pT->part_data,pB->part_data,pC->part_data);	  
	  _BB.UpShift2D();	  
	}
	return 1;
      }

      //////////////////////////////////////////////////////////////////////
      protected:
      template <class MatrixRowsOperation>
      void _rowsOperation(valarray<T>& _temp, _StaplTriple<int,int,MatrixRowsOperation> _args){
	//cout<<"received on "<<this->myid<<endl;
	PARTID partid;
	if(dist.IsLocal(_args.first, _args.second,partid)){
	  part_iterator iti = this->GetPart(partid)->begin() + (_args.first - this->GetPart(partid)->GetI());
	  element_iterator eii,iend;
	  T* eij = &_temp[0];
	  iend = (*iti).end();
	  for(eii = (*iti).begin(); eii != iend;++eii,++eij){
	    *eii = _args.third(*eii,*eij);
	  }
	}
	else
	  cout<<"ERROR: while performing operations on rows in pMatrix"<<endl;
      }

      public:
      /**@brief Operation on two rows. Store the result in the first row;
       * rowi = func(rowi,rowj)
       *@param unsigned int _rowi first row.
       *@param unsigned int _rowj second row.
       *@param MatrixRowsOperation& _func user function that will be executed.
       */
      template <class MatrixRowsOperation>
      void RowsOperation(unsigned int _rowi, unsigned int _rowj, MatrixRowsOperation& _func){
	// !!!   Works for one part only; The logic gets complicated for multiple parts
	typedef valarray<T> ARRAY;
	typedef _StaplTriple<int,int,MatrixRowsOperation> ARGS;
	vector<PARTID> partidi,partidj;
	vector<unsigned int> firstjofi,firstjofj;
	part_iterator iti,itj;
	element_iterator eii, eij, iend,jend;
	vector<ARRAY> vtemp;
	ARRAY temp(1);
	int to = -1;//where to sent the row

	//cout<<"size of the part"<<this->GetPart(0)->GetVectorData()->size()<<endl;

	if(this->dist.IsLocalRow(_rowi,firstjofi,partidi)){
	  //if _rowi is local
	  if(this->dist.IsLocalRow(_rowj,firstjofj,partidj)){
	    for(int k=0;k<firstjofi.size();++k){
	      //swap localy i and j
	      iti = this->GetPart(partidi[k])->begin() + (_rowi - this->GetPart(partidi[k])->GetI());
	      itj = this->GetPart(partidj[k])->begin() + (_rowj - this->GetPart(partidj[k])->GetI());
	      iend = (*iti).end();jend=(*itj).end();
	      eij = (*itj).begin();
	      for(eii = (*iti).begin(); eii != iend;++eii,++eij){
		*eii = _func(*eii,*eij);
	      }
	    }
	  }
	}
	else{//rowi is not local; only J can be local; if true send it to the
	  //owner of i
	  if(this->dist.IsLocalRow(_rowj,firstjofj,partidj)){
	    for(int k=0;k<firstjofj.size();++k){
	      temp.resize(this->GetPart(partidj[k])->GetnColumns());
	      iti = this->GetPart(partidj[k])->begin() + (_rowj - this->GetPart(partidj[k])->GetI());
	      copy((*iti).begin() , (*iti).end(), &temp[0]);
	      to = this->dist.LookupRow(_rowi,firstjofj[k]).locpid();
	      //cout<<"sent to "<<to<<endl;
	      ARGS  _args(_rowi,firstjofj[k],_func);
	      stapl::async_rmi(to,getHandle(),
			       (void (this_type::*)(ARRAY&,ARGS))&this_type::_rowsOperation,temp,_args);
	    }
	  }
	}
      }
      //@}

      /**@name Matrix specific */
      //@{
      /**@brief 
       *Get threads logical layout.
       *@return pair<unsigned int, unsigned int> threads layout.
      */
      pair<unsigned int, unsigned int> GetLayout(){
	return layout;
      }      

      /**@brief 
       *Get threads logical layout transposed.
       *@return pair<unsigned int, unsigned int> threads layout transposed.
      */
      pair<unsigned int, unsigned int> GetTransposedLayout(){
	return pair<unsigned int, unsigned int>(layout.second, layout.first);
      }
      

protected:
      void _replaceRow(valarray<T>& _temp,pair<unsigned int, unsigned int>& _args){
	//cout<<"received on "<<this->myid<<endl;
	PARTID partid;
	if(dist.IsLocal(_args.first, _args.second,partid)){
	  part_iterator iti = this->GetPart(partid)->begin() + (_args.first - this->GetPart(partid)->GetI());
	  copy(&_temp[0], &_temp[_temp.size()], (*iti).begin());
	}
	else
	  cout<<"ERROR: while swapping rows in pMatrix"<<endl;
      }

      public:
      /**@brief 
       *Swap two rows of the matrix;
       *@param unsigned int _rowi first row.
       *@param unsigned int _rowj second row.
       */
      void SwapRows(unsigned int _rowi, unsigned int _rowj){
	// !!!   Works for one part only; The logic gets complicated for multiple parts
	typedef valarray<T> ARRAY;
	typedef pair<unsigned int, unsigned int> ARGS;
	vector<PARTID> partidi,partidj;
	vector<unsigned int> firstjofi,firstjofj;
	part_iterator iti,itj;
	vector<ARRAY> vtemp;
	ARRAY temp(1);
	int toj = -1, toi=-1;//where to sent the row
	unsigned int destrow;

	if(this->dist.IsLocalRow(_rowi,firstjofi,partidi)){
	  //if _rowi is local
	  if(this->dist.IsLocalRow(_rowj,firstjofj,partidj)){
	    for(int k=0;k<firstjofi.size();++k){
	      //swap localy i and j
	      iti = this->GetPart(partidi[k])->begin() + (_rowi - this->GetPart(partidi[k])->GetI());
	      itj = this->GetPart(partidj[k])->begin() + (_rowj - this->GetPart(partidj[k])->GetI());
	      mtl::swap(*iti,*itj);
	    }
	  }
	  else{
	    //send rowi to owner of _rowj
	    for(int k=0;k<firstjofi.size();++k){
	      temp.resize(this->GetPart(partidi[k])->GetnColumns());
	      iti = this->GetPart(partidi[k])->begin() + (_rowi - this->GetPart(partidi[k])->GetI());
	      copy((*iti).begin() , (*iti).end(), &temp[0]);
	      
	      toj = this->dist.LookupRow(_rowj,firstjofi[k]).locpid();
	      destrow = _rowj;
	      vtemp.push_back(temp);
	    }
	  }
	}
	else{//rowi is not local; only J can be local; if true send it to the
	  //owner of i
	  if(this->dist.IsLocalRow(_rowj,firstjofj,partidj)){
	    for(int k=0;k<firstjofj.size();++k){
	      temp.resize(this->GetPart(partidj[k])->GetnColumns());
	      iti = this->GetPart(partidj[k])->begin() + (_rowj - this->GetPart(partidj[k])->GetI());
	      copy((*iti).begin() , (*iti).end(), &temp[0]);
	      toi = this->dist.LookupRow(_rowi,firstjofj[k]).locpid();
	      destrow = _rowi;
	      vtemp.push_back(temp);
	    }
	  }
	}
	stapl::rmi_fence();
	if(toj != -1){
	  for(int k=0;k<firstjofi.size();++k){
	    cout<<"sent to "<<toj<<endl;
	    stapl::async_rmi(toj,
			     getHandle(),
			     &this_type::_replaceRow,
			     vtemp[k],
			     ARGS(destrow,firstjofi[k]));
	  }
	}
	if(toi != -1){
	  for(int k=0;k<firstjofj.size();++k){
	    cout<<"sent to "<<toi<<endl;
	    stapl::async_rmi(toi,
			     getHandle(),
			     &this_type::_replaceRow,
			     vtemp[k],
			     ARGS(destrow,firstjofj[k]));
	  }
	}
	stapl::rmi_fence();	
      }

      unsigned int GetRowId(iterator _it){
	Part_type *p = _it.GetPart();
	return _it - local_begin() + p.GetI(); 
      }




      //LU decomposition algorithms
      valarray<T> __replace_return_row(_StaplPair<unsigned int, unsigned int> _args, valarray<T>& _data){
	typename Part_type::Container_type::iterator rowi;
	typename Part_type::Container_type::OneD::iterator xrowi;
	PARTID partid;
	Part_type* p;
	if(this->dist.IsLocal(_args.first , _args.second, partid)){
	  p = this->GetPart(partid);
	  valarray<T> _row(p->GetnColumns());
	  rowi = p->part_data.begin() + _args.first - p->GetI();
	  xrowi = (*rowi).begin();
	  for(int k=0;k<p->GetnColumns();++k,++xrowi){
	    _row[k] = *xrowi;
	  }
	  //perform the swap locally
	  xrowi = (*rowi).begin();
	  for(int k=0;k<p->GetnColumns();++k,++xrowi){
	    *xrowi = _data[k];
	  }
	  return _row;
	}
	else{
	  cout<<"ERROR inside __replce_return_row"<<endl; 
	  return valarray<T>();
	}
    }

      void __SwapRows(_StaplPair<unsigned int,unsigned int> _ij, vector<_StaplTriple<unsigned int, T, unsigned int> >& _pivots, unsigned int _status){

	PARTID partid;
	pair<unsigned int, unsigned int> ij, toij;
	unsigned int to;
	Part_type* lp;
	typename Part_type::Container_type::iterator rowi;
	typename Part_type::Container_type::OneD::iterator xrowi;

	pid2ij(this->myid, ij);
	toij = ij;
	for(parts_internal_iterator it = pcontainer_parts.begin(); 
	    it !=pcontainer_parts.end(); 
	    ++it) {
	  Part_type* p = *it;
	  if(p->GetI() == _ij.first && p->GetJ() != _ij.second){
	    valarray<T> _row(p->GetnColumns());
	    valarray<T> _rowSend(p->GetnColumns());

	    //for all pivots perform the swap
	    for(int i=0;i < _status;++i){
	      //cout<<"swap rows:"<<i<<":"<<_pivots[i].third<<endl;
	      if(_pivots[i].third != p->GetI() + i){
		if(_pivots[i].first == this->myid){
		  if(this->dist.IsLocal(_pivots[i].third , p->GetJ(), partid)){
		    lp = this->GetPart(partid);
		    mtl::swap(mtl::rows(p->part_data)[i], mtl::rows(lp->part_data)[_pivots[i].third - lp->GetI()]); /* swap the rows */
		  }
		}
		else{
		  rowi = p->part_data.begin() + i;
		  xrowi = (*rowi).begin();
		  for(int k=0;k<p->GetnColumns();++k,++xrowi){
		    _rowSend[k] = *xrowi;
		  }
		  Location loc = this->dist.Lookup(_pivots[i].third,p->GetJ());
		  stapl_assert(loc.ValidLocation(),"Invalid location for get element");    
		  _StaplPair<unsigned int, unsigned int> args(_pivots[i].third,p->GetJ());
		  _row = stapl::sync_rmi(loc.locpid(),this->getHandle(),
					 &this_type::__replace_return_row,
					 args,_rowSend);
		  //copy from _row to row i
		  xrowi = (*rowi).begin();
		  for(int k=0;k<p->GetnColumns();++k,++xrowi){
		    *xrowi = _row[k];
		  }
		}
	      }
	    }
	  }
	}
      }

      //void broadcastSwapInfoLeftRight(Part_type* p, vector<_StaplTriple<unsigned int, T, unsigned int> >& _pivots){
      void broadcastSwapInfoLeftRight(unsigned int ib, vector<_StaplTriple<unsigned int, T, unsigned int> >& _pivots,unsigned int _status){
	typedef _StaplPair<unsigned int, unsigned int> ARGS;
	//pair<unsigned int, unsigned int> ij, toij;
	//unsigned int to;
	//pid2ij(this->myid, ij);
	//toij = ij;
	//send the pivot information to the left and right of the current 
	//processor
	if(_status != 0){
	  ARGS args(ib,ib);

	  /*
	    for(int s =0;s < layout.second; ++s){
	    if(s != ij.second){
	    toij.second = s;
	    ij2pid(to,toij);
	    stapl::async_rmi(to,this->getHandle(),
	    &this_type::__SwapRows,
	    args,_pivots);
	    }
	    else{
	    //swap locally
	    __SwapRows(args,_pivots);
	    }
	    }
	  */
	  __SwapRows(args,_pivots,_status);
	  stapl::rmi_fence();
	}
      }

      //step 9 and 11 of the algorithm
      void __SendDown(_StaplQuad<unsigned int,unsigned int,unsigned int,unsigned int> _args, vector<T> _temp){
	
	Part_type* A11;
	PARTID partid;
	//compute the global bi out of i,j (_args.third,_args.fourth)
	unsigned int bj = this->dist.GetGlobalBlockId(_args.third,_args.fourth).second;
	if(_temp.size() == 0){
	  //the call is local so the data is local
	  if(this->dist.IsLocal(_args.third,_args.fourth, partid))
	    A11 = *(pcontainer_parts.begin() + partid);
	  else
	    cout<<"ERROR while calling send right"<<endl;
	}
	else{
	  //this is async called
	  A11 = new Part_type(_args.first, 
			    _args.second, 
			    _args.third, 
			    _args.fourth,0);
	  A11->SetPart(_temp);
	  //if(A12[bj] != NULL) delete A12[bj];
	}
	//cout<<myid<<"Down A12 set for bi="<<bj<<" "<<_args.third<<" "<<_args.fourth<<endl;
	flagsA12[bj] = true;
	A12[bj] = A11;
	//multiply this with all already received A21s
	for(parts_internal_iterator it = pcontainer_parts.begin(); 
	    it !=pcontainer_parts.end(); 
	    ++it) {
	  Part_type* lp = *it;
	  //if this column of blocks and down from the current one
	  if(lp->GetJ() == _args.fourth && lp->GetI() > _args.third){
	    //if the corresponding A21 is here perform the multiplication
	    //cout<<"Check down:"<<lp->GetI() / this->dist.Get_mb()<<endl;
	    if(flagsA21[lp->GetI() / this->dist.Get_mb()] == true){
	      //A21 is already received so perform the multiplication
	      //if(myid == 8){
	      //mtl::print_all_matrix(A21[lp->GetI() / this->dist.Get_mb()]->part_data);
	      //	mtl::print_all_matrix(A12[bj]->part_data);
	      //mtl::print_all_matrix(lp->part_data);
	      //}
	      mtl::mult(mtl::scaled(A21[lp->GetI() / this->dist.Get_mb()]->part_data,T(-1)),A12[bj]->part_data,lp->part_data);
	      //if(myid == 8){
	      //int idx = lp->GetI() / this->dist.Get_mb();
	      //cout<<"performing multiplication"<<idx<<endl;
	      //mtl::print_all_matrix(lp->part_data);
 	      //}
	    }
	  }
	}//end for all parts
      }

      //step 10 and 11 of the algorithm
      void __SendRight(_StaplQuad<unsigned int,unsigned int,unsigned int,unsigned int> _args, vector<T> _temp){
	PARTID partid;
	Part_type* A11;
	//compute the global bi out of i,j (_args.third,_args.fourth)
	unsigned int bi = this->dist.GetGlobalBlockId(_args.third,_args.fourth).first;
	if(_temp.size() == 0){
	  //the call is local so the data is local
	  if(this->dist.IsLocal(_args.third,_args.fourth, partid))
	    A11 = *(pcontainer_parts.begin() + partid);
	  else
	    cout<<"ERROR while calling send right"<<endl;
	}
	else{
	  A11 = new Part_type(_args.first, 
			      _args.second, 
			      _args.third, 
			      _args.fourth,0);
	  A11->SetPart(_temp);
	  //if(A21[bi] != NULL) delete A21[bi];
	}
	//cout<<"RIGHT A21 set for bi="<<bi<<endl;
	flagsA21[bi] = true;
	A21[bi] = A11;
	//multiply this with all already received A21s
	for(parts_internal_iterator it = pcontainer_parts.begin(); 
	    it !=pcontainer_parts.end(); 
	    ++it) {
	  Part_type* lp = *it;
	  //if this column of blocks and down from the current one
	  if(lp->GetI() == _args.third && lp->GetJ() > _args.fourth){
	    //if the corresponding A12 is here perform the multiplication
	    //cout<<"Check right:"<<lp->GetJ() / this->dist.Get_mb()<<endl;
	    if(flagsA12[lp->GetJ() / this->dist.Get_nb()] == true){
	      //A12 is already received so perform the multiplication
	      //if(myid == 8){
	      //mtl::print_all_matrix(A12[lp->GetJ() / this->dist.Get_nb()]->part_data);
	      //mtl::print_all_matrix(A21[bi]->part_data);
	      //mtl::print_all_matrix(lp->part_data);
	      //}
	      //mtl::mult(mtl::scaled(A12[lp->GetJ() / this->dist.Get_nb()]->part_data,T(-1)),A21[bi]->part_data,lp->part_data);
	      mtl::mult(mtl::scaled(A21[bi]->part_data,T(-1)),A12[lp->GetJ() / this->dist.Get_nb()]->part_data,lp->part_data);
	      //if(myid == 8){
	      //int idx = lp->GetJ() / this->dist.Get_nb();
	      //cout<<"RIGHT performing multiplication for "<< idx <<endl;
	      //mtl::print_all_matrix(lp->part_data);
	      //}
	    }
	  }
	}//end for all parts
      }

      void __MultiplyLL(_StaplQuad<unsigned int,unsigned int,unsigned int,unsigned int> _args, vector<T> _temp) {

	typedef typename Part_type::Container_type SEQ_MATRIX;
	typedef _StaplQuad<unsigned int,unsigned int,	unsigned int,unsigned int> ARGS;

	pair<unsigned int, unsigned int> ij, downtoij;
	unsigned int to;
	vector<T> empty_v;
	pid2ij(this->myid, ij);
	downtoij = ij;

	Part_type* A11 = new Part_type(_args.first, 
				       _args.second, 
				      _args.third, 
				      _args.fourth,0);
	A11->SetPart(_temp);
	//now perform the multiplication
	//First extract L_11 out of A11
	typename mtl::triangle_view<SEQ_MATRIX,mtl::unit_lower>::type L_11(A11->part_data);
	//second multiply L_11 with A12 (all local components of A12)
	for(parts_internal_iterator it = pcontainer_parts.begin(); 
	    it !=pcontainer_parts.end(); 
	    ++it) {
	  Part_type* lp = *it;
	  //if this row of blocks and to the right of the current one
	  if(lp->GetI() == _args.third && lp->GetJ() > A11->GetJ()){
	    mtl::tri_solve(L_11,lp->part_data,mtl::left_side());
	    //send down to all procs on the same column A12
	    ARGS args(lp->GetnRows(),lp->GetnColumns(),lp->GetI(),lp->GetJ());
	    for(int t =0;t < layout.first; ++t){
	      if(t != ij.first){
		downtoij.first = t;
		ij2pid(to,downtoij);
		stapl::async_rmi(to,this->getHandle(),
				 &this_type::__SendDown,
				 args,*(lp->GetVectorData()));
	      }
	      else{
		//there are parts that are local also
		__SendDown(args,empty_v);
	      }
	    }
	  }
	}//end for all parts
      }

      void broadcastMultiplyLLRight(unsigned int ib, unsigned int _status){
	typedef typename Part_type::Container_type SEQ_MATRIX;
	pair<unsigned int, unsigned int> ij, toij,downtoij;
	typedef _StaplQuad<unsigned int,unsigned int,unsigned int,unsigned int> ARGS;
	unsigned int to;
	PARTID partid;
	if(_status != 0){
	  if(dist.IsLocal(ib,ib,partid)){
	    Part_type* p = this->GetPart(partid);
	    vector<T> _temp;
	    pid2ij(this->myid, ij);
	    toij = ij;
	    downtoij = ij;
	    //send the pivot information to the left and right of the current 
	    //processor
	    for(int s =0;s < layout.second; ++s){
	      if(s != ij.second){
		toij.second = s;
		ij2pid(to,toij);
		ARGS args(p->GetnRows(),p->GetnColumns(),ib,p->GetJ());
		stapl::async_rmi(to,this->getHandle(),
				 &this_type::__MultiplyLL,
				 args,*(p->GetVectorData()));
	      }
	      else{
		//multiply locally(Step 8)
		//First extract L_11 out of A11
		typename mtl::triangle_view<SEQ_MATRIX,mtl::unit_lower>::type L_11(p->part_data);
		//second multiply L_11 with A12 (all local components of A12)
		for(parts_internal_iterator it = pcontainer_parts.begin(); 
		    it !=pcontainer_parts.end(); 
		    ++it) {
		  Part_type* lp = *it;
		  //if this row of blocks and to the right of the current one
		  if(lp->GetI() == ib && lp->GetJ() > p->GetJ()){
		    mtl::tri_solve(L_11,lp->part_data,mtl::left_side());
		    //send down to all procs on the same column A12
		    ARGS args(lp->GetnRows(),lp->GetnColumns(),lp->GetI(),lp->GetJ());
		    for(int t =0;t < layout.first; ++t){
		      if(t != ij.first){
			downtoij.first = t;
			ij2pid(to,downtoij);
			stapl::async_rmi(to,this->getHandle(),
					 &this_type::__SendDown,
					 args,*(lp->GetVectorData()));
		      }
		      else{
			//there are parts that are local also
			__SendDown(args,_temp);
		      }
		    }
		  }
		}//end for all parts
	      }
	    }
	  }
	}
	//stapl::rmi_fence();
      }


      void rank_one_update_column(_StaplPenta<unsigned int,unsigned int,unsigned int,unsigned int,unsigned int> _args, 
				  valarray<T> _temp){
	typedef _StaplPenta<unsigned int,unsigned int,unsigned int,unsigned int,unsigned int> ARGS;
	typedef _StaplQuad<unsigned int,unsigned int,unsigned int,unsigned int> ARGS_SENDRIGHT;

	typedef typename Part_type::Container_type DenseMatrix;
	typedef typename mtl::columns_type<typename Part_type::Container_type>::type ColumnMatrix;
	T* xrowi;
	typename Part_type::Container_type::submatrix_type subA;
	pair<unsigned int, unsigned int> ij, toij;
	unsigned int to;
	vector<T> empty_v;
	
	for(parts_internal_iterator it = pcontainer_parts.begin(); 
	    it !=pcontainer_parts.end(); 
	    ++it) {
	  Part_type* lp = *it;
	  //if this column of blocks and down from the current one
	  if(lp->GetJ() == _args.fourth && lp->GetI() > _args.third){
	    //rank one update code
	    ColumnMatrix L(mtl::columns(lp->part_data));
	    typename ColumnMatrix::iterator columni = L.begin() + _args.fifth;
	      
	    typename DenseMatrix::submatrix_type::iterator ii;
	    typename DenseMatrix::submatrix_type::OneD::iterator jj, jjend;
	    typename ColumnMatrix::OneD::iterator yycolumni;
	    /*
	    if(myid == 2){
	      cout<<"PROBLEM"<<_args.fifth<<endl;
	      for(int k=0;k<lp->GetnColumns();++k){
		cout<<_temp[k]<<" ";
	      }
	      cout<<endl;
	    }
	    */
	    if(columni == L.end()) break;
	    //if (j < MM - 1)
	    mtl::scale(*columni, T(1) / _temp[_args.fifth]);    /* update column under the pivot */
	    //j+1 is very important
	    subA = lp->part_data.sub_matrix(0, lp->GetnRows(), _args.fifth + 1, lp->GetnColumns());
	    yycolumni=(*columni).begin();
	    for (ii = subA.begin(); ii != subA.end(); ++ii) {
	      jj = (*ii).begin(); jjend = (*ii).end();
	      //j+1 is very important
	      xrowi = &_temp[_args.fifth+1];
	      for (; jj != jjend; ++jj,++xrowi)
		*jj -= (*xrowi) * (*yycolumni);
	      ++yycolumni;
	    }
	    //mtl::print_all_matrix(lp->part_data);
	  }
	  
	  /*
	    if((_args.fifth + 1) == lp->GetnColumns() || _temp[0] == 0){
	    //here A11,A21 is factorized; send A21to the right now;
	    pid2ij(this->myid, ij);
	    toij = ij;
	    //send the pivot information to the left and right of the current 
	    //processor
	    ARGS_SENDRIGHT sr_args(lp->GetnRows(),lp->GetnColumns(),lp->GetI(),lp->GetJ());
	    for(int s =0;s < layout.second; ++s){
	    if(s != ij.second){
	    toij.second = s;
	    ij2pid(to,toij);
	    stapl::async_rmi(to,this->getHandle(),
	    &this_type::__SendRight,
	    sr_args,*(lp->GetVectorData()));
	    }
	    else{
	    __SendRight(sr_args,empty_v);
	    }
	    }
	    }
	  */
	}//end for all parts
      }

      valarray<T> __broadcast_pivotrow(_StaplPair<_StaplPenta<unsigned int,unsigned int,unsigned int,unsigned int,unsigned int>,
				                  _StaplPair<unsigned int, unsigned int> > _args, 
				       valarray<T>& _data){
	typename Part_type::Container_type::iterator rowi;
	typename Part_type::Container_type::OneD::iterator xrowi;
	PARTID partid;					
	pair<unsigned int, unsigned int> ij, toij;
	unsigned int to;
	Part_type* p;
	
	pid2ij(this->myid, ij);
	toij = ij;
	if(this->dist.IsLocal(_args.second.first , _args.first.fourth, partid)){
	  p = this->GetPart(partid);
	  valarray<T> _row(p->GetnColumns());
	  rowi = p->part_data.begin() + _args.second.first - p->GetI();
	  xrowi = (*rowi).begin();
	  for(int k=0;k<p->GetnColumns();++k,++xrowi){
	    _row[k] = *xrowi;
	  }
	  for(int s =0;s < layout.first; ++s){
	    if(s != ij.first){
	      toij.first = s;
	      ij2pid(to,toij);
	      if(to != _args.second.second){
		//if(to == 3) 
		//cout<<this->myid<<" broadcast calling rank on 3"<<endl;
		stapl::async_rmi(to,this->getHandle(),
			       &this_type::rank_one_update_column,
			       _args.first,_row);
	      }
	    }
	  }
	  //perform the swap locally
	  xrowi = (*rowi).begin();
	  for(int k=0;k<p->GetnColumns();++k,++xrowi){
	    *xrowi = _data[k];
	  }
	  
	  //see if I can schedule this asynchronous before return _row
	  rank_one_update_column(_args.first,_row);
	  
	  return _row;
	}
	else{
	  cout<<"ERROR inside __broadcast_pivotrow"<<endl; 
	  return valarray<T>();
	}
      }
				       
      void swap_A11A12(_StaplTriple<unsigned int, T, unsigned int>& pi, Part_type*  part,unsigned int lj){
        typedef _StaplPenta<unsigned int,unsigned int,unsigned int,unsigned int,unsigned int> ARGS;	
	PARTID partid;
	pair<unsigned int, unsigned int> ij, toij;
	unsigned int to;
	Part_type* lp;
	typename Part_type::Container_type::iterator rowi;
	typename Part_type::Container_type::OneD::iterator xrowi;
	valarray<T> _row(part->GetnColumns());
	valarray<T> _rowSend(part->GetnColumns());

	pid2ij(this->myid, ij);
	toij = ij;
	ARGS args(part->GetnRows(),part->GetnColumns(),part->GetI(),part->GetJ(),lj);
	if(pi.first == this->myid){
	  if(this->dist.IsLocal(pi.third , part->GetJ(), partid)){
	    //if the pivot is the same with the current row don't swap
	    if (pi.third != part->GetI() + lj){
	      lp = this->GetPart(partid);
	      mtl::swap(mtl::rows(part->part_data)[lj], mtl::rows(lp->part_data)[pi.third - lp->GetI()]); /* swap the rows */
	    }
	    //broadcast the new row down;
	    //????
	    rowi = part->part_data.begin() + lj;
	    xrowi = (*rowi).begin();
	    for(int k=0;k<part->GetnColumns();++k,++xrowi){
	      _row[k] = *xrowi;
	    }
     	    for(int s =0;s < layout.first; ++s){
	      if(s != ij.first){
		toij.first = s;
		ij2pid(to,toij);
		//if(to == 3) 
		//cout<<this->myid<<" calling swapA11A12rank on 3"<<endl;
		stapl::async_rmi(to,this->getHandle(),
			       &this_type::rank_one_update_column,
				 args,_row);
	      }
	    }
	  }
	}
	else{
	  rowi = part->part_data.begin() + lj;
	  xrowi = (*rowi).begin();
	  for(int k=0;k<part->GetnColumns();++k,++xrowi){
	    _rowSend[k] = *xrowi;
	  }
	  _StaplPair<ARGS,_StaplPair<unsigned int, unsigned int> > bargs(args,_StaplPair<unsigned int, unsigned int>(pi.third,this->myid));
	  _row = stapl::sync_rmi(pi.first,this->getHandle(),
			   &this_type::__broadcast_pivotrow,
			   bargs,_rowSend);
	  //copy from _row to row i
	  rowi = part->part_data.begin() + lj;
	  xrowi = (*rowi).begin();
	  for(int k=0;k<part->GetnColumns();++k,++xrowi){
	    *xrowi = _row[k];
	  }
	}
	
      }
      void _max_reduce(_StaplTriple<unsigned int, T, unsigned int>* in, _StaplTriple<unsigned int, T, unsigned int>* inout) {
	if(in->second > inout->second) *inout = *in;
      }

      template <class literator>
       _StaplTriple<unsigned int, T, unsigned int> 
      find_pivot(literator& dcoli, unsigned int _i,unsigned int _j, unsigned int j,bool local){
	typedef _StaplTriple<unsigned int, T, unsigned int> PIVOTINFO;
	typedef typename Part_type::Container_type DenseMatrix;
	typedef typename mtl::columns_type<typename Part_type::Container_type>::type ColumnMatrix;

	PIVOTINFO ip,result;
	pair<unsigned int, unsigned int> ij;	
	unsigned int idx;
	T _max;

	stapl::rmi_fence();

	pid2ij(this->myid, ij);
	if(local){
	  ip.first = this->myid;
	  idx = mtl::max_abs_index(*dcoli);//the global row id
	  ip.second = ((*dcoli)[idx]>=0)?(*dcoli)[idx]:-1 * (*dcoli)[idx];
	  ip.third = idx + _i;
	}
	else{
	  ip.first  = -1;
	  ip.second = T(0);
	  ip.third  = 0;
	}
	for(parts_internal_iterator it = pcontainer_parts.begin(); 
	    it !=pcontainer_parts.end(); 
	    ++it) {
	      Part_type* lp = *it;
	      //if this row of blocks and to the right of the current one
	      if(lp->GetJ() == _j && lp->GetI() > _i){
		ip.first = this->myid;
		ColumnMatrix L(mtl::columns(lp->part_data));
		typename ColumnMatrix::iterator columni = L.begin() + j;
		idx = mtl::max_abs_index(*columni);
		_max = ((*columni)[idx]>=0)?(*columni)[idx]:-1 * (*columni)[idx];
		if(_max > ip.second){
		  ip.second = _max;
		  ip.third = lp->GetI() + idx; 
		}
	      }
	    }
	//here all threads have their own maximum computed
	//perform the reduce; 
	stapl::rmi_fence();
	stapl::reduce_rmi( &ip, &result, this->getHandle(), &this_type::_max_reduce,true);
	return result;
      }

      unsigned int _factorizeA11A21(unsigned int _i, unsigned int _j, vector<_StaplTriple<unsigned int, T, unsigned int> >& ipvt){
	//this code is copy paste from MTL lu_factor algorithm
	//I need to do this to overlap the computation with communication
	typedef _StaplTriple<unsigned int, T, unsigned int> PIVOTINFO;
	typedef typename Part_type::Container_type DenseMatrix;
	typedef typename mtl::rows_type<DenseMatrix>::type RowMatrix;
	typedef typename mtl::columns_type<DenseMatrix>::type ColumnMatrix;
	typedef typename mtl::triangle_view<ColumnMatrix, mtl::lower>::type Lower;
	typedef typename mtl::triangle_view<RowMatrix, mtl::unit_upper>::type Unit_Upper;
	typedef typename mtl::triangle_view<ColumnMatrix, mtl::unit_lower>::type Unit_Lower;
	typedef typename DenseMatrix::size_type sizet;
	
	int info = 0;
	pair<unsigned int, unsigned int> ij, toij;
	typedef _StaplPenta<unsigned int,unsigned int,unsigned int,unsigned int,unsigned int> ARGS;
	typename DenseMatrix::submatrix_type subA;
	typename Lower::iterator dcoli;
	typename Unit_Upper::iterator rowi;
	typename Unit_Upper::OneD::iterator xrowi;
	typename Unit_Lower::iterator columni;
	typename Unit_Lower::iterator trowi,tcolumni;
	typename Unit_Lower::OneD::iterator ycolumni;
	unsigned int _status = 0;
	unsigned int to;
	vector<T> _temp;
	Part_type* p;
	PARTID partid;
	unsigned int j, jp, MM,NN;
	unsigned int nb=this->dist.Get_nb();
	bool local;
	PIVOTINFO pi;
	if(dist.IsLocal(_i,_j,partid)){
	  ipvt.clear();
	  //only the active thread will point to a relevant part;
	  //the other threads will point to part 0; this is a 
	  //compiler limitation
	  p = this->GetPart(partid);
	  //ipvt.resize(p->GetnRows());
	  MM = p->part_data.nrows();
	  NN = p->part_data.ncols();
	  local = true;
	}
	else {
	  partid = 0;
	  p = this->GetPart(partid);
	  NN=0;MM=0;
	  local = false;
	}
	valarray<T> _row(NN);
	Lower D(mtl::columns(p->part_data));
	Unit_Upper U(mtl::rows(p->part_data));
	Unit_Lower L(mtl::columns(p->part_data));
	if(local){
	  dcoli   = D.begin();
	  rowi    = U.begin();
	  columni = L.begin();
	}
	unsigned int end = ((_i+nb) < N)?_i+nb:N;
	//all threads will loop the same number of times
	for (j = 0; j < end - _i; ++j) {
	  /*
	  if(myid == 0){
	    for (int ii1 = 0; ii1 < M; ++ii1) {
	      for (int jj1 = 0; jj1 < N; ++jj1) {
		cout<<this->GetElement(ii1,jj1)<<" ";
	      }
	      cout<<endl;
	    }
	  }
	  stapl::rmi_fence();
	  */

	  pi = find_pivot(dcoli,_i,_j,j,local);		   /* find pivot information*/

	  ipvt[j] = pi;

	  //cout<<this->myid<<"Pivot value:"<<pi.second<<"iteration::"<<_i+j<<endl;

	  if ( pi.second != T(0) ) {	  /* make sure pivot isn't zero */
	    _status++;
	    if(local){
	      //if (pi.third != _i + j){
	      //mtl::swap(mtl::rows(p->part_data)[j], mtl::rows(p->part_data)[jp]); /* swap the rows */
		swap_A11A12(pi,p,j);
		//}
	      if (j < MM - 1)
		mtl::scale(*columni, T(1) / p->part_data(j,j));    /* update column under the pivot */
	    }//if local
	  } else {
	    for(int s =0;s < stapl::get_num_threads(); ++s){
	      if(s != this->myid)
		stapl::async_rmi(s,this->getHandle(),
				 &this_type::__LU_SetDone);
	    }
	    done_flag=true;
	    info = j + 1;
	  }

	  if(local){
	    if(done_flag == false || (done_flag==true && j != 0)){
	      //rank one update the current block
	      if (j < min(MM , NN)) {
		if(done_flag == false){
		  subA = p->part_data.sub_matrix(j+1, MM, j+1, NN);
		  /* TODO: Better to have an adaptor here -- p->part_data.L. */
		  //there is a copy involved here that I don't like
		  //mtl::copy(*columni, c);  
		  //mtl::copy(*rowi, r);  /* translate to submatrix coords */
		  //mtl::rank_one_update(subA, mtl::scaled(c, T(-1)), r); /* update the submatrix */
		  typename DenseMatrix::submatrix_type::iterator ii;
		  typename DenseMatrix::submatrix_type::OneD::iterator jj, jjend;
		  ycolumni=(*columni).begin();
		  for (ii = subA.begin(); ii != subA.end(); ++ii) {
		    jj = (*ii).begin(); jjend = (*ii).end();
		    xrowi = (*rowi).begin();
		    for (; jj != jjend; ++jj,++xrowi){
		      //cout<<*jj<<"-="<<*ycolumni<<" * "<<*xrowi<<" ";
		      *jj -= (*xrowi) * (*ycolumni);
		    }
		    //cout<<endl;
		    ++ycolumni;
		  }
		}
		
		//rank one update all the other blocks in the column 
		//from this one down
		for(parts_internal_iterator it = pcontainer_parts.begin(); 
		    it !=pcontainer_parts.end(); 
		    ++it) {
		  Part_type* lp = *it;
		  //if this column of blocks and down from the current one
		  if(lp->GetJ() == p->GetJ() && lp->GetI() > p->GetI()){
		    if(done_flag == false){
		      //rank one update code
		      ColumnMatrix LL(mtl::columns(lp->part_data));
		      typename ColumnMatrix::iterator ccolumni = LL.begin() + j;
		      typename DenseMatrix::submatrix_type::iterator ii;
		      typename DenseMatrix::submatrix_type::OneD::iterator jj, jjend;
		      typename ColumnMatrix::OneD::iterator yycolumni;
		      
		      if(ccolumni == LL.end()) break;

		      //cout<<"factorizing DOWN..."<<endl;
		      //mtl::print_all_matrix(lp->part_data);

		      //if (j < MM - 1)
		      mtl::scale(*ccolumni, T(1) / p->part_data(j,j));    /* update column under the pivot */
		      
		      
		      //j+1 is very important
		      subA = lp->part_data.sub_matrix(0, lp->GetnRows(), j + 1,lp->GetnColumns());
		      
		      //cout<<"subA DOWN..."<<endl;
		      //mtl::print_all_matrix(subA);
		      //cout<<"------------------------"<<endl;
		      
		      yycolumni=(*ccolumni).begin();
		      for (ii = subA.begin(); ii != subA.end(); ++ii) {
			jj = (*ii).begin(); jjend = (*ii).end();
			xrowi = (*rowi).begin();
			for (; jj != jjend; ++jj,++xrowi){
			  //cout<<"DOWN"<<*jj<<"-="<<*yycolumni<<" * "<<*xrowi<<" ";
			  *jj -= (*xrowi) * (*yycolumni);
			}
			++yycolumni;
		      }
		    }
		    /*
		      if((j+1) == lp->GetnColumns() || done_flag == true){
		      //here A11,A21 is factorized; send A21to the right now;
		      pid2ij(this->myid, ij);
		      toij = ij;
		      //send the pivot information to the left and right of the current 
		      //processor
		      ARGS_SENDRIGHT args(lp->GetnRows(),lp->GetnColumns(),lp->GetI(),lp->GetJ());
		      for(int s =0;s < layout.second; ++s){
		      if(s != ij.second){
		      toij.second = s;
		      ij2pid(to,toij);
		      stapl::async_rmi(to,this->getHandle(),
		      &this_type::__SendRight,
		      args,*(lp->GetVectorData()));
		      }
		      else{
			__SendRight(args,_temp);
		      }
		    }
		    }
		    */
		  }
		}//end for all parts
	      }
	    }
	    ++dcoli; 
	    ++rowi; 
	    ++columni;
	  }
	  /*
	    if(myid == 0){
	    for (int ii1 = 0; ii1 < M; ++ii1) {
	      for (int jj1 = 0; jj1 < N; ++jj1) {
		cout<<this->GetElement(ii1,jj1)<<" ";
	      }
	      cout<<endl;
	    }
	  }
	  stapl::rmi_fence();
	  */
	}
	//ipvt[j] = j;
	return _status;
      }//end factorizeA11A21

      void __LU_SetDone(){
	done_flag =true;
      }

      void broadcastA12Right(unsigned int ib, unsigned int status){
	typedef _StaplQuad<unsigned int,unsigned int,unsigned int,unsigned int> ARGS_SENDRIGHT;
	pair<unsigned int, unsigned int> ij, toij;
	unsigned int to;
	vector<T> _temp;
	pid2ij(this->myid, ij);
	toij = ij;
	//send the pivot information to the left and right of the current 
	//processor
	//from this one down
	if(status != 0){
	  for(parts_internal_iterator it = pcontainer_parts.begin(); 
	      it !=pcontainer_parts.end(); 
	      ++it) {
	    Part_type* lp = *it;
	    //if this column of blocks and down from the current one
	    if(lp->GetJ() == ib && lp->GetI() > ib){
	      ARGS_SENDRIGHT args(lp->GetnRows(),lp->GetnColumns(),lp->GetI(),lp->GetJ());
	      for(int s =0;s < layout.second; ++s){
		if(s != ij.second){
		  toij.second = s;
		  ij2pid(to,toij);
		  stapl::async_rmi(to,this->getHandle(),
				   &this_type::__SendRight,
				   args,*(lp->GetVectorData()));
		}
		else{
		  __SendRight(args,_temp);
		}
	      }
	    }
	  }
	}//if status
      }

      void LU(){
	typedef _StaplTriple<unsigned int, T, unsigned int> PIVOTINFO;
	unsigned int k;
	unsigned int ib;
	unsigned int end;
	unsigned int nb=dist.Get_nb();
	unsigned int status;
	PARTID partid;
	Part_type* p;
	vector<PIVOTINFO> pivots(M);
	done_flag = false;
	flagsA12.resize(this->dist.BQ);
	flagsA21.resize(this->dist.BP);
	A12.resize(this->dist.BQ);
	A21.resize(this->dist.BP);
	ib = 0;
	while(ib<N && done_flag == false){

	  for(k=0;k<this->dist.BQ;++k){
	    flagsA12[k] = false;
	    //here I have to free up some space;
	    //if(A12[k] != NULL && !this->dist.IsLocal(A12[k]->GetI(),A12[k]->GetJ(),partid))
	    //delete A12[k];
	    A12[k] = NULL;
	  }
	  for(k=0;k<this->dist.BP;++k){
	    flagsA21[k] = false;
	    //here I have to free up some space;
	    //if(A21[k] != NULL && !this->dist.IsLocal(A21[k]->GetI(),A21[k]->GetJ(),partid))
	    //delete A21[k];
	    A21[k] = NULL;
	  }
	  stapl::rmi_fence();
	  //if(dist.IsLocal(ib,ib,partid)){

	  //LU current block and the blocks on the same column(Steps 1..4 and 10)
	  status = _factorizeA11A21(ib,ib,pivots);

	  
	  //pivots.resize(p->GetnColumns());
	  //mtl::lu_factor(p->part_data, pivots);
	  
	  //broadcats the pivot information to the processors that owns
	  //the other columns and perform the swap (step5,6)

	  broadcastSwapInfoLeftRight(ib,pivots,status);

	  //broadcast A12 right
	  broadcastA12Right(ib,status);

	  //broadcast LL right and perform LL^-1 * A 1,2 (Step 7,8)
	  //this step triggers also steps 9,10,11
	  broadcastMultiplyLLRight(ib,status);
	  
	  //}//if I own current block
	  ib+=nb;
	  
	  stapl::rmi_fence();
	  /*
	  if(myid == 0){
	  for (int i = 0; i < M; ++i) {
	    for (int j = 0; j < N; ++j) {
	  cout<<this->GetElement(i,j)<<" ";
	    }
	    cout<<endl;
	  }
	  }
	  stapl::rmi_fence();
	  */
	}//for all columns
      }//end LU decomposition
      
      //----------------------------------------------------
      //----------------------------------------------------
      //LU decomp w/o pivoting
      //----------------------------------------------------
      //----------------------------------------------------

      void __SwapRowsNP(_StaplPair<unsigned int,unsigned int> _ij, vector<unsigned int>& _pivots){
	PARTID partid;
	
	for(parts_internal_iterator it = pcontainer_parts.begin(); 
	    it !=pcontainer_parts.end(); 
	    ++it) {
	  Part_type* p = *it;
	  if(p->GetI() == _ij.first && p->GetJ() != _ij.second){
	    //for all pivots perform the swap
	    for(int i=0;i<_pivots.size();++i){
	      cout<<"swap rows:"<<i<<":"<<_pivots[i]<<endl;
	      if(_pivots[i] != i)
	      mtl::swap(mtl::rows(p->part_data)[i], 
			mtl::rows(p->part_data)[_pivots[i]]); /* swap the rows */ 
	    }
	  }//if the right row
	}//for all parts
      }

      void broadcastSwapInfoLeftRightNP(unsigned int rowid, unsigned int colid, vector<unsigned int>& _pivots){
	typedef _StaplPair<unsigned int, unsigned int> ARGS;
	pair<unsigned int, unsigned int> ij, toij;
	unsigned int to;
	pid2ij(this->myid, ij);
	toij = ij;
	//send the pivot information to the left and right of the current 
	//processor
	ARGS args(rowid,colid);
	for(int s =0;s < layout.second; ++s){
	  if(s != ij.second){
	    toij.second = s;
	    ij2pid(to,toij);
	    stapl::async_rmi(to,this->getHandle(),
			     &this_type::__SwapRowsNP,
			     args,_pivots);
	  }
	  else{
	    //swap locally
	    __SwapRowsNP(args,_pivots);
	  }
	}
	//stapl::rmi_fence();
      }

      //step 9 and 11 of the algorithm
      void __SendDownNP(_StaplQuad<unsigned int,unsigned int,unsigned int,unsigned int> _args, vector<T> _temp){
	
	Part_type* A11;
	PARTID partid;
	//compute the global bi out of i,j (_args.third,_args.fourth)
	unsigned int bj = this->dist.GetGlobalBlockId(_args.third,_args.fourth).second;
	if(_temp.size() == 0){
	  //the call is local so the data is local
	  if(this->dist.IsLocal(_args.third,_args.fourth, partid))
	    A11 = *(pcontainer_parts.begin() + partid);
	  else
	    cout<<"ERROR while calling send right"<<endl;
	}
	else{
	  //this is async called
	  A11 = new Part_type(_args.first, 
			    _args.second, 
			    _args.third, 
			    _args.fourth,0);
	  A11->SetPart(_temp);
	  //if(A12[bj] != NULL) delete A12[bj];
	}
	//cout<<myid<<"Down A12 set for bi="<<bj<<" "<<_args.third<<" "<<_args.fourth<<endl;
	flagsA12[bj] = true;
	A12[bj] = A11;
	//multiply this with all already received A21s
	for(parts_internal_iterator it = pcontainer_parts.begin(); 
	    it !=pcontainer_parts.end(); 
	    ++it) {
	  Part_type* lp = *it;
	  //if this column of blocks and down from the current one
	  if(lp->GetJ() == _args.fourth && lp->GetI() > _args.third){
	    //if the corresponding A21 is here perform the multiplication
	    //cout<<"Check down:"<<lp->GetI() / this->dist.Get_mb()<<endl;
	    if(flagsA21[lp->GetI() / this->dist.Get_mb()] == true){
	      //A21 is already received so perform the multiplication
	      //if(myid == 8){
	      //mtl::print_all_matrix(A21[lp->GetI() / this->dist.Get_mb()]->part_data);
	      //	mtl::print_all_matrix(A12[bj]->part_data);
	      //mtl::print_all_matrix(lp->part_data);
	      //}
	      mtl::mult(mtl::scaled(A21[lp->GetI() / this->dist.Get_mb()]->part_data,T(-1)),A12[bj]->part_data,lp->part_data);
	      //if(myid == 8){
	      //int idx = lp->GetI() / this->dist.Get_mb();
	      //cout<<"performing multiplication"<<idx<<endl;
	      //mtl::print_all_matrix(lp->part_data);
 	      //}
	    }
	  }
	}//end for all parts
      }

      //step 10 and 11 of the algorithm
      void __SendRightNP(_StaplQuad<unsigned int,unsigned int,unsigned int,unsigned int> _args, vector<T> _temp){
	PARTID partid;
	Part_type* A11;
	//compute the global bi out of i,j (_args.third,_args.fourth)
	unsigned int bi = this->dist.GetGlobalBlockId(_args.third,_args.fourth).first;
	if(_temp.size() == 0){
	  //the call is local so the data is local
	  if(this->dist.IsLocal(_args.third,_args.fourth, partid))
	    A11 = *(pcontainer_parts.begin() + partid);
	  else
	    cout<<"ERROR while calling send right"<<endl;
	}
	else{
	  A11 = new Part_type(_args.first, 
			      _args.second, 
			      _args.third, 
			      _args.fourth,0);
	  A11->SetPart(_temp);
	  //if(A21[bi] != NULL) delete A21[bi];
	}
	//cout<<"RIGHT A21 set for bi="<<bi<<endl;
	flagsA21[bi] = true;
	A21[bi] = A11;
	//multiply this with all already received A21s
	for(parts_internal_iterator it = pcontainer_parts.begin(); 
	    it !=pcontainer_parts.end(); 
	    ++it) {
	  Part_type* lp = *it;
	  //if this column of blocks and down from the current one
	  if(lp->GetI() == _args.third && lp->GetJ() > _args.fourth){
	    //if the corresponding A12 is here perform the multiplication
	    //cout<<"Check right:"<<lp->GetJ() / this->dist.Get_mb()<<endl;
	    if(flagsA12[lp->GetJ() / this->dist.Get_nb()] == true){
	      //A12 is already received so perform the multiplication
	      //if(myid == 8){
	      //mtl::print_all_matrix(A12[lp->GetJ() / this->dist.Get_nb()]->part_data);
	      //mtl::print_all_matrix(A21[bi]->part_data);
	      //mtl::print_all_matrix(lp->part_data);
	      //}
	      //mtl::mult(mtl::scaled(A12[lp->GetJ() / this->dist.Get_nb()]->part_data,T(-1)),A21[bi]->part_data,lp->part_data);
	      mtl::mult(mtl::scaled(A21[bi]->part_data,T(-1)),A12[lp->GetJ() / this->dist.Get_nb()]->part_data,lp->part_data);
	      //if(myid == 8){
	      //int idx = lp->GetJ() / this->dist.Get_nb();
	      //cout<<"RIGHT performing multiplication for "<< idx <<endl;
	      //mtl::print_all_matrix(lp->part_data);
	      //}
	    }
	  }
	}//end for all parts
      }

      void __MultiplyLLNP(_StaplQuad<unsigned int,unsigned int,unsigned int,unsigned int> _args, vector<T> _temp) {

	typedef typename Part_type::Container_type SEQ_MATRIX;
	typedef _StaplQuad<unsigned int,unsigned int,	unsigned int,unsigned int> ARGS;

	pair<unsigned int, unsigned int> ij, downtoij;
	unsigned int to;
	vector<T> empty_v;
	pid2ij(this->myid, ij);
	downtoij = ij;

	Part_type* A11 = new Part_type(_args.first, 
				       _args.second, 
				      _args.third, 
				      _args.fourth,0);
	A11->SetPart(_temp);
	//now perform the multiplication
	//First extract L_11 out of A11
	typename mtl::triangle_view<SEQ_MATRIX,mtl::unit_lower>::type L_11(A11->part_data);
	//second multiply L_11 with A12 (all local components of A12)
	for(parts_internal_iterator it = pcontainer_parts.begin(); 
	    it !=pcontainer_parts.end(); 
	    ++it) {
	  Part_type* lp = *it;
	  //if this row of blocks and to the right of the current one
	  if(lp->GetI() == _args.third && lp->GetJ() > A11->GetJ()){
	    mtl::tri_solve(L_11,lp->part_data,mtl::left_side());
	    //send down to all procs on the same column A12
	    ARGS args(lp->GetnRows(),lp->GetnColumns(),lp->GetI(),lp->GetJ());
	    for(int t =0;t < layout.first; ++t){
	      if(t != ij.first){
		downtoij.first = t;
		ij2pid(to,downtoij);
		stapl::async_rmi(to,this->getHandle(),
				 &this_type::__SendDownNP,
				 args,*(lp->GetVectorData()));
	      }
	      else{
		//there are parts that are local also
		__SendDownNP(args,empty_v);
	      }
	    }
	  }
	}//end for all parts
      }

      void broadcastMultiplyLLRightNP(Part_type* p, unsigned int rowid){
	typedef typename Part_type::Container_type SEQ_MATRIX;
	pair<unsigned int, unsigned int> ij, toij,downtoij;
	typedef _StaplQuad<unsigned int,unsigned int,unsigned int,unsigned int> ARGS;
	unsigned int to;
	vector<T> _temp;
	pid2ij(this->myid, ij);
	toij = ij;
	downtoij = ij;
	//send the pivot information to the left and right of the current 
	//processor
	for(int s =0;s < layout.second; ++s){
	  if(s != ij.second){
	    toij.second = s;
	    ij2pid(to,toij);
	    ARGS args(p->GetnRows(),p->GetnColumns(),rowid,p->GetJ());
	    stapl::async_rmi(to,this->getHandle(),
			     &this_type::__MultiplyLLNP,
			     args,*(p->GetVectorData()));
	  }
	  else{
	    //multiply locally(Step 8)
	    //First extract L_11 out of A11
	    typename mtl::triangle_view<SEQ_MATRIX,mtl::unit_lower>::type L_11(p->part_data);
	    //second multiply L_11 with A12 (all local components of A12)
	    for(parts_internal_iterator it = pcontainer_parts.begin(); 
		it !=pcontainer_parts.end(); 
		++it) {
	      Part_type* lp = *it;
	      //if this row of blocks and to the right of the current one
	      if(lp->GetI() == rowid && lp->GetJ() > p->GetJ()){
		mtl::tri_solve(L_11,lp->part_data,mtl::left_side());
		//send down to all procs on the same column A12
		ARGS args(lp->GetnRows(),lp->GetnColumns(),lp->GetI(),lp->GetJ());
		for(int t =0;t < layout.first; ++t){
		  if(t != ij.first){
		    downtoij.first = t;
		    ij2pid(to,downtoij);
		    stapl::async_rmi(to,this->getHandle(),
				     &this_type::__SendDownNP,
				     args,*(lp->GetVectorData()));
		  }
		  else{
		    //there are parts that are local also
		    __SendDownNP(args,_temp);
		  }
		}
	      }
	    }//end for all parts
	  }
	}
	//stapl::rmi_fence();
      }


      void rank_one_update_columnNP(_StaplPenta<unsigned int,unsigned int,unsigned int,unsigned int,unsigned int> _args, 
				  valarray<T> _temp){
	typedef _StaplPenta<unsigned int,unsigned int,unsigned int,unsigned int,unsigned int> ARGS;
	typedef _StaplQuad<unsigned int,unsigned int,unsigned int,unsigned int> ARGS_SENDRIGHT;

	typedef typename Part_type::Container_type DenseMatrix;
	typedef typename mtl::columns_type<typename Part_type::Container_type>::type ColumnMatrix;
	T* xrowi;
	typename Part_type::Container_type::submatrix_type subA;
	pair<unsigned int, unsigned int> ij, toij;
	unsigned int to;
	vector<T> empty_v;
	
	for(parts_internal_iterator it = pcontainer_parts.begin(); 
	    it !=pcontainer_parts.end(); 
	    ++it) {
	  Part_type* lp = *it;
	  //if this column of blocks and down from the current one
	  if(lp->GetJ() == _args.fourth && lp->GetI() > _args.third){
	    if(_temp[0] != T(0)){
	      //rank one update code
	      ColumnMatrix L(mtl::columns(lp->part_data));
	      typename ColumnMatrix::iterator columni = L.begin() + _args.fifth;
	      
	      typename DenseMatrix::submatrix_type::iterator ii;
	      typename DenseMatrix::submatrix_type::OneD::iterator jj, jjend;
	      typename ColumnMatrix::OneD::iterator yycolumni;
	      
	      /*
		if(myid == 7){
		cout<<"PROBLEM"<<endl;
		for(int k=0;k<lp->GetnColumns() - _args.fifth;++k){
		cout<<_temp[k]<<" ";
		}
		cout<<endl;
		}
	      */

	      if(columni == L.end()) break;
	      //if (j < MM - 1)
	      mtl::scale(*columni, T(1) / _temp[0]);    /* update column under the pivot */
	      subA = lp->part_data.sub_matrix(0, lp->GetnRows(), _args.fifth + 1, lp->GetnColumns());
	      yycolumni=(*columni).begin();
	      for (ii = subA.begin(); ii != subA.end(); ++ii) {
		jj = (*ii).begin(); jjend = (*ii).end();
		xrowi = &_temp[0];++xrowi;
		for (; jj != jjend; ++jj,++xrowi)
		  *jj -= (*xrowi) * (*yycolumni);
		++yycolumni;
	      }
	    }
	    if((_args.fifth + 1) == lp->GetnColumns() || _temp[0] == 0){
	      //here A11,A21 is factorized; send A21to the right now;
	      pid2ij(this->myid, ij);
	      toij = ij;
	      //send the pivot information to the left and right of the current 
	      //processor
	      ARGS_SENDRIGHT sr_args(lp->GetnRows(),lp->GetnColumns(),lp->GetI(),lp->GetJ());
	      for(int s =0;s < layout.second; ++s){
		if(s != ij.second){
		  toij.second = s;
		  ij2pid(to,toij);
		  stapl::async_rmi(to,this->getHandle(),
				   &this_type::__SendRightNP,
				   sr_args,*(lp->GetVectorData()));
		}
		else{
		  __SendRightNP(sr_args,empty_v);
		}
	      }
	    }
	  }
	}//end for all parts
      }

      void _factorizeA11A21NP(unsigned int _i, unsigned int _j, unsigned int partid, vector<unsigned int>& ipvt){
	//this code is copy paste from MTL lu_factor algorithm
	//I need to do this to overlap the computation with communication
	typedef typename Part_type::Container_type DenseMatrix;
	typedef typename mtl::rows_type<DenseMatrix>::type RowMatrix;
	typedef typename mtl::columns_type<DenseMatrix>::type ColumnMatrix;
	typedef typename mtl::triangle_view<ColumnMatrix, mtl::lower>::type Lower;
	typedef typename mtl::triangle_view<RowMatrix, mtl::unit_upper>::type Unit_Upper;
	typedef typename mtl::triangle_view<ColumnMatrix, mtl::unit_lower>::type Unit_Lower;
	typedef typename DenseMatrix::size_type sizet;
	int info = 0;
	pair<unsigned int, unsigned int> ij, toij;
	typedef _StaplPenta<unsigned int,unsigned int,unsigned int,unsigned int,unsigned int> ARGS;
	typedef _StaplQuad<unsigned int,unsigned int,unsigned int,unsigned int> ARGS_SENDRIGHT;
	unsigned int to;
	vector<T> _temp;
	Part_type* p;

	ipvt.clear();
	p = this->GetPart(partid);
	//cout<<"factorizing ..."<<endl;
	//mtl::print_all_matrix(p->part_data);

	ipvt.resize(p->GetnRows());
	sizet j, jp, MM = p->part_data.nrows(), NN = p->part_data.ncols();
	valarray<T> _row(NN);

	Lower D(mtl::columns(p->part_data));
	Unit_Upper U(mtl::rows(p->part_data));
	Unit_Lower L(mtl::columns(p->part_data));
	//mtl::dense1D<T> c(MM), r(NN);
	typename DenseMatrix::submatrix_type subA;
	
	typename Lower::iterator dcoli = D.begin();
	typename Unit_Upper::iterator rowi = U.begin();
	typename Unit_Upper::OneD::iterator xrowi;

	typename Unit_Lower::iterator columni = L.begin();
	typename Unit_Lower::iterator trowi,tcolumni;
	typename Unit_Lower::OneD::iterator ycolumni;

	for (j = 0; j < min(MM , NN); ++j, ++dcoli, ++rowi, ++columni) {
	  
	  //jp = mtl::max_abs_index(*dcoli);		   /* find pivot */
	  jp = j;
	  ipvt[j] = jp;

	  if ( p->part_data(jp, j) != T(0) ) {	  /* make sure pivot isn't zero */
	    if (jp != j)
	      mtl::swap(mtl::rows(p->part_data)[j], mtl::rows(p->part_data)[jp]); /* swap the rows */
	    if (j < MM - 1)
	      mtl::scale(*columni, T(1) / p->part_data(j,j));    /* update column under the pivot */
	  } else {
	    for(int s =0;s < stapl::get_num_threads(); ++s){
	      if(s != this->myid)
		stapl::async_rmi(s,this->getHandle(),
			       &this_type::__LU_SetDoneNP);
	    }
	    done_flag=true;
	    info = j + 1;
	  }

	  if(done_flag == false ||(done_flag==true && j != 0)){
	    //here I know the pivot; and the row corresponding to it so broadcast to 
	    //al other blocks in this column	  
	    pid2ij(this->myid, ij);
	    toij = ij;
	    //send the pivot information to the left and right of the current 
	    //processor
	    ARGS args(p->GetnRows(),p->GetnColumns(),p->GetI(),p->GetJ(),j);
	    _row[0] = p->part_data(j,j);//this can be optimized when this is T(0)
	    xrowi = (*rowi).begin();
	    for(int k=j+1;k<NN;++k,++xrowi){
	      _row[k - j] = *xrowi;
	    }
	    
	    for(int s =0;s < layout.first; ++s){
	      if(s != ij.first){
		toij.first = s;
		ij2pid(to,toij);
		stapl::async_rmi(to,this->getHandle(),
			       &this_type::rank_one_update_columnNP,
				 args,_row);
	      }
	    }
	    //rank one update the current block
	    if (j < min(MM , NN)) {
	      if(done_flag == false){
		subA = p->part_data.sub_matrix(j+1, MM, j+1, NN);
		/* TODO: Better to have an adaptor here -- p->part_data.L. */
		//there is a copy involved here that I don't like
		//mtl::copy(*columni, c);  
		//mtl::copy(*rowi, r);  /* translate to submatrix coords */
		//mtl::rank_one_update(subA, mtl::scaled(c, T(-1)), r); /* update the submatrix */
		typename DenseMatrix::submatrix_type::iterator ii;
		typename DenseMatrix::submatrix_type::OneD::iterator jj, jjend;
		ycolumni=(*columni).begin();
		for (ii = subA.begin(); ii != subA.end(); ++ii) {
		  jj = (*ii).begin(); jjend = (*ii).end();
		  xrowi = (*rowi).begin();
		  for (; jj != jjend; ++jj,++xrowi){
		    //cout<<*jj<<"-="<<*ycolumni<<" * "<<*xrowi<<" ";
		    *jj -= (*xrowi) * (*ycolumni);
		  }
		  //cout<<endl;
		  ++ycolumni;
		}
	      }
	      
	      //rank one update all the other blocks in the column 
	      //from this one down
	      for(parts_internal_iterator it = pcontainer_parts.begin(); 
		  it !=pcontainer_parts.end(); 
		  ++it) {
		Part_type* lp = *it;
		//if this column of blocks and down from the current one
		if(lp->GetJ() == p->GetJ() && lp->GetI() > p->GetI()){
		  if(done_flag == false){
		    //rank one update code
		    ColumnMatrix LL(mtl::columns(lp->part_data));
		    typename ColumnMatrix::iterator ccolumni = LL.begin() + j;
		    typename DenseMatrix::submatrix_type::iterator ii;
		    typename DenseMatrix::submatrix_type::OneD::iterator jj, jjend;
		    typename ColumnMatrix::OneD::iterator yycolumni;
		    
		    if(ccolumni == LL.end()) break;
		    //if (j < MM - 1)
		    mtl::scale(*ccolumni, T(1) / p->part_data(j,j));    /* update column under the pivot */
		    
		    //cout<<"factorizing DOWN..."<<endl;
		    //mtl::print_all_matrix(lp->part_data);
		    
		    subA = lp->part_data.sub_matrix(0, lp->GetnRows(), j + 1,lp->GetnColumns());
		    
		    //cout<<"subA DOWN..."<<endl;
		    //mtl::print_all_matrix(subA);
		    //cout<<"------------------------"<<endl;
		  
		    yycolumni=(*ccolumni).begin();
		    for (ii = subA.begin(); ii != subA.end(); ++ii) {
		      jj = (*ii).begin(); jjend = (*ii).end();
		      xrowi = (*rowi).begin();
		      for (; jj != jjend; ++jj,++xrowi){
			//cout<<"DOWN"<<*jj<<"-="<<*yycolumni<<" * "<<*xrowi<<" ";
			*jj -= (*xrowi) * (*yycolumni);
		      }
		      ++yycolumni;
		    }
		  }
		  if((j+1) == lp->GetnColumns() || done_flag == true){
		    //here A11,A21 is factorized; send A21to the right now;
		    pid2ij(this->myid, ij);
		    toij = ij;
		    //send the pivot information to the left and right of the current 
		    //processor
		    ARGS_SENDRIGHT args(lp->GetnRows(),lp->GetnColumns(),lp->GetI(),lp->GetJ());
		    for(int s =0;s < layout.second; ++s){
		      if(s != ij.second){
			toij.second = s;
			ij2pid(to,toij);
			stapl::async_rmi(to,this->getHandle(),
					 &this_type::__SendRightNP,
					 args,*(lp->GetVectorData()));
		      }
		      else{
			__SendRightNP(args,_temp);
		      }
		    }
		  }
		}
	      }//end for all parts
	    }
	  }
	}
	ipvt[j] = j;
      }//end factorizeA11A21

      void __LU_SetDoneNP(){
	done_flag =true;
      }

      void LU_NP(){
	unsigned int k;
	unsigned int ib;
	unsigned int end;
	unsigned int nb=dist.Get_nb();
	PARTID partid;
	Part_type* p;
	vector<unsigned int> pivots;
	done_flag = false;
	flagsA12.resize(this->dist.BQ);
	flagsA21.resize(this->dist.BP);
	A12.resize(this->dist.BQ);
	A21.resize(this->dist.BP);
	ib = 0;
	while(ib<N && done_flag == false){

	  for(k=0;k<this->dist.BQ;++k){
	    flagsA12[k] = false;
	    //here I have to free up some space;
	    //if(A12[k] != NULL && !this->dist.IsLocal(A12[k]->GetI(),A12[k]->GetJ(),partid))
	    //delete A12[k];
	    A12[k] = NULL;
	  }
	  for(k=0;k<this->dist.BP;++k){
	    flagsA21[k] = false;
	    //here I have to free up some space;
	    //if(A21[k] != NULL && !this->dist.IsLocal(A21[k]->GetI(),A21[k]->GetJ(),partid))
	    //delete A21[k];
	    A21[k] = NULL;
	  }
	  stapl::rmi_fence();
	  stapl::rmi_fence();
	  end = ((ib+nb-1) < N)?ib+nb-1:N;//take the minimum between ib+mb-1 and N
	  if(dist.IsLocal(ib,ib,partid)){

	    //LU current block and the blocks on the same column(Steps 1..4 and 10)
	    _factorizeA11A21NP(ib,ib,partid,pivots);

	    p = this->GetPart(partid);
	    //pivots.resize(p->GetnColumns());
	    //mtl::lu_factor(p->part_data, pivots);
	    
	    //broadcats the pivot information to the processors that owns
	    //the other columns and perform the swap (step5,6)

	    // broadcastSwapInfoLeftRight(ib,ib,pivots);

	    //broadcast LL right and perform LL^-1 * A 1,2 (Step 7,8)
	    //this step triggers also steps 9,10,11
	    broadcastMultiplyLLRightNP(p,ib);
	  }//if I own current block
	  //row_id+=dist.Get_mb();
	  ib+=nb;
	  stapl::rmi_fence();
	  //if(myid == 0){
	  //for (int i = 0; i < M; ++i) {
	  //  for (int j = 0; j < N; ++j) {
	  //cout<<this->GetElement(i,j)<<" ";
	  //  }
	  //  cout<<endl;
	  //}
	  //}
	  //stapl::rmi_fence();
	}//for all columns
      }//end LUNP decomposition
      
      
      
      //@}

      /**@name Input/Output */
      //@{
      /**@brief 
       *Display Distribution Info.
       */
      //IO methods
      void DisplayDistributionInfo(){
	this->dist.DisplayDistributionInfo();
      }
      //@}
      //=======================================
      //PContainerParts bookkeeping 
      //=======================================
      public:
    
      /*add pContainer parts 
       *\b LOCAL
       */
    
      /*Method to add an empty part to the pContainer
	 Calls the methods in the Base
      */
      virtual int AddPart() {
	return BasePContainer_type::AddPart();
      }
      
      /*init boundary info for multiple parts
       */
      virtual void InitBoundaryInfo(PARTID _partid, Location _before, Location _after) {
	this->dist.InitBoundaryInfo(_partid, _before, _after);
      }
      
      /*Setup bdry info after redistribution
       */
      void SetPartBdry(PARTID _id, const Partbdryinfo_type& _bd) {
	this->SetPartBdry(_id,_bd);
      }

      void MergeSubContainer() {
	cout<<"\n This is -in MergeSubContainer ...... NOT YET IMPLEMENTED"<<endl;
      }

      int SetSubContainer(const vector<pContainerPart_type>  _ct) {
	printf("\n This is -SetSubContainer- method from pMatrixBCD ...... NOT YET IMPLEMENTED");
	return ERROR;
      }
      
      void BuildSubContainer( const vector<Matrix_Range_type>&,
			      vector<pContainerPart_type>&){
				printf("\n This is -BuildSubContainer- method from pMatrixBCD ...... NOT YET IMPLEMENTED");
			      }
      private:

    };//end of pMatrixBCD

} //end namespace stapl
//@}
#endif
