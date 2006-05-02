/*!
	\file GraphPart.h
	\author Gabriel Tanase	
	\date Jan. 9, 03
	\ingroup stapl
	\brief Derived from BaseDistribution. I
	nherits all of its functionality

 	Internal to stapl developer. 
*/
#ifndef GRAPHDISTRIBUTION_H
#define GRAPHDISTRIBUTION_H

#include "BaseDistribution.h"
/**
 * @ingroup pgraph
 * @{
 */
namespace stapl{
/**
 *Graph distribution inherits the functionality of BaseDistribution. Optimizations
 *specific to the pGraph pcontainer can be implemented here.
 *
 *Status: The GraphDistribution inherits all of its functionality from BaseDistribution.
 *There only methods 
 *implemented here are the constructor that perform the registration and 
 *methods to read/write the distribution to a stream.
*/

template<class ELEMENT_SET>
class GraphDistribution : public BaseDistribution<ELEMENT_SET>
{
  using BaseDistribution<ELEMENT_SET>::element_location_map; 
  using BaseDistribution<ELEMENT_SET>::element_location_cache; 
  using BaseDistribution<ELEMENT_SET>::myid;
  using BaseDistribution<ELEMENT_SET>::nprocs;
  
  //the next line doesn't compile on aCC; <not yet implemented>
  //using typename BaseDistribution<ELEMENT_SET>::CILIT;
  
  typedef typename BaseDistribution<ELEMENT_SET>::CILIT CILIT;

 public:
  
  //===========================
  //constructors & destructors
  //===========================
  /**@brief
   *Default constructor.
   */
  GraphDistribution() : BaseDistribution<ELEMENT_SET>() {
    this->register_this(this);
  }
  
  GraphDistribution(ELEMENT_SET* _pg) : 
    BaseDistribution<ELEMENT_SET> (_pg) {
    this->register_this(this);
  }
  /**@brief
   *Copy constructor.
   */
  GraphDistribution(const GraphDistribution& other):BaseDistribution<ELEMENT_SET>(other){
    this->register_this(this);
  }

  bool _IsLocal(VID _gid) {
    int tmp=-1;
    if(_IsLocal(_gid,tmp) ) return true;
    else return false;
  }

  /*@brief
   *Destructor. Unregisters the object.
   */
  ~GraphDistribution() {
   rmiHandle handle = this->getHandle();
	 if (handle > -1)
     unregister_rmi_object(handle);
  }

  /**@brief
   *Writes the contents of the location map to the output stream. 
   *Called by the function pFileWrite in pGraph; The parts will be ignored
   *when
   *@param ostream the output stream where the distribution information
   *will be written.
   */
  void LocationMapWrite(ostream& _myostream){

    _myostream << "LOCMAPSTART"<<endl;
    _myostream << element_location_map.size()<<endl;
    CILIT it=element_location_map.begin();
    for(;it!=element_location_map.end(); it++) {
      _myostream<<it->first<<" "<<it->second.locpid()<<endl;
    } 
    _myostream << "LOCMAPSTOP"<<endl;
  }

  /**@brief
   *Read the content of the location map from the input stream. 
   *Called by the function pFileRead in pGraph; 
   *@param istream the input stream from where the distribution information
   *will be read.
   */
  void LocationMapRead(istream& _myistream){
    char tagstring[100];

    _myistream >> tagstring;
    if ( !strstr(tagstring,"LOCMAPSTART") ) {
      cout << endl << "In LocationMapRead: didn't read MAPSTART tag right";
      return;
    }

    int num_elements;
    _myistream >> num_elements;
    
    for(int i=0; i<num_elements; ++i){
	VID v;
	PID p;
	_myistream >> v;
	_myistream >> p;

	//???????????????????????????????????????????????????//
	//make a Location and add it to the maps
	Location _loc(p,0);
	_StaplPair<GID,Location> lm(v,_loc);
	this->Add2LocationMap(lm);
	//element_location_map[v] = _loc;
      }
    _myistream >> tagstring;
    if ( !strstr(tagstring,"LOCMAPSTOP") ) {
      cout << endl << "In LocationMapRead: didn't read LOCMAPEND tag right";
      return;
    }
  }

};//end class

}//end namespace stapl
//@}
#endif



