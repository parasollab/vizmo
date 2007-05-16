#ifndef RMITOOLS_H
#define RMITOOLS_H

#include <runtime.h>
#include <Defines.h>
//-----------------------------------------------------------------------

//stl pair can pack and send with rmi now
//defined _StaplPair as pair in DistributionDefines.h

/* template<class First,class Second> */
/* class _StaplPair */
/* { */

/* public: */
/*   First first; */
/*   Second second; */
/*   _StaplPair(){} */
/*   _StaplPair(First f, Second s):first(f), second(s){} */
/*   void define_type(stapl::typer &t) */
/*     { */
/*       t.local(first); */
/*       t.local(second); */
/*     } */
/* }; */


//--------------------------------------------------------------------
template<class First,class Second,class Third>
class _StaplTriple
{
  
public:
  First first;
  Second second;
  Third third;
  _StaplTriple(){}
  _StaplTriple(First f, Second s, Third t):first(f), second(s), third(t){}
  void define_type(stapl::typer &t)
    {
      t.local(first);
      t.local(second);
      t.local(third);
    }
};

//------------------------------------------------------------------


template<class First,class Second,class Third,class Fourth>
class _StaplQuad
{
  
public:
  First first;
  Second second;
  Third third;
  Fourth fourth;
  _StaplQuad(){}
  _StaplQuad(First f, Second s, Third t, Fourth ft):
    first(f), second(s), third(t),fourth(ft){}
  void define_type(stapl::typer &t)
    {
      t.local(first);
      t.local(second);
      t.local(third);
      t.local(fourth);
    }
};
//-------------------------------------------------------------------

template<class First,class Second,class Third,class Fourth,class Fifth>
class _StaplPenta
{
  
public:
  First first;
  Second second;
  Third third;
  Fourth fourth;
  Fifth fifth;
  _StaplPenta(){}
  _StaplPenta(First f, Second s, Third t, Fourth ft,Fifth fft):
    first(f), second(s), third(t),fourth(ft),fifth(fft){}
  void define_type(stapl::typer &t)
    {
      t.local(first);
      t.local(second);
      t.local(third);
      t.local(fourth);
      t.local(fifth);
    }
};

template<class First,class Second,class Third,class Fourth,class Fifth, class Sixth>
class _StaplSix
{

 public:
  First first;
  Second second;
  Third third;
  Fourth fourth;
  Fifth fifth;
  Sixth sixth;

  _StaplSix(){}
  _StaplSix(First f, Second s, Third t, Fourth ft,Fifth fft, Sixth sth):
    first(f), second(s), third(t),fourth(ft),fifth(fft),sixth(sth){}

  void define_type(stapl::typer &t)
    {
      t.local(first);
      t.local(second);
      t.local(third);
      t.local(fourth);
      t.local(fifth);
      t.local(sixth);
    }
};


//----------------------------------------------------------------
/*
This object can be inherited by all objects that want to support
rmi operations on it's methods;
If an object inherits multiple objects this one has to be first
*/
class BasePObject {
protected:
  stapl::rmiHandle Handle;
  int thread_id;
  int num_threads;

public:
  BasePObject() {
    Handle = -1;
    thread_id   = stapl::get_thread_id();
    num_threads = stapl::get_num_threads();
    //Handle = stapl::register_rmi_object( this );
#ifdef STAPL_DEBUG
    cout<<"registered "<<Handle<<endl;
#endif
  }
  
  stapl::rmiHandle register_this(void* _handle){
    Handle = stapl::register_rmi_object( _handle );
#ifdef STAPL_DEBUG
    cout<<"registered "<<Handle<<endl;
#endif
    return Handle;
  }

  inline stapl::rmiHandle getHandle() const {
    return Handle;
  }

  BasePObject& operator=(const BasePObject& other) {
    //	  Handle = stapl::register_rmi_object(this);
    return *this;
  }

  ~BasePObject() {
//			stapl::unregister_rmi_object( Handle );
  }

};    

#endif
