/*!
	\file splBaseElement.h
	\author  	
	\date Jan. 9, 03
	\ingroup stapl
	\brief Base Element class to attach Global ID to each element

	A wrapper over user's data type. 
	(for example: User instantiate pList<int>, internally we store
	list<splBaseElement<int> > in each pList Part.)

 	Internal to stapl developer. 
*/

#ifndef BASEELEMENT_H
#define BASEELEMENT_H
#include "PContainerExceptions.h"

namespace stapl {

  template<class R,class T>
    class DefaultExtractor{
    public:
    DefaultExtractor(){}
    
    R& GetUserData(const T& _current) const{
      return (*_current).GetUserData();
    }

    GID GetGID(const T& _current) const{
      return (*_current).GetGid();
    }
    
  };

template<class T>
class splAbstractBaseElement {

 public:
  splAbstractBaseElement() {}
  ~splAbstractBaseElement() {}
  
  virtual GID GetGid() const = 0;
  virtual void SetGid(GID) = 0;

  virtual const T& GetUserData() const = 0;

  //potential trouble, will hide the const one above.
  virtual T& GetUserData() = 0;

  virtual void SetUserData(const T&) = 0;

};

template<class T>
class splBaseElement : public splAbstractBaseElement<T> {
 protected:
  GID gid;
  T data;

 public:
  typedef T value_type;
  typedef splBaseElement<value_type> element_type;

  void define_type(typer &t)  {
    t.local(gid);
    t.local(data);
  }

 public:
  splBaseElement() : gid(0), data() {}
  splBaseElement(const T& _d) : data(_d) {}

  splBaseElement(GID _g, const T& _d) : gid(_g), data(_d) {}

  splBaseElement(const element_type& _t) :gid(_t.gid), data(_t.data) {}
  ~splBaseElement() {}
  
  element_type& operator=(const element_type& _t) {
    gid = _t.gid; 
    data = _t.data;
    return *this;
  }

  bool operator==(const element_type& _t) const {
    return (gid == _t.gid && data == _t.data);
  }

  virtual void Set(GID _gid, const T& _d) {
    gid = _gid;
    data = _d;
  }

  virtual GID GetGid() const { return gid; }
  virtual void SetGid(GID _gid) { gid = _gid; }

  virtual const T& GetUserData() const { return data; }

  //potential trouble, will hide the const one above.
  virtual T& GetUserData() { return data; }

  virtual void SetUserData(const T& _d) { data = _d; }
};

template <class T>
ostream& operator<< (ostream &s, const splBaseElement<T>& be) {
  s << be.GetGid() << " "
    << endl;
  return s;
}

} //end namespace stapl

#endif
