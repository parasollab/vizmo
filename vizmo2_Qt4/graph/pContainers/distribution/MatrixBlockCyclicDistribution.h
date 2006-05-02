/*!
	\file MatrixBlockCyclicDistribution.h
	\author Gabriel Tanase	
	\date MAr. 20, 04
	\ingroup stapl
	\brief Matrix  Block Cyclic Distribution class : 
	Maps from global indices (i,j) to processor
	containing the indices
 	Internal to stapl developer. 
*/
#ifndef MATRIXBLOCKCYCLICDISTRIBUTION_H
#define MATRIXBLOCKCYCLICDISTRIBUTION_H

#include <runtime.h>
#include "rmitools.h"

#include "DistributionDefines.h"
#include "BaseDistribution.h"
#include <assert.h>
/**
 * @ingroup pmatrix
 * @{
 */
namespace stapl {
  
  /**
   * Matrix distribution info. For each part of the pMatrix we will store an instance of this 
   * class. Basically we encapsulate here information about the corresponding part like
   * the position in the global matrix, size and location information to allow us fast Lookup() 
   * function.
   */
  class BlockInfo {
  
  public:
    //The indices for the first element that we have in a part
    unsigned int bi,bj;
    unsigned int I;
    unsigned int J;
    unsigned int nRows;
    unsigned int nColumns;
    //location where the part lives(local part id, processor id)
    Location loc;

    /**
     *Default constructor.
     */
    BlockInfo(){}

    /**
     *Constructor
     *@param _i Upper left corner row in the global matrix.
     *@param _j Upper left corner column in the global matrix.
     *@param _m rows number.
     *@param _n columns number.
     *@param Location the thread which will own the part.
     */
    BlockInfo(unsigned int _i, 
	      unsigned int _j, 
	      unsigned int _m,
	      unsigned int _n,
	      Location _l) : I(_i),J(_j),nRows(_m),nColumns(_n)
    {}

    /**
     *Get the row corresponding to the upper left corner of the corresponding part.
     *@return unsigned int row identifier.
     */
    unsigned int GetI() const{
      return I;
    }
    /**
     *Get the column corresponding to the upper left corner of the corresponding part.
     *@return unsigned int column identifier.
     */
    unsigned int GetJ() const{
      return J;
    }
    /**
     *Get the number of rows.
     *@return unsigned int the number of rows.
     */
    unsigned int GetnRows() const{
      return nRows;
    }
    /**
     *Get the number of columns.
     *@return unsigned int the number of columns.
     */
    int GetnColumns() const{
      return nColumns;
    }

    /**
     *Get the location for the corresponding part.
     */
    Location GetLocation() const{
      return loc;
    }

    /**
     * Set the row corresponding to the upper left corner of the corresponding part.
     */
    void SetI(unsigned int i){
      I = i;
    }  
    /**
     * Set the column corresponding to the upper left corner of the corresponding part.
     */  
    void SetJ(int j){
      J = j;
    }

    /**
     * Set the number of rows.
     */
    void SetnRows(unsigned int i){
      nRows = i;
    }    
    /**
     * Set the number of columns.
     */
    void SetnColumns(int j){
      nColumns = j;
    }

    /**
     *Set the location for the corresponding part.
     */
    void SetLocation(Location l){
      loc = l;
    }   

    /**
     *Define type for the BlockInfo
     */
    void define_type(stapl::typer &t)
    {
      t.local(bi);
      t.local(bj);
      t.local(I);
      t.local(J);
      t.local(nRows);
      t.local(nColumns);
      t.local(loc);
    }
  };
  //@}

   /**
   * @ingroup pmatrix
   *@{
   * Matrix distribution. Specialized distribution where the distribution 
   * information is replicated across all computation threads. The distribution 
   *information is minimal and it reduces to a BlockInfo class per part.
   */
  class MatrixBlockCyclicDistribution : public BaseDistribution<pair<int,GID> > {            
  public:
    
    //===========================
    //types
    //===========================
    
    ///MatrixBlockCyclic Distribution
    typedef MatrixBlockCyclicDistribution                       MatrixBlockCyclic_Distribution_type;
 
    ///The distribution info storage type
    typedef vector<BlockInfo>           DistributionInfo_type;
    typedef vector<BlockInfo>::iterator DistributionInfo_iterator;
    typedef pair<int, MatrixBlockCyclic_Distribution_type>      distribution_pair;
    typedef BaseDistribution<pair<int,GID> >                    BaseDistribution_type;
    typedef pair<unsigned int, unsigned int>                    Layout_type;
    //===========================
    //data
    //===========================
  public:
    unsigned int M,N;
    unsigned int mb,nb;
    Layout_type layout;//processors layout P,Q
    unsigned int bP,bQ;//blocks per processor on P axis 
                       //blocks per processor on Q axis 
    unsigned int BP,BQ;//blocks on P/Q axis for global matrix
  public:
    //===========================
    //constructors & destructors
    //===========================
    
    /**@brief Default constructor.
     */
    MatrixBlockCyclicDistribution(){                                
      this->register_this(this);
      rmi_fence(); 
    }

    /**@brief 
     *Constructor
     *@param _m global number of rows. 
     *@param _n global number of columns.
     *@param DistributionInfo_type the distribution info that has to be replicated 
     *across all the threads. 
     */
    MatrixBlockCyclicDistribution(unsigned int _m, 
				  unsigned int _n, 
				  unsigned int _mb, 
				  unsigned int _nb,
				  unsigned int _bP, 
				  unsigned int _bQ,
				  Layout_type _l) : M(_m),N(_n),mb(_mb),nb(_nb),bP(_bP),bQ(_bQ),layout(_l){

      BP = M / mb + ((M%mb != 0)?1:0);
      BQ = N / nb + ((N%nb != 0)?1:0);
      this->register_this(this);
      rmi_fence(); 
    }
    
    /**@brief Copy constructor.
     */
    MatrixBlockCyclicDistribution(const MatrixBlockCyclicDistribution& _other) : BaseDistribution_type(_other) {
      M  = _other.M;
      N  = _other.N;
      mb = _other.mb;
      nb = _other.nb;
      bP = _other.bP;
      bQ = _other.bQ;
      BP = _other.BP;
      BQ = _other.BQ;
      layout = _other.layout;
      this->register_this(this);
      rmi_fence(); 
    }
    
    /**@brief Destructor.
     */
    ~MatrixBlockCyclicDistribution() {
      stapl::unregister_rmi_object( this->getHandle() );
    }
    
    //===========================
    //public methods
    //===========================

    /**@brief Assignment operator.
     */
    MatrixBlockCyclic_Distribution_type& operator= (const MatrixBlockCyclic_Distribution_type& _other) {
      BaseDistribution_type::operator=(_other);
      M  = _other.M;
      N  = _other.N;
      mb = _other.mb;
      nb = _other.nb;
      bP = _other.bP;
      bQ = _other.bQ;
      BP = _other.BP;
      BQ = _other.BQ;
      layout = _other.layout;
      return *this;
    }
    
    
    /**@brief Initializing array distribution; make a copy of the distribution info 
     *and set the local information
     *@param _m global number of rows. 
     *@param _n global number of columns.
     *@param DistributionInfo_type the distribution info that has to be replicated 
     *across all the threads. 
     */
    void InitializeMatrixBlockCyclicDistribution(unsigned int _m, 
						 unsigned int _n, 
						 unsigned int _mb, 
						 unsigned int _nb,
						 unsigned int _bP, 
						 unsigned int _bQ,
						 Layout_type _l){
      M  = _m;
      N  = _n;
      mb = _mb;
      nb = _nb;
      bP = _bP;
      bQ = _bQ;
      BP = M / mb + ((M%mb != 0)?1:0);
      BQ = N / nb + ((N%nb != 0)?1:0);
      layout = _l;
    }
    
    /**@brief 
     *Clear the distribution info data structure.
     */
    void ClearDistribution(){
      partbrdyinfo.clear();
    }

    unsigned int Get_mb(){
      return mb;
    }

    unsigned int Get_nb(){
      return nb;
    }


    //====================================
    //Methods for element location
    //====================================

  protected:
    /*
     * Finding the part to which a certain gid/index belongs; 
     * The parts are stored in a stored order so we can perform a binary search to find 
     * the part that contains the requested gid/index
     */
    PARTID SearchPart(GID _gid) const {
      /*
      int __len = distribution_info.size();
      int __half,__middle,__first=0;
      while (__len > 0) {
	__half = __len >> 1;
	__middle = __first;
	__middle += __half;
	if (_gid < distribution_info[__middle].GetGID())
	  //search inside the left half
	  __len = __half;
	else if(_gid >= distribution_info[__middle].GetGID() && 
	     _gid < distribution_info[__middle].GetGID() + distribution_info[__middle].GetSize())
	  //found
	    return __middle;
	else {
	  //search to the right
	  __first = __middle;
	  ++__first;
	  __len = __len - __half - 1;
	}
      }
      */
      return -1;
    }

  public:
    /**
     *for the element specified trough _i and _j return the Block_i, Block_j in the global 
     *matrix
     */
    pair<unsigned int, unsigned int> GetGlobalBlockId(unsigned int _i, unsigned int _j){
      return pair<unsigned int, unsigned int>(_i/mb,_j/nb);
    }

    unsigned int GlobalBlockIdToLocal(unsigned int _g){
      unsigned int B_i,B_j;
      B_i = _g / BQ;
      B_j = _g % BQ;
      unsigned int b_i = B_i / layout.first;
      unsigned int b_j = B_j / layout.second;
      return (b_i*bQ + b_j);
    }

    /*An element is local if it is in the specified part
     */
    virtual bool IsLocal(GID _gid, PARTID& partid) const{
      return IsLocal(_gid / N, _gid % N,partid);
    }




    /**@brief Test if an element specified by its (i,j) is local or not.
     *@param index _i of the element
     *@param index _j of the element
     *@param PARTID& this is an output argument. If the (_i,_j) is local it will be initialized with the 
     *the PARTID where the element lives. 
     *@return true if the GID is local and false otherwise.
     */
    bool IsLocal(unsigned int _i,unsigned int _j, PARTID& partid) const{
      unsigned int B_i = (_i / mb);
      unsigned int B_j = (_j / nb);
      unsigned int p_i = B_i % layout.first;
      unsigned int p_j = B_j % layout.second;
      unsigned int b_i = B_i / layout.first;
      unsigned int b_j = B_j / layout.second;

      if (p_i*layout.second + p_j == this->myid){
	//fill information about part id(block id) and return true
	//cout<<"fix the part id information"<<endl;
	partid = b_i*bQ + b_j;
	return true;
      }
      else {
	//cout<<"fix the part id information"<<endl;
	partid = B_i*BQ + B_j;
	return false;
      }
    }

      inline 
      void pid2ij(unsigned int _pid, pair<unsigned int, unsigned int>& _p) const {
	_p.first = _pid / layout.second;
	_p.second = _pid % layout.second;
      }

    /**@brief Test if the current part has data from a certain row.
     *@param index _rowid of the row
     *@param index _colid leftmost column I own from the row(output)
     *@param PARTID& this is an output argument. If the (_i,_j) is local it will be initialized with the 
     *the PARTID where the element lives. 
     *@return true if the GID is local and false otherwise.
     */
    bool IsLocalRow(/*in*/unsigned int _rowid,
		    /*out*/vector<unsigned int>& _colids,
		    /*out*/vector<PARTID>& partids) const{
      //linear search
      cout<<"Check IsLocalRow"<<endl;
      pair<unsigned int, unsigned int> _current;
      unsigned int B_i = (_rowid / mb);
      unsigned int p_i = B_i % layout.first;
      unsigned int b_i = B_i / layout.first;
      pid2ij(this->myid,_current);
      if(p_i == _current.first){
	//the row has local components
	int j=_current.second;
	while(j * nb < N){
	  _colids.push_back(j*nb);
	  unsigned int b_j = j / layout.second;
	  partids.push_back(b_i*bQ + b_j);
	  j+=layout.second;
	}
	return true;
      }
      else 
	return false;
    }
    
    /**@brief 
     *Lookup methods; First is for compatibility with BasePContainer
     *The other is specific to matrix
    */
    virtual Location Lookup(GID _gid) const {
      return Lookup(_gid / N, _gid % N);
    }
    

    /**@brief Find where an element specified by its (i,j) lives.
     *@param index _i of the element
     *@param index _j of the element
     *@return Location that owns the GID
     */
    Location Lookup(unsigned int _i, unsigned int _j) const {
      unsigned int B_i = (_i / mb);
      unsigned int B_j = (_j / nb);
      unsigned int p_i = B_i % layout.first;
      unsigned int p_j = B_j % layout.second;
      unsigned int b_i = B_i / layout.first;
      unsigned int b_j = B_j / layout.second;
      //cout<<"B_i="<<B_i<<" B_j="<<B_j<<" p_i="<<p_i<<" p_j"<<p_j<<" b_i="<<b_i<<" b_j"<<b_j<<endl;
      return Location(p_i*layout.second + p_j , B_i*(N/nb + ((N%nb != 0)?1:0)) + B_j);
    }
    

    /**@brief Find which thread owns the element (_rowid, _colid)
     *@param index _rowid of the element
     *@param index _colid of the element
     *@return Location that owns the GID
     */
    Location LookupRow(unsigned int _rowid, unsigned int _colid) const {
      //to be replaced by binary search SearchPart()
      return Lookup(_rowid, _colid);
    }
  
    virtual void Add2LocationMap(_StaplPair<GID,Location>& _x) {}
    virtual void Add2PartIDMap(GID _gid,PARTID _pid) {}

    //===========================
    //bookkeeping for element_location_map                  
    //could be called both from local and from remote
    //===========================

    /*Method to add a part to the distribution vector
     */
    void AddPartToDistributionVector(GID start, int size, Location loc){
      cout<<"AddPartToDistributionVector not implemented yet"<<endl;
    }
    
    /*Method to add a part to the distribution vector
     */
    void AddPartToDistributionVector(BlockInfo d){
      cout<<"AddPartToDistributionVector not available for BCD"<<endl;
    }

    /* Method that allows a new part to be added to the pContainer
     * Called from the pMatrixBlockCyclic
     * Broadcasts the change to the rest of the processors
     */
    void AddPart(GID start, int size, Location loc){
      cout<<"AddPart not available for BCD"<<endl;
      /*
      MatrixBlockCyclicDistributionInfo d;
      d.SetSize(size);
      d.SetGID(start);
      d.SetLocation(loc);
      distribution_info.push_back(d);
      for (int i=0; i<nprocs; i++){
	if (i != this->myid)
	  async_rmi(i, getHandle(), 
		    &MatrixBlockCyclicDistribution::AddPartUpdate, d);
      }
      */
    }

    /*Method to update the distribution vector
     *on the rest of the processors
    */
    void AddPartUpdate(BlockInfo d){
      cout<<"AddPart not available for BCD"<<endl;
    }

    /**@brief 
     *Method to display the distribution vector
     */
    virtual void DisplayDistributionInfo() const {       
      cout<<"MatrixBlockCyclic Distribution Vector"<<endl;
      /*
      for(int i=0; i< distribution_info.size(); i++) {
	cout<<"Processor: "<<distribution_info[i].GetLocation().locpid()<<endl;
	cout<<"Part    : "<<distribution_info[i].GetLocation().partid()<<endl;
	cout<<"I       : "<<distribution_info[i].GetI()<<endl;
	cout<<"J       : "<<distribution_info[i].GetJ()<<endl;
	cout<<"M   : "<<distribution_info[i].GetnRows()<<endl;
	cout<<"N: "<<distribution_info[i].GetnColumns()<<endl;
      } 
      */
    }
    

        
  };  //end of MatrixBlockCyclicDistribution
  
} //end namespace stapl
//@}
#endif
