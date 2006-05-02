#ifndef _RMI_REGISTRY_H
#define _RMI_REGISTRY_H

// The RMI registry is an array of void* (which are cast to the appropriate
// class pointers by RMI requests).  Handles are indices into the array,
// making translation as fast as an array dereference.  It assumes programs
// are SPMD, meaning all objects are registered in the same order on all
// processors, such that handles will be assigned in the same order.  As
// objects are registered, the registry will grow by doubling in size, similar
// to a std::vector.  Empty entries of the registry maintain a free list, by
// keeping the index of the next available index.  Removed objects are
// inserted at the front of the free list.


// The initial size for the RMI registry.  Although the registry will grow as
// needed, it is more efficient to allocate it beforehand if a known, constant
// number of objects will need to be registered.  Note, one entry in the entry
// is reserved for fast registration via execute_parallel_task().  May be
// defined at compile time (e.g., -D_DEFAULT_REGISTRY_SIZE=???).
#include <string>
#include <vector>
#include <set>
#include <map>
#include <utility>
#include <typeinfo>
#ifndef _DEFAULT_REGISTRY_SIZE
#define _DEFAULT_REGISTRY_SIZE 20
#endif


#ifndef _ARMI_REGISTRY_CHECK_TYPE  
namespace stapl { 
  
  typedef pointer_to_integral<void*>::integral rmi_integral;
  typedef rmi_integral rmiHandle;
  
  class rmiRegistry {
    
    void** objRegistry;
    rmi_integral registryIndex;
    rmi_integral registrySize;
    std::set<void *> registeredElements;
    std::vector<void *> registryTrace;
    
  public:
    inline rmiRegistry() : registryIndex(1), registrySize(_DEFAULT_REGISTRY_SIZE) {
      objRegistry = static_cast<void**>( safe_malloc(sizeof(void*)*registrySize) );
      objRegistry[0] = 0;
      for( rmi_integral i=1; i<registrySize-1; i++ )
	objRegistry[i] = reinterpret_cast<void*>( i+1 );
      objRegistry[registrySize-1] = 0;
    }
    
    inline ~rmiRegistry() {
      free( objRegistry );
    }
    
    inline void* operator[](const rmiHandle handle) const {
      return objRegistry[handle];
    }
    
    inline void fastRegisterTask(void* const parallelTask) {
      objRegistry[0] = parallelTask;
    }
    
    inline const std::vector<void*>& getTrace(void) {
      return registryTrace;
    }

    inline rmiHandle registerObject(void* const objectPointer) {
      //Log this registration for checker
      registryTrace.push_back(objectPointer);
  
      //Check for previous registration
      typedef std::set<void*>::iterator tempType; //aCC parser gets confused inlined
      std::pair<tempType, bool>  insertReturn;
      insertReturn = registeredElements.insert(objectPointer);
      stapl_assert(insertReturn.second, "registerObject: object already registered");
      
      //Double Size of Registry if we have no more room
      if( registryIndex == 0 ) {
	const rmi_integral tmp = registryIndex = registrySize;
	registrySize *= 2;
	objRegistry = static_cast<void**>
	  ( realloc(objRegistry, sizeof(void*)*registrySize) );
	stapl_assert( objRegistry != 0, "object registry reallocation failure" );
	for( rmi_integral i=tmp; i<registrySize-1; i++ )
	  objRegistry[i] = reinterpret_cast<void*>( i+1 );
	objRegistry[registrySize-1] = 0;
      }

      //Create Handle and return it
      const rmiHandle tmp = registryIndex;
      registryIndex = reinterpret_cast<rmi_integral>( objRegistry[registryIndex] );
      objRegistry[tmp] = objectPointer;
      return tmp;
    }
    
    inline void updateObject(rmiHandle handle, void* const objectPointer) {
      stapl_assert( handle > 0 && handle < registrySize, "invalid rmiHandle" );
      registeredElements.erase(objRegistry[handle]);
      typedef std::set<void*>::iterator tempType; //aCC parser gets confused inlined
      std::pair<tempType, bool>  insertReturn;
      insertReturn = registeredElements.insert(objectPointer);
      stapl_assert(insertReturn.second, "updateObject: object already registered");
      objRegistry[handle] = objectPointer;
    }
    
    inline void unregisterObject(rmiHandle handle) {
      stapl_assert( handle > 0 && handle < registrySize, "invalid rmiHandle" );
      registeredElements.erase(objRegistry[handle]);
      objRegistry[handle] = reinterpret_cast<void*>( registryIndex );
      registryIndex = handle;
    }
  };
} //namespace stapl

#else //#ifndef _ARMI_REGISTRY_CHECK_TYPE  

namespace stapl {

  typedef int typeIntegral;
  typedef pointer_to_integral<void*>::integral objectIntegral;
  typedef std::pair<typeIntegral, objectIntegral> rmiHandle;
  
  class registry {
    typedef pointer_to_integral<void*>::integral rmi_integral;
    typedef rmi_integral rmiHandle;
    
    void** objRegistry;
    bool managed;
    rmi_integral registryIndex;
    rmi_integral registrySize;
    
  public:
    inline registry() : registryIndex(1), registrySize(_DEFAULT_REGISTRY_SIZE), managed(true) {
      objRegistry = static_cast<void**>( safe_malloc(sizeof(void*)*registrySize) );
      objRegistry[0] = 0;
      for( rmi_integral i=1; i<registrySize-1; i++ )
	objRegistry[i] = reinterpret_cast<void*>( i+1 );
      objRegistry[registrySize-1] = 0;
    }
    
    
    inline registry(registry const &r) {
      registryIndex = r.registryIndex;
      registrySize = r.registrySize;
      objRegistry = static_cast<void**>
	( safe_malloc(sizeof(void*)*registrySize) );
      for (rmi_integral i=1; i<registrySize-1; i++) 
	objRegistry[i] = r.objRegistry[i];
      managed = true;
    }
    
    inline void setUnmanaged() {
      managed = false;
      for( rmi_integral i=1; i<registrySize; i++ )
	objRegistry[i] = reinterpret_cast<void*>(0);    
    }
    
    inline ~registry() {
      free( objRegistry );
    }
    
    inline void* operator[](const rmiHandle handle) const {
      return objRegistry[handle];
    }
    
    inline registry& operator=(const registry &r) {
      if (&r != this) {
	registryIndex = r.registryIndex;
	registrySize = r.registrySize;
	free(objRegistry);
	objRegistry = static_cast<void**>
	  ( safe_malloc(sizeof(void*)*registrySize) );
	for (rmi_integral i=1; i<registrySize-1; i++) 
	  objRegistry[i] = r.objRegistry[i];
      }
      return *this;
    }
    
    
    inline void fastRegisterTask(void* const parallelTask) {
      objRegistry[0] = parallelTask;
    }
    
    inline rmiHandle registerObject(void* const objectPointer) {   
      if( registryIndex == 0 ) {
	const rmi_integral tmp = registryIndex = registrySize;
	registrySize *= 2;
	objRegistry = static_cast<void**>
	  ( realloc(objRegistry, sizeof(void*)*registrySize) );
	stapl_assert( objRegistry != 0, "object registry reallocation failure" );
	for( rmi_integral i=tmp; i<registrySize-1; i++ )
	  objRegistry[i] = reinterpret_cast<void*>( i+1 );
	objRegistry[registrySize-1] = 0;
      }
      
      const rmiHandle tmp = registryIndex;
      registryIndex = reinterpret_cast<rmi_integral>( objRegistry[registryIndex] );
      objRegistry[tmp] = objectPointer;
      return tmp;
    }
    
    
    inline rmiHandle registerObject(void *const objectPointer, int idx) {
      stapl_assert(idx > 0, "manual registration index must be greater than 0");
      stapl_assert(!managed,"explicit index registering can only be called on a unmanaged registry");
      if (idx >= registrySize) {
	int oldSize = registrySize;
	registrySize *= 2;
	objRegistry = static_cast<void**>
	  ( realloc(objRegistry, sizeof(void*)*registrySize) );
	for (int i = oldSize; i < registrySize; i++)
	  objRegistry[i] = reinterpret_cast<void*>(0);
      }
      
      objRegistry[idx] = objectPointer;
      const rmiHandle tmp = idx;
      return tmp;
    }
    
    
    inline void updateObject(rmiHandle const &handle, void* const objectPointer) {
      stapl_assert( handle > 0 && handle < registrySize, "invalid rmiHandle" );
      objRegistry[handle] = objectPointer;
    }
    
    inline void unregisterObject(rmiHandle const &handle) {
      stapl_assert( handle > 0 && handle < registrySize, "invalid rmiHandle" );
      if (managed) {
	objRegistry[handle] = reinterpret_cast<void*>( registryIndex );
	registryIndex = handle;
      } else {
	objRegistry[handle] = 0;
      }
    }
  };
  
  
  class rmiRegistry {
    struct lttype { 
      bool operator()(const std::type_info* t1, const std::type_info* t2) const {
	return t1->before(*t2);
      }
    };
    
    std::set<void*> registeredObjects;                 //Check for double registration
    std::map<const type_info*, int, lttype> registeredTypes; //Keep track of prev. seen types
    std::vector<registry> objRegistry;
    
    
  public:    
    inline rmiRegistry() {
      objRegistry.reserve(_DEFAULT_REGISTRY_SIZE);
      objRegistry.resize(1);
      objRegistry[0].setUnmanaged();  //This registry used for fast & explicit reg. calls
    }
    
    inline ~rmiRegistry() { }
    
    inline void* operator[](const rmiHandle handle) const {
      return objRegistry[handle.first][handle.second];
    }
    
    inline void fastRegisterTask(void* const parallelTask) {
      objRegistry[0].fastRegisterTask(parallelTask);
    }
    
    inline rmiHandle registerObject(void* const objectPointer,
				    std::type_info const &objectType) {     
      //Check for previous registration
      typedef std::set<void*>::iterator tempType; //aCC parser gets confused inlined
      std::pair<tempType, bool>  insertReturn;
      insertReturn = registeredObjects.insert(objectPointer); 
      stapl_assert(insertReturn.second, "registerObject: object already registered");
      
      //Have we encountered this type of object before
      int &typeIndex = registeredTypes[&objectType];
      if (typeIndex == 0) {
	typeIndex = objRegistry.size();
	objRegistry.resize(typeIndex + 1);
      } 
      
      //Create Handle in type subregistry and return
      const rmiHandle handle(typeIndex,objRegistry[typeIndex].registerObject(objectPointer));
      return handle;
    }
    
    inline rmiHandle registerObject(void *const objectPointer, int idx) {
      //Verify object not already registered
      typedef std::set<void*>::iterator tempType; //aCC parser gets confused inlined
      std::pair<tempType, bool>  insertReturn;
      insertReturn = registeredObjects.insert(objectPointer); 
      stapl_assert(insertReturn.second, "registerObject: object already registered");
      
      //Insert in explicit registration registry (0) and return handle
      const rmiHandle handle(0,objRegistry[0].registerObject(objectPointer,idx));
      return handle;
    }
    
    inline void updateObject (rmiHandle const &handle, void* const objectPointer,  
			      std::type_info const &objectType) {     
      stapl_assert(0, "updateObject has deprecated because of lack of use");
      stapl_assert(handle.first >= 0 && handle.first <= objRegistry.size() + 1, "invalid rmiHandle");
      
      //Remove old object from list of registered objects
      registeredObjects.erase(objRegistry[handle.first][handle.second]);

      //Verify new object not already registered
      typedef std::set<void*>::iterator tempType; //aCC parser gets confused inlined
      std::pair<tempType, bool>  insertReturn;
      insertReturn = registeredObjects.insert(objectPointer);
      stapl_assert(!insertReturn.second, "updateObject: object already registered");
      
      //Upated from the type subregistry
      objRegistry[handle.first].updateObject(handle.second, objectPointer);  
    }
    
    inline void unregisterObject(rmiHandle const &handle) {
      stapl_assert(handle.first >= 0 && handle.first <= objRegistry.size() + 1, "invalid rmiHandle");
      //Remove from list of registered objects
      registeredObjects.erase(objRegistry[handle.first][handle.second]);
      
      //Unregistry from the type subregistry
      objRegistry[handle.first].unregisterObject(handle.second);
    }
  };
  
} //end namespace stapl
  
#endif // #ifndef _ARMI_REGISTRY_CHECK_TYPE  
#endif // _RMI_REGISTRY_H



