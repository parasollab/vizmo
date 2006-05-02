
////////////////////////////////////////////////////////////////////////////
// File :   private_mechanism.h
// Despcription : Private store for the pContainers
// Author  : Alin Jula
// Date :       08/22/02
// Last update :08/12/02
// Texas A&M University,College Station, TX
///////////////////////////////////////////////////////////////////////////
#ifndef STAPL_PRIVATE_MECHANISM
#define STAPL_PRIVATE_MECHANISM

#include <vector>


using namespace _STLP_STD;


namespace stapl {
#define STAPL_PUSH_BACK  1
#define STAPL_INSERT     2
#define STAPL_ERASE      3




template <class Type_Value, class Iterator>
class Private_Item{
 public:
  //data
  Iterator __it;
  Type_Value __value;
  size_t   __op;
 public:
  Private_Item() : __op(-1),__it(),__value() {}
  Private_Item(const Type_Value & __v,Iterator __i,size_t __o): __value(__v),__it(__i),__op(__o){}
  Private_Item(const Type_Value & __v,size_t __o): __value(__v),__it(),__op(__o){}
  Private_Item(const Private_Item & __priv)
    {
      __it=__priv.__it;
      __value=__priv.__value;
      __op=__priv.__op;
    }
};



template <class Type_Value, class Iterator>
class Private_Storage{
 public:
  vector<vector<Private_Item<Type_Value,Iterator> > >  __storage;
 public:
  Private_Storage() { __storage.resize(get_num_threads());}
  Private_Storage(Private_Storage & __s)
    { __storage=__s;}
  void clear()
    {
	for(int i=0;i<__storage.size();i++)
           __storage[i].clear();	
    }
};


}// end namespace stapl;

#endif
