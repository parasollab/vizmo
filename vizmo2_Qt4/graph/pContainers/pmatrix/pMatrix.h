/*!
  \file pMatrix.h
  \author Gabriel Tanase	
  \date Jan. 25, 04
  \ingroup stapl
  \brief pMatrix class

  Internal to stapl developer. 
*/
#ifndef PMATRIX_H
#define PMATRIX_H
#include <vector>
#include "BasePContainer.h"
#include "mtl/mtl.h"
#include "MatrixPart.h"
#include "MatrixDistribution.h"
#include "valarray"
#ifdef STAPL_ATLAS
#include "../../../tools/mtl/mtl_bench/mtl-spec-atlas/mtl_specializations.h"
#endif


/**
 * @ingroup pContainers
 * @defgroup pmatrix Parallel Matrix
 * @{
 * 
 **/
namespace stapl {

  //tags
  /**stapl_row_major tag. 
   *Class used as a tag to specify that the matrix is row major.
   */
  struct stapl_row_major{
    public:
     typedef mtl::row_major stapl_matrix_orientation;
    pair<unsigned int, unsigned int> GetProcessorsLayout(){
      return pair<unsigned int, unsigned int>(stapl::get_num_threads(),1);	
    }
  };

  struct stapl_column_row_major{
    public:
     typedef mtl::row_major stapl_matrix_orientation;

    pair<unsigned int, unsigned int> GetProcessorsLayout(){	
      return pair<unsigned int, unsigned int>(1,stapl::get_num_threads());	
    }
  };

  /**stapl_column_major tag.
   *Class used as a tag to specify that the matrix is column major.
   */
  struct stapl_column_column_major{
    public:
     typedef mtl::column_major stapl_matrix_orientation;
    pair<unsigned int, unsigned int> GetProcessorsLayout(){	
      return pair<unsigned int, unsigned int>(1,stapl::get_num_threads());	
    }    
  };

  /**stapl_block tag.
   *Class used as a tag to specify that the matrix is block cyclic stored.
   *This class assumes that the processors are organized as a mesh for better efficiency.
   */
  struct stapl_block_row{
    public:
     typedef mtl::row_major stapl_matrix_orientation;     

    /**@brief 
     *Method that computes the processors layout. Right is a static mapping
     *from threads number to a layout. It has to be extended in the future 
     *to be smarter.
     *@return pair<unsigned int, unsigned int> (threads on x, threads on y) 
     */
    pair<unsigned int, unsigned int> GetProcessorsLayout(){
      //this method has to become smarter; 
      //it is suppose to return the processors logical layout
      int nprocs = stapl::get_num_threads();
      switch (nprocs) {
      case 1 : return pair<unsigned int, unsigned int>(1,1);
      case 2 : return pair<unsigned int, unsigned int>(2,1);
      case 4 : return pair<unsigned int, unsigned int>(2,2);
      case 8 : return pair<unsigned int, unsigned int>(4,2);
      case 9 : return pair<unsigned int, unsigned int>(3,3);
      case 12 : return pair<unsigned int, unsigned int>(4,3);
      case 16 : return pair<unsigned int, unsigned int>(4,4);  
      case 25 : return pair<unsigned int, unsigned int>(5,5);  
      case 32 : return pair<unsigned int, unsigned int>(8,4);
      case 64 : return pair<unsigned int, unsigned int>(8,8);
      case 128 : return pair<unsigned int, unsigned int>(16,8);
      default: {
#ifdef STAPL_DEBUG
	cout<<"One dimensional layout choosen by default"<<endl;
#endif
	return pair<unsigned int, unsigned int>(nprocs,1);
      }
      }
    }
  };

  struct stapl_block_row_column{
    public:
     typedef mtl::column_major stapl_matrix_orientation;     

    /**@brief 
     *Method that computes the processors layout. Right is a static mapping
     *from threads number to a layout. It has to be extended in the future 
     *to be smarter.
     *@return pair<unsigned int, unsigned int> (threads on x, threads on y) 
     */
    pair<unsigned int, unsigned int> GetProcessorsLayout(){
      //this method has to become smarter; 
      //it is suppose to return the processors logical layout
      int nprocs = stapl::get_num_threads();
      switch (nprocs) {
      case 1 : return pair<unsigned int, unsigned int>(1,1);
      case 2 : return pair<unsigned int, unsigned int>(2,1);
      case 4 : return pair<unsigned int, unsigned int>(2,2);
      case 8 : return pair<unsigned int, unsigned int>(4,2);
      case 9 : return pair<unsigned int, unsigned int>(3,3);
      case 12 : return pair<unsigned int, unsigned int>(4,3);
      case 16 : return pair<unsigned int, unsigned int>(4,4);  
      case 25 : return pair<unsigned int, unsigned int>(5,5);  
      case 32 : return pair<unsigned int, unsigned int>(8,4);
      case 64 : return pair<unsigned int, unsigned int>(8,8);
      case 128 : return pair<unsigned int, unsigned int>(16,8);
      default: {
#ifdef STAPL_DEBUG
	cout<<"One dimensional layout choosen by default"<<endl;
#endif
	return pair<unsigned int, unsigned int>(nprocs,1);
      }
      }
    }
  };


  struct stapl_block_column{
    public:
     typedef mtl::column_major stapl_matrix_orientation;     

    /**@brief 
     *Method that computes the processors layout. Right is a static mapping
     *from threads number to a layout. It has to be extended in the future 
     *to be smarter.
     *@return pair<unsigned int, unsigned int> (threads on x, threads on y) 
     */
    pair<unsigned int, unsigned int> GetProcessorsLayout(){
      //this method has to become smarter; 
      //it is suppose to return the processors logical layout
      int nprocs = stapl::get_num_threads();
      switch (nprocs) {
      case 1 : return pair<unsigned int, unsigned int>(1,1);
      case 2 : return pair<unsigned int, unsigned int>(1,2);
      case 4 : return pair<unsigned int, unsigned int>(2,2);
      case 8 : return pair<unsigned int, unsigned int>(2,4);
      case 9 : return pair<unsigned int, unsigned int>(3,3);
      case 12 : return pair<unsigned int, unsigned int>(3,4);
      case 16 : return pair<unsigned int, unsigned int>(4,4);  
      case 25 : return pair<unsigned int, unsigned int>(5,5);  
      case 32 : return pair<unsigned int, unsigned int>(4,8);
      case 64 : return pair<unsigned int, unsigned int>(8,8);
      case 128 : return pair<unsigned int, unsigned int>(16,8);
      default: {
#ifdef STAPL_DEBUG
	cout<<"One dimensional layout choosen by default"<<endl;
#endif
	return pair<unsigned int, unsigned int>(nprocs,1);
      }
      }
    }
  };


  //template<class T,class Shape = mtl::rectangle<>, class Storage = mtl::dense<>, class Orientation = mtl::row_major>

  /**
   *pMatrix class definition
   *pMatrix implements the parallel matrix container. There are two templates that has 
   *to be specified at the instantiation: 
   * T the type of elements of the pMatrix
   * MatrixOrientation the tag defining the orientation of the matrix.
   * \nosubgrouping 
   */
  template<class T,class MatrixOrientation = stapl_row_major>
    class pMatrix : 
    public BasePContainer<MatrixPart<T,mtl::rectangle<>,mtl::dense<>,typename MatrixOrientation::stapl_matrix_orientation>, 
    MatrixDistribution,no_trace, random_access_iterator_tag, stapl_element_tag > {

      public:
      //=======================================
      //types
      //=======================================
      ///The type of the elements of the pMatrix.
      typedef T value_type;

      //required by mtl; not used right now by stapl
      typedef mtl::rectangle<> Shape;
      typedef mtl::dense<>     Storage;

      ///Matrix orientation.
      typedef typename MatrixOrientation::stapl_matrix_orientation Orientation;

      typedef typename MatrixPart<value_type,Shape,Storage,Orientation>::Element_Set_type Matrix_Range_type;
      ///Distribution defines
      typedef MatrixDistribution Distribution;
  
      ///Matrix Part.
      typedef  MatrixPart<value_type,Shape,Storage,Orientation> pContainerPart_type;
      ///BasePContainer. The base class for pMatrix.
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
      typedef pMatrix<T,MatrixOrientation>                this_type;

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


      public:
      //=======================================
      //constructors 
      //=======================================
       /**@name Constructors and Destructor */
      //@{
      /**@brief  Default Constructor
       */
      pMatrix() {
        this->register_this(this);
        rmi_fence(); 
      }

      /**@brief  Specific Constructor; A default processor layout is chosen
       *@param unsigned int _m the number of rows.
       *@param unsigned int _n the number of columns.
       */
      pMatrix(unsigned int _m, unsigned int _n){
	int i,j;
	Location l;
	M=_m; N=_n;
	layout = MatrixOrientation().GetProcessorsLayout();
	vector<MatrixDistributionInfo> di;
	MatrixDistributionInfo dentry,local;

	int nrp = _m / layout.first; //number rows per processor
	int ncp = _n / layout.second;//number columns per processor
	
	//set for all procs but the last row/last col
	for(i=0;i<layout.first - 1;++i){
	  for(j=0;j<layout.second - 1;++j){
	    dentry.SetI(i*nrp);dentry.SetJ(j*ncp); 
	    dentry.SetnRows(nrp);dentry.SetnColumns(ncp); 
	    l.SetPid(i*layout.second+j);l.SetPartId(0);
	    dentry.SetLocation(l);
	    di.push_back(dentry);
	    if(i*layout.second+j == this->myid) local = dentry;
	  }
	}
	//last row but last element
	for(j=0;j<layout.second - 1;++j){
	  dentry.SetI((layout.first - 1)*nrp); dentry.SetJ(j*ncp); 
	  dentry.SetnRows(_m - (layout.first - 1)*nrp);dentry.SetnColumns(ncp); 
	  l.SetPid((layout.first - 1) * layout.second+j);l.SetPartId(0);
	  dentry.SetLocation(l);
	  di.push_back(dentry);
	  if((layout.first - 1) * layout.second+j == this->myid) local = dentry;
	}
	//last Column but last element
	for(i=0;i<layout.first - 1;++i){
	  dentry.SetI(i*nrp);dentry.SetJ((layout.second - 1)*ncp); 
	  dentry.SetnRows(nrp);dentry.SetnColumns(_n - (layout.second - 1)*ncp); 
	  l.SetPid(i*layout.second + layout.second - 1);l.SetPartId(0);
	  dentry.SetLocation(l);
	  di.push_back(dentry);
	  if(i*layout.second + layout.second - 1 == this->myid) local = dentry;
	}
	//last row last column last element
	dentry.SetI((layout.first-1)*nrp);dentry.SetJ((layout.second - 1)*ncp); 
	dentry.SetnRows(_m - (layout.first - 1)*nrp);dentry.SetnColumns(_n - (layout.second - 1)*ncp); 
	l.SetPid(layout.first * layout.second -1);l.SetPartId(0);
	dentry.SetLocation(l);
	di.push_back(dentry);
	if(layout.first * layout.second -1 == this->myid) local = dentry;

	for(i=0;i<this->pcontainer_parts.size();++i){
	  delete this->pcontainer_parts[i];
	}
	this->pcontainer_parts.clear();
	//alocate local part according to local	
	Part_type *p = new Part_type(local.GetnRows(),
				     local.GetnColumns(),
				     local.GetI(),
				     local.GetJ(),
				     local.GetLocation().partid());

	this->pcontainer_parts.push_back(p);
	this->register_this(this);
        this->dist.InitializeMatrixDistribution(_m,_n,di);
        stapl::rmi_fence();
      }
      
      /**@brief  Constructor where the user can specify the processors layout.
       *@param _m the rows number.
       *@param _n the columns number.
       *@param _layout the processor layout.
       */
      pMatrix(unsigned int _m, unsigned int _n,const pair<unsigned int, unsigned int>& _layout){
	int i,j;
	Location l;
	M=_m; N=_n;
	layout = _layout;

	vector<MatrixDistributionInfo> di;
	MatrixDistributionInfo dentry,local;

	int nrp = _m / _layout.first; //number rows per processor
	int ncp = _n / _layout.second;//number columns per processor
	
	//set for all procs but the last row/last col
	for(i=0;i<_layout.first - 1;++i){
	  for(j=0;j<_layout.second - 1;++j){
	    dentry.SetI(i*nrp);dentry.SetJ(j*ncp); 
	    dentry.SetnRows(nrp);dentry.SetnColumns(ncp); 
	    l.SetPid(i*_layout.second+j);l.SetPartId(0);
	    dentry.SetLocation(l);
	    di.push_back(dentry);
	    if(i*_layout.second+j == this->myid) local = dentry;
	  }
	}
	//last row but last element
	for(j=0;j<_layout.second - 1;++j){
	  dentry.SetI((_layout.first - 1)*nrp); dentry.SetJ(j*ncp); 
	  dentry.SetnRows(_m - (_layout.first - 1)*nrp);dentry.SetnColumns(ncp); 
	  l.SetPid((_layout.first - 1) * _layout.second+j);l.SetPartId(0);
	  dentry.SetLocation(l);
	  di.push_back(dentry);
	  if((_layout.first - 1) * _layout.second+j == this->myid) local = dentry;
	}
	//last Column but last element
	for(i=0;i<_layout.first - 1;++i){
	  dentry.SetI(i*nrp);dentry.SetJ((_layout.second - 1)*ncp); 
	  dentry.SetnRows(nrp);dentry.SetnColumns(_n - (_layout.second - 1)*ncp); 
	  l.SetPid(i*_layout.second + _layout.second - 1);l.SetPartId(0);
	  dentry.SetLocation(l);
	  di.push_back(dentry);
	  if(i*_layout.second + _layout.second - 1 == this->myid) local = dentry;
	}
	//last row last column last element
	dentry.SetI((_layout.first-1)*nrp);dentry.SetJ((_layout.second - 1)*ncp); 
	dentry.SetnRows(_m - (_layout.first - 1)*nrp);dentry.SetnColumns(_n - (_layout.second - 1)*ncp); 
	l.SetPid(_layout.first * _layout.second -1);l.SetPartId(0);
	dentry.SetLocation(l);
	di.push_back(dentry);
	if(_layout.first * _layout.second -1 == this->myid) local = dentry;

	for(i=0;i<this->pcontainer_parts.size();++i){
	  delete this->pcontainer_parts[i];
	}
	this->pcontainer_parts.clear();
	//alocate local part according to local	
	Part_type *p = new Part_type(local.GetnRows(),
				     local.GetnColumns(),
				     local.GetI(),
				     local.GetJ(),
				     local.GetLocation().partid());

	this->pcontainer_parts.push_back(p);
	this->register_this(this);
        this->dist.InitializeMatrixDistribution(_m,_n,di);
        stapl::rmi_fence();
      }

      /**@brief  Constructor with a specified distribution.
       *@param unsigned int _m the number of rows.
       *@param unsigned int _n the number of columns.
       *@param const MatrixDistributionInfo_type the user specified distribution.
       */
      pMatrix(unsigned int _m, unsigned int _n, const MatrixDistributionInfo_type& _distinfo) {
	int i;
	for(i=0;i<this->pcontainer_parts.size();++i)
	delete this->pcontainer_parts[i];
	this->pcontainer_parts.clear();
	for(i=0;i<_distinfo.size();++i){
	  if(_distinfo[i].GetLocation().locpid() == this->myid){
	    Part_type* p = new Part_type(_distinfo[i].GetnRows(),
					 _distinfo[i].GetnColumns(),
					 _distinfo[i].GetI(),
					 _distinfo[i].GetJ(),
					 _distinfo[i].GetLocation().partid());
	    this->pcontainer_parts.push_back(p);
	  }
	}
        this->dist.InitializeMatrixDistribution(_m,_n,_distinfo);
        this->register_this(this);
        rmi_fence(); 
      }
      
      /**@brief  Copy constructor 
       */
      pMatrix(const pMatrix& _other) : BasePContainer_type(_other) {
	M = _other.M;
	N = _other.N;
        this->register_this(this);
        rmi_fence(); 
      }

      /**@brief  
       *Destructor
       */     
      ~pMatrix() {
        stapl::unregister_rmi_object( this->getHandle());
      }

      /**@brief 
       * Initialize the data of the pMatrix; The functor will be called for every index of
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

      /**@brief 
       *Define type for pMatrix.
       */
      void define_type(typer &t)  {
        cout<<"ERROR::Method define_type not yet implemented for pMatrix"<<endl;
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
	  p = this->GetPart(_args.third);
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
    
      virtual int _SetElement(const _StaplQuad<unsigned int, unsigned int,PARTID, value_type>& _args) {
	Part_type  *p;
	if(_args.third != INVALID_PART) {
	  p = GetPart(_args.third);
	  (*p)(_args.first - p->GetI() ,_args.second - p->GetJ()) = _args.fourth;
	  return OK; 
	}
	return ERROR;
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
	  stapl::sync_rmi(loc.locpid(),getHandle(),
			  &this_type::_SetElement,
			  _StaplQuad<unsigned int, unsigned int,PARTID, value_type>(_i,_j,loc.partid(),_t));
	}
      }
      //@}

      //=========================================================
      //algorithms provided by the pMatrix Class
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
      ///////////////////////////////////////////////////////////////////////////
      //   MATRIX MATRIX multiply
      ///////////////////////////////////////////////////////////////////////////
      template <class T1, class T2, class T3>
      inline void sequential_mult(T1& A, T2& B, T3& C){
#ifdef STAPL_ATLAS
	mtl::mult_atlas(A,B,C);
#else
	mtl::mult(A,B,C);
#endif
      }

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

	  mtl::mult(pA->part_data,
		    pB->part_data, 
		    pC->part_data.sub_matrix(row_from,row_to,col_from,col_to));
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
	  this->sequential_mult(pA->part_data,pB->part_data,pC->part_data);
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
	    this->sequential_mult(pA->part_data,pB->part_data,pC->part_data);
	  else
	    this->sequential_mult(pT->part_data,pB->part_data,pC->part_data);	  
	  if(this->nprocs != 1) 
	    _BB.UpShift2D();	  
	}
	return 1;
      }

      //////////////////////////////////////////////////////////////////////
      public:
      template <class MatrixRowsOperation>
      void _rowsOperation(valarray<T> _temp, _StaplPair<int,MatrixRowsOperation> _args){
	//cout<<"received on "<<this->myid<<endl;
	part_iterator iti = this->GetPart(0)->begin() + (_args.first - this->GetPart(0)->GetI());
	element_iterator eii,iend;
	T* eij = &_temp[0];
	iend = (*iti).end();
	for(eii = (*iti).begin(); eii != iend;++eii,++eij){
	  *eii = _args.second(*eii,*eij);
	}
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
	typedef _StaplPair<int,MatrixRowsOperation> ARGS;

	PARTID partidi,partidj;
	unsigned int firstjofi,firstjofj;
	part_iterator iti,itj;
	element_iterator eii, eij, iend,jend;

	ARRAY temp(1);
	int to = -1;//where to sent the row

	//cout<<"size of the part"<<this->GetPart(0)->GetVectorData()->size()<<endl;

	if(this->dist.IsLocalRow(_rowi,firstjofi,partidi)){
	  //if _rowi is local
	  if(this->dist.IsLocalRow(_rowj,firstjofj,partidj)){
	    //swap localy i and j
	    iti = this->GetPart(partidi)->begin() + (_rowi - this->GetPart(partidi)->GetI());
	    itj = this->GetPart(partidj)->begin() + (_rowj - this->GetPart(partidj)->GetI());
	    iend = (*iti).end();jend=(*itj).end();
	    eij = (*itj).begin();
	    for(eii = (*iti).begin(); eii != iend;++eii,++eij){
	      *eii = _func(*eii,*eij);
	    }
	  }
	}
	else{//rowi is not local; only J can be local; if true send it to the
	  //owner of i
	  if(this->dist.IsLocalRow(_rowj,firstjofj,partidj)){
	    temp.resize(this->GetPart(partidj)->GetnColumns());
	    iti = this->GetPart(partidj)->begin() + (_rowj - this->GetPart(partidj)->GetI());
	    copy((*iti).begin() , (*iti).end(), &temp[0]);
	    to = this->dist.LookupRow(_rowi,firstjofj).locpid();
	    //cout<<"sent to "<<to<<endl;
	    ARGS  _args(_rowi,_func);
	    //stapl::async_rmi(to,getHandle(),
			     //(void (this_type::*)(ARRAY,ARGS))&this_type::_rowsOperation,temp,_args);
	    //			     &this_type::_rowsOperation,temp,_args);
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
      void _replaceRow(valarray<T>& _temp, unsigned int _row){
	//cout<<"received on "<<this->myid<<endl;
	part_iterator iti = this->GetPart(0)->begin() + (_row - this->GetPart(0)->GetI());
	copy(&_temp[0], &_temp[_temp.size()], (*iti).begin());
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
	PARTID partidi,partidj;
	unsigned int firstjofi,firstjofj;
	part_iterator iti,itj;
	ARRAY temp(1);
	int to = -1;//where to sent the row
	unsigned int destrow;

	if(this->dist.IsLocalRow(_rowi,firstjofi,partidi)){
	  //if _rowi is local
	  if(this->dist.IsLocalRow(_rowj,firstjofj,partidj)){
	    //swap localy i and j
	    iti = this->GetPart(partidi)->begin() + (_rowi - this->GetPart(partidi)->GetI());
	    itj = this->GetPart(partidj)->begin() + (_rowj - this->GetPart(partidj)->GetI());
	    mtl::swap(*iti,*itj);
	    
	  }
	  else{
	    //send rowi to owner of _rowj
	    temp.resize(this->GetPart(partidi)->GetnColumns());
	    iti = this->GetPart(partidi)->begin() + (_rowi - this->GetPart(partidi)->GetI());
	    copy((*iti).begin() , (*iti).end(), &temp[0]);

	    to = this->dist.LookupRow(_rowj,firstjofi).locpid();
	    destrow = _rowj;
	  }
	}
	else{//rowi is not local; only J can be local; if true send it to the
	  //owner of i
	  if(this->dist.IsLocalRow(_rowj,firstjofj,partidj)){
	    temp.resize(this->GetPart(partidj)->GetnColumns());
	    iti = this->GetPart(partidj)->begin() + (_rowj - this->GetPart(partidj)->GetI());
	    copy((*iti).begin() , (*iti).end(), &temp[0]);
	    to = this->dist.LookupRow(_rowi,firstjofj).locpid();
	    destrow = _rowi;
	  }
	}
	stapl::rmi_fence();
	if(to != -1){
	  //cout<<"sent to "<<to<<endl;
	  stapl::async_rmi(to,getHandle(),&this_type::_replaceRow,temp,destrow);
	}
	stapl::rmi_fence();	
      }

      unsigned int GetRowId(iterator _it){
	Part_type *p = _it.GetPart();
	return _it - local_begin() + p.GetI(); 
      }

      //===============================
      // column sort derived methods
      //===============================
      void _shiftCyclicDown(vector<T>& _temp){	
	Part_type* p = this->GetPart(0);
	vector<T>* localdata = p->GetVectorData();
	std::copy(_temp.begin(),_temp.end(),localdata->begin());
	//return 0;
      }

      public: 
      void ShiftCyclicDown(int _pos){
	//thread id to whom I sent and from which I receive
	int to;
	int s;
	Part_type *p = this->GetPart(0);
	vector<T> *localdata = p->GetVectorData();
	vector<T> temp(_pos);
	//shift the local data with _pos positions 
	//first copy the last _pos entries from local data into temp;
	s = localdata->size() - _pos;
	std::copy(localdata->begin() + s,localdata->end(),temp.begin());
	//now shift data locally
	std::copy(localdata->rbegin()+_pos,localdata->rend(),localdata->rbegin());
	//compute the dest thread
	to = (this->myid + 1) % this->nprocs;
	stapl::rmi_fence();
	stapl::async_rmi(to,getHandle(),&this_type::_shiftCyclicDown,temp);
	stapl::rmi_fence();
      }

      void _shiftCyclicUp(int _pos, vector<T>& _temp){	
	Part_type* p = this->GetPart(0);
	vector<T>* localdata = p->GetVectorData();
	std::copy(_temp.begin(),_temp.end(),localdata->begin()+_pos);
	//return 0;
      }

      public: 
      void ShiftCyclicUp(int _pos){
	//thread id to whom I sent and from which I receive
	int to;
	int s;
	Part_type *p = this->GetPart(0);
	vector<T> *localdata = p->GetVectorData();
	vector<T> temp(_pos);
	//shift the local data with _pos positions 
	//first copy the last _pos entries from local data into temp;
	std::copy(localdata->begin(),localdata->begin()+_pos,temp.begin());
	//now shift data locally
	std::copy(localdata->begin()+_pos,localdata->end(),localdata->begin());
	//compute the dest thread
	to = (this->myid + this->nprocs - 1) % this->nprocs;
	stapl::rmi_fence();
	stapl::async_rmi(to,getHandle(),
			 &this_type::_shiftCyclicUp,
			 localdata->size() - _pos,
			 temp);
	stapl::rmi_fence();
      }


      void _transposeReshape(int source, vector<T>& _temp){	
	int s;
	Part_type* p = this->GetPart(0);
	vector<T>* localdata = p->GetVectorData();
	s = localdata->size() / this->nprocs;
	std::copy(_temp.begin(),_temp.end(),localdata->begin()+source*s);
	//return 0;
      }

      public: 
      void TransposeReshape(){
	int i,s;
	int localsize;
	vector<vector<T> > _buf(this->nprocs);
	Part_type *p = this->GetPart(0);
	vector<T> *localdata = p->GetVectorData();
	typename vector<T>::iterator it,itend;
	localsize = localdata->size();
	//allocate space for the data that will be sent
	for(s=0;s<this->nprocs;++s){
	  _buf[s].resize(localsize / this->nprocs);
	}
	int idx=0;
	it = localdata->begin();
	itend = localdata->end();
	while(it < itend){
	  for(i=0;i<this->nprocs;++i){
	    _buf[i][idx] = *it;
	    ++it;
	  }
	  ++idx;
	}
	//make sure that data is copied inside buf
	stapl::rmi_fence();
	for(s=0;s<this->nprocs;++s){
	  stapl::async_rmi(s,getHandle(),
			   &this_type::_transposeReshape,
			   this->myid, 
			   _buf[s]);
	}
	stapl::rmi_fence();
      }


      void _reshapeTranspose(int source, vector<T>& _temp){	
	int i,s;
	Part_type* p = this->GetPart(0);
	vector<T>* localdata = p->GetVectorData();
	s = localdata->size() / this->nprocs;
	typename vector<T>::iterator it,itend,lit;
	lit = localdata->begin();
	it = _temp.begin();itend = _temp.end();
	std::advance(lit,source);
	while(it != itend){
	  *lit = *it++;
	  std::advance(lit,this->nprocs);
	}
	//std::copy(_temp.begin(),_temp.end(),localdata->begin()+source*s);
	//return 0;
      }

      public: 
      void ReshapeTranspose(){
	int d,i,s;
	int localsize;
	vector<vector<T> > _buf(this->nprocs);
	Part_type *p = this->GetPart(0);
	vector<T> *localdata = p->GetVectorData();
	typename vector<T>::iterator it,itend;
	localsize = localdata->size();
	//allocate space for the data that will be sent
	s=localsize / this->nprocs;
	for(i=0;i<this->nprocs;++i){
	  _buf[i].resize(s);
	}
	it = localdata->begin();
	itend = localdata->end();
	d=0;
	while(it < itend){
	  for(i=0;i<s;++i){
	    _buf[d][i] = *it;
	    ++it;
	  }
	  ++d;
	}
	//make sure that data is copied inside buf
	stapl::rmi_fence();
	for(i=0;i<this->nprocs;++i){
	  stapl::async_rmi(i,getHandle(),
			   &this_type::_reshapeTranspose,
			   this->myid, 
			   _buf[i]);
	}
	stapl::rmi_fence();
      }

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
	printf("\n This is -SetSubContainer- method from pMatrix ...... NOT YET IMPLEMENTED");
	return ERROR;
      }
      
      void BuildSubContainer( const vector<Matrix_Range_type>&,
			      vector<pContainerPart_type>&){
				printf("\n This is -BuildSubContainer- method from pMatrix ...... NOT YET IMPLEMENTED");
			      }
      private:

    };//end of pMatrix

} //end namespace stapl
//@}
#endif
