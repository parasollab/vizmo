/*!
	\file MatrixDistribution.h
	\author Gabriel Tanase	
	\date Jan. 25, 04
	\ingroup stapl
	\brief Matrix Distribution class : Maps from global indices (i,j) to processor
	containing the indices
 	Internal to stapl developer. 
*/
#ifndef MATRIXDISTRIBUTION_H
#define MATRIXDISTRIBUTION_H

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
  class MatrixDistributionInfo {
  
  public:
    //The indices for the first element that we have in a part
    unsigned int I;
    unsigned int J;
    unsigned int nRows;
    unsigned int nColumns;
    //location where the part lives(local part id, processor id)
    Location loc;

    /**
     *Default constructor.
     */
    MatrixDistributionInfo(){}

    /**
     *Constructor
     *@param _i Upper left corner row in the global matrix.
     *@param _j Upper left corner column in the global matrix.
     *@param _m rows number.
     *@param _n columns number.
     *@param Location the thread which will own the part.
     */
    MatrixDistributionInfo(unsigned int _i, 
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
     *Define type for the MatrixDistributionInfo
     */
    void define_type(stapl::typer &t)
    {
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
   *information is minimal and it reduces to a MatrixDistributionInfo class per part.
   */
  class MatrixDistribution : public BaseDistribution<pair<int,GID> > {            
  public:
    
    //===========================
    //types
    //===========================
    

    ///Matrix Distribution
    typedef MatrixDistribution                       Matrix_Distribution_type;
 
    ///The distribution info storage type
    typedef vector<MatrixDistributionInfo>           DistributionInfo_type;
    typedef vector<MatrixDistributionInfo>::iterator DistributionInfo_iterator;
    typedef pair<int, Matrix_Distribution_type>      distribution_pair;
    typedef BaseDistribution<pair<int,GID> >         BaseDistribution_type;
    //===========================
    //data
    //===========================
  protected:
    //the distribution information; it will be replicated across
    //all processors
    DistributionInfo_type  distribution_info;
  public:
    int nRows,nColumns;
    
  public:
    //===========================
    //constructors & destructors
    //===========================
    
    /**@brief Default constructor.
     */
    MatrixDistribution(){                                     
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
    MatrixDistribution(int _m, int _n, const DistributionInfo_type& _di) : distribution_info(_di),nRows(_m),nColumns(_n){
      this->register_this(this);
      rmi_fence(); 
    }
    
    /**@brief Copy constructor.
     */
    MatrixDistribution(const MatrixDistribution& _other) : BaseDistribution_type(_other) {
      nRows        = _other.nRows;
      nColumns     = _other.nColumns;
      distribution_info = _other.distribution_info;  
      this->register_this(this);
      rmi_fence(); 
    }
    
    /**@brief Destructor.
     */
    ~MatrixDistribution() {
      stapl::unregister_rmi_object( this->getHandle() );
    }
    
    //===========================
    //public methods
    //===========================

    /**@brief Assignment operator.
     */
    Matrix_Distribution_type& operator= (const Matrix_Distribution_type& _other) {
      BaseDistribution_type::operator=(_other);
      distribution_info = _other.distribution_info;  
      nRows        = _other.nRows;
      nColumns     = _other.nColumns;
      return *this;
    }
    
    
    /**@brief Initializing array distribution; make a copy of the distribution info 
     *and set the local information
     *@param _m global number of rows. 
     *@param _n global number of columns.
     *@param DistributionInfo_type the distribution info that has to be replicated 
     *across all the threads. 
     */
    void InitializeMatrixDistribution(int _m, int _n, const DistributionInfo_type& _di){
      distribution_info = _di; 
      nRows    = _m;
      nColumns = _n;
    }
    
    /**@brief 
     *Clear the distribution info data structure.
     */
    void ClearDistribution(){
      distribution_info.clear();
      partbrdyinfo.clear();
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
    /*An element is local if it is in the specified part
     */
    virtual bool IsLocal(GID _gid, PARTID& partid) const{
      return IsLocal(_gid / nColumns, _gid % nColumns,partid);
    }


    /**@brief Test if an element specified by its (i,j) is local or not.
     *@param index _i of the element
     *@param index _j of the element
     *@param PARTID& this is an output argument. If the (_i,_j) is local it will be initialized with the 
     *the PARTID where the element lives. 
     *@return true if the GID is local and false otherwise.
     */
    bool IsLocal(unsigned int _i,unsigned int _j, PARTID& partid) const{
      /*
      //binary search SearchPart()
      int temp = SearchPart(_gid);
      if(temp == -1) {
	cout<<"Cannot find element in Location Map with given gid "<<_gid<<endl;
	return false;
      }
      if(distribution_info[temp].GetLocation().locpid() == this->myid){
	partid = distribution_info[temp].GetLocation().partid();
	return true;
      }
      else 
	return false;
      */
      //linear search
      for(int i = 0;i<distribution_info.size();i++){
	if (_i >= distribution_info[i].I && 
	    _i < distribution_info[i].I + distribution_info[i].nRows &&
	    _j >= distribution_info[i].J && 
	    _j < distribution_info[i].J + distribution_info[i].nColumns) {

	  partid = distribution_info[i].GetLocation().partid(); 

	  if(distribution_info[i].GetLocation().locpid() == this->myid){
	    return true;
	  }
	  else{ 
	    return false;
	  }
	}
      }
      cout<<"ERROR MatrixDistribution: While looking for part that contains element"<<_i<<" "<<_j<<endl;
      return false;
    }


    /**@brief Test if the current part has data from a certain row.
     *@param index _rowid of the row
     *@param index _colid leftmost column I own from the row(output)
     *@param PARTID& this is an output argument. If the (_i,_j) is local it will be initialized with the 
     *the PARTID where the element lives. 
     *@return true if the GID is local and false otherwise.
     */
    bool IsLocalRow(/*in*/unsigned int _rowid,/*out*/unsigned int &_colid,/*out*/PARTID& partid ) const{
      //linear search
      for(int i = 0;i<distribution_info.size();++i){
	if (_rowid >= distribution_info[i].I && 
	    _rowid < distribution_info[i].I + distribution_info[i].nRows){
	  partid = distribution_info[i].GetLocation().partid();
	  _colid = distribution_info[i].J;
	  if(distribution_info[i].GetLocation().locpid() == this->myid){
	    return true;
	  }
	}
      }
      return false;
    }
    
    /**@brief 
     *Lookup methods; First is for compatibility with BasePContainer
     *The other is specific to matrix
    */
    virtual Location Lookup(GID _gid) const {
      return Lookup(_gid / nColumns, _gid % nColumns);
    }
    

    /**@brief Find where an element specified by its (i,j) lives.
     *@param index _i of the element
     *@param index _j of the element
     *@return Location that owns the GID
     */
    Location Lookup(unsigned int _i, unsigned int _j) const {
      //to be replaced by binary search SearchPart()
      for(int i = 0;i<distribution_info.size();i++){
	if (_i >= distribution_info[i].I && 
	    _i < distribution_info[i].I + distribution_info[i].nRows &&
	    _j >= distribution_info[i].J && 
	    _j < distribution_info[i].J + distribution_info[i].nColumns)
	  return Location(distribution_info[i].GetLocation().locpid(), distribution_info[i].GetLocation().partid());
      }
      cout<<"ERROR MatrixDistribution: Cannot find element in Location Map with given gid "<<_i<<" "<<_j<<endl;
      return Location::InvalidLocation();
    }
    

    /**@brief Find which thread owns the element (_rowid, _colid)
     *@param index _rowid of the element
     *@param index _colid of the element
     *@return Location that owns the GID
     */
    Location LookupRow(unsigned int _rowid, unsigned int _colid) const {
      //to be replaced by binary search SearchPart()
      for(int i = 0;i<distribution_info.size();i++){
	if (_rowid >= distribution_info[i].I && 
	    _rowid < distribution_info[i].I + distribution_info[i].nRows &&
	    _colid >= distribution_info[i].J && 
	    _colid < distribution_info[i].J + distribution_info[i].nColumns)
	  return Location(distribution_info[i].GetLocation().locpid(), distribution_info[i].GetLocation().partid());
      }
      cout<<"ERROR MatrixDistribution: Cannot find info about specified row,column"<<_rowid<<" "<<_colid<<endl;
      return Location::InvalidLocation();
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
    void AddPartToDistributionVector(MatrixDistributionInfo d){
      cout<<"AddPartToDistributionVector not implemented yet"<<endl;
    }

    /* Method that allows a new part to be added to the pContainer
     * Called from the pMatrix
     * Broadcasts the change to the rest of the processors
     */
    void AddPart(GID start, int size, Location loc){
      cout<<"AddPart not implemented yet"<<endl;
      /*
      MatrixDistributionInfo d;
      d.SetSize(size);
      d.SetGID(start);
      d.SetLocation(loc);
      distribution_info.push_back(d);
      for (int i=0; i<nprocs; i++){
	if (i != this->myid)
	  async_rmi(i, getHandle(), 
		    &MatrixDistribution::AddPartUpdate, d);
      }
      */
    }

    /*Method to update the distribution vector
     *on the rest of the processors
    */
    void AddPartUpdate(MatrixDistributionInfo d){
      cout<<"AddPart not implemented yet"<<endl;
    }

    /**@brief 
     *Method to display the distribution vector
     */
    virtual void DisplayDistributionInfo() const {       
      cout<<"Matrix Distribution Vector"<<endl;
      for(int i=0; i< distribution_info.size(); i++) {
	cout<<"Processor: "<<distribution_info[i].GetLocation().locpid()<<endl;
	cout<<"Part    : "<<distribution_info[i].GetLocation().partid()<<endl;
	cout<<"I       : "<<distribution_info[i].GetI()<<endl;
	cout<<"J       : "<<distribution_info[i].GetJ()<<endl;
	cout<<"nRows   : "<<distribution_info[i].GetnRows()<<endl;
	cout<<"nColumns: "<<distribution_info[i].GetnColumns()<<endl;
      } 
    }
    

        
  };  //end of MatrixDistribution
  
} //end namespace stapl
//@}
#endif
