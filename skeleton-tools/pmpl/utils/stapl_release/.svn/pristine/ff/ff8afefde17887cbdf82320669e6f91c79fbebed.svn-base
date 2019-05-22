/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/


#include <stapl/runtime.hpp>

#include <iostream>

#include <stapl/containers/array/array.hpp>
#include <stapl/algorithms/algorithm.hpp>

#include <cstdlib>
#include <sys/time.h>

using namespace std;
using namespace stapl;

struct rand_gen_cont {
  static const char alfa[4];
  typedef char value_type;
  rand_gen_cont() {
    srand((stapl::get_location_id()+100)*3+stapl::get_location_id());
  }
  value_type get_element(int idx) { return alfa[rand()%4]; }

  size_t version(void) const
  {
    return 0;
  }
};

struct super_struct {
  static int n;
  typedef int value_type;
  super_struct() { }
  value_type get_element(int idx) { return n++;}
};

int super_struct::n = 0;
const char rand_gen_cont::alfa[] = {'A','B','C','D'};


struct strmatch {
  string cad;
  strmatch(string _cad) : cad(_cad) {}

  template<typename View>
  bool operator()(View x) const {
    return std::equal(cad.begin(),cad.end(),x.begin());
  }
};

void PrintMessage(const char* test, bool passed=true) {
  if(passed)
    fprintf(stderr, "%s: Passed\n", test);
  else
    fprintf(stderr, "%s: Failed\n", test);
}


stapl::exit_code stapl_main(int argc, char* argv[])
{
  typedef indexed_domain<size_t>       vec_dom_type;

  int nelem = 100;
  int nfill = 0;

  if (argc>=2)
    nelem = atoi(argv[1]);

  if (argc==3)
    nfill = atoi(argv[2]);


  //  cout << "argv[1] " << nelem << " argv[2] " << nfill << endl;

  typedef array<char>                                  p_string_type;
  typedef array<int>                                   p_int_type;
  typedef array_view<p_string_type,vec_dom_type>       pstringView;
  typedef array_view<p_array<int>, vec_dom_type>       pintView;
  typedef array_view<rand_gen_cont,vec_dom_type>       randGenView;
  typedef array_view<super_struct, vec_dom_type>       superView;

  p_string_type pstr(nelem);
  p_string_type pstr1(nelem);
  p_string_type pstr2(nelem);
  p_string_type pstr3(nelem);
  p_string_type pstr4(nelem);
  p_string_type pstr5(nelem);
  p_string_type pstr6(nelem);
  p_string_type pstr7(nelem);
  //  p_array<int> pint(nelem);

  pstringView pstrv(&pstr);
  pstringView pstrv1(&pstr);
  pstringView pstrv2(&pstr2);
  pstringView pstrv3(&pstr);
  pstringView pstrv4(&pstr2);
  pstringView pstrv5(&pstr);
  pstringView pstrv6(&pstr2);
  pstringView pstrv7(&pstr);


  rand_gen_cont rgc;
  randGenView rgenv(&rgc,vec_dom_type(pstr.size()));

  super_struct ss;
  superView sview(&ss, vec_dom_type(nelem));


  // Copy from a generator view into the parray
  copy(rgenv,pstrv);
  copy(rgenv,pstrv1);
  copy(rgenv,pstrv2);
  copy(rgenv,pstrv3);
  copy(rgenv,pstrv4);
  copy(rgenv,pstrv5);
  copy(rgenv,pstrv6);
  copy(rgenv,pstrv7);

  bool passed;
  pstringView::iterator iter;
  pstringView::iterator iter2;
  int i,j;



  fill(pstrv1,'X');

  passed  = true;
  iter = pstrv1.begin();
  for(i=0;iter!=pstrv1.end();i++, iter++) {
    if((*iter)!='X') passed = false;
  }
  if(i!=nelem) passed = false;
  PrintMessage("fill", passed);

  fill_n(pstrv2,'X',nfill);

  passed = true;
  iter = pstrv2.begin();

  for(i=0;i<nfill && iter!=pstrv2.end();i++, iter++) {
    if((*iter)!='X') passed = false;
  }
  for(j=i; iter!=pstrv2.end(); j++, iter++) {
    if((*iter)=='X') passed = false;
  }
  if(j!=nelem) passed = false;
  PrintMessage("fill_n", passed);


  replace(pstrv3,'A','T');

  passed = true;
  iter = pstrv3.begin();
  iter2 = pstrv.begin();
  for(i=0; i<nelem && iter!=pstrv3.end() && iter2!=pstrv.end(); i++, iter++, iter2++) {
    if( (*iter)==(*iter2) || ((*iter)=='T' && (*iter)=='A'))
      continue;
    else {
      passed = false;
      break;
    }
  }
  if(i!=nelem) passed = false;
  PrintMessage("replace", passed);


  replace_copy(pstrv,pstrv4, 'A', 'T');

  passed = true;
  iter = pstrv4.begin();
  iter2 = pstrv.begin();
  for(i=0; i<nelem && iter!=pstrv4.end() && iter2!=pstrv.end(); i++, iter++, iter2++) {
    if( (*iter)==(*iter2) || ((*iter)=='T' && (*iter)=='A'))
      continue;
    else {
      passed = false;
      break;
    }
  }
  if(i!=nelem) passed = false;
  PrintMessage("replace_copy", passed);

  return EXIT_SUCCESS;
}
