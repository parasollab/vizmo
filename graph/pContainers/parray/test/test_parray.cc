#include "pArray.h"
#include "pRange.h"

using namespace stapl;

class assignx{
public:
  assignx(){}
  int operator()(int& data){
    data = 2334;
    return OK;
  }
  void define_type(stapl::typer &t){}
};

void stapl_print(const char* s){
  if(stapl::get_thread_id() == 0)
    cout << s << flush;
}

void stapl_main(int argc, char *argv[]) {
  
  if (argc<2){
    printf("\n Usage driver pArray : > driver_pArray #elements");
    exit(0);
  }

  typedef stapl::pArray<int> PARRAY;
  typedef stapl::linear_boundary<PARRAY::iterator> linearBoundaryType;
  
  int myid=stapl::get_thread_id();
  int nprocs=stapl::get_num_threads();
  int N=atoi(argv[1]);
  int i=0;
  //=======================================
  // empty pArray
  //======================================= 
  stapl_print("testing implicit pArray constructor...");
  PARRAY p_ax(N);
  stapl_print("Passed\n"); 


  //=======================================
  // pArray with default distribution
  //=======================================
  
  stapl_print("testing specific pArray constructor with default distribution...");
  PARRAY p_ay(N);  
  stapl::rmi_fence();
  //build links
  if (nprocs==1) {
    p_ay.InitBoundaryInfo(0, Location(INVALID_PID,INVALID_PART), Location(INVALID_PID,INVALID_PART) );
  } else {
    if (myid==0) {
      p_ay.InitBoundaryInfo(0, Location(INVALID_PID,INVALID_PART), Location(1,0) );
    } else if (myid==nprocs-1) {
      p_ay.InitBoundaryInfo(0, Location(myid-1, 0), Location(INVALID_PID,INVALID_PART) );
    } else {
      p_ay.InitBoundaryInfo(0, Location(myid-1, 0), Location(myid+1, 0) );
    }
  }
  stapl::rmi_fence();

  if (p_ay.size()!=N) {
    stapl_print("\nError in specific pArray constructor 1, size method");
  } else {
    stapl_print("Passed\n");
  }

  stapl_print("testing SetElement...");
  // filling a pArray; test for SetElement(index value)
  if (myid==0){
    for (i=0; i < N; i++){
      p_ay.SetElement(i,i);
    }
  }
  stapl::rmi_fence();
  stapl_print("Passed\n");
  stapl_print("testing GetElement...");
  if (myid==0){
    for (i=0; i<N; i++){
      if ( p_ay.GetElement(i) != i)
	stapl_print("Error in pArray GetElement\n"); 
    }
  }
  stapl_print("Passed\n");

  stapl_print("testing get_prange and pArray(PRange,bool)...");
  PARRAY::PRange mypr(&p_ay);
  p_ay.get_prange(mypr);
  stapl::rmi_fence();
  PARRAY copy1(mypr, true);
  stapl::rmi_fence();
  if (myid==0){
    for (i=0; i<N; i++){
      if ( copy1.GetElement(i) != i)
	stapl_print("Error in testing get_prange and pArray(PRange,true)\n");
    }
  }
  stapl::rmi_fence();
  PARRAY copy2(mypr, false);
  stapl::rmi_fence();
  if (copy2.local_size() != p_ay.get_num_parts() )
    stapl_print("Error in testing get_prange and pArray(PRange,false)\n");
  stapl_print("Passed\n");

  stapl_print("testing UpdateElement...");
  assignx ax;
  if(myid == nprocs - 1){ 
    p_ay.UpdateElement(2 ,ax);
  }
  stapl::rmi_fence();
  if(p_ay.GetElement(2) != 2334){
    cout<<"Error while testing update element"<<endl;
  } else {
    stapl_print("Passed\n");
  }
  stapl::rmi_fence();

  stapl_print("testing multiplying by a scalar and [] operator...");
  p_ax = 2 * p_ay;
  stapl::rmi_fence();
  if (myid==0){
    bool passed = true;
    for (i=0; i<N; i++){
      if ( p_ax[i] != p_ay[i] * 2) {
	cout<<"Error in pArray, p_ax=p_ay/2"<<endl; 
        passed = false;
      }
    }
    if (passed) {
      stapl_print("Passed\n");
    }
  }  
  stapl::rmi_fence();
  
  stapl_print("testing adding a scalar and [] operator...");
  //testing adding scalar
  p_ax =  7 + p_ax;
  p_ax =  p_ax + 7;
  p_ax = p_ax - 7;
  stapl::rmi_fence();

  if (myid==0){
    bool passed = true;
    for (i=0; i<N; i++){
      if (p_ax[i]!=2*p_ay[i]+7) {
	cout<<"Error in pArray, p_ax=2*p_ay+7"<<endl; 
        passed = false;
      }
    }
    if (passed) {
      stapl_print("Passed\n");
    }
  }
  stapl::rmi_fence();

  stapl_print("testing accumulate...");
  PARRAY a(N);
  int total_acc=0,total_dot=0;
  double norm = 0.0;

  stapl::rmi_fence();

  if(myid ==0){
    for (i=0; i<N; ++i){
      a.SetElement(i,i);
      total_acc += i;
      total_dot += i*i;
    }
    norm = sqrt((long double)total_dot);
  }
  stapl::rmi_fence();
  //testing accumulate
  int accum = a.accumulate();
  stapl::rmi_fence();
  if (myid==0){
    if (accum != total_acc) {
      cout<<"Error in accumulate"<<endl;
    } else {
      stapl_print("Passed\n");
    }
  }

  stapl_print("testing dot product...");
  //testing dot product
  int dot = a.dotproduct(a);
  stapl::rmi_fence();
  if (myid==0){
    if (dot!=total_dot) {
      cout<<"Error in dot"<<endl;
    } else {
      stapl_print("Passed\n");
    }
  }  

  stapl_print("testing euclidean...");

  long double d = a.euclideannorm();
  stapl::rmi_fence();
  if (myid==0){
    if (d - norm > 0.0000001) {
      cout << "Error in euclidean norm" << d << " != " << norm << endl;
    } else {
      stapl_print("Passed\n");
    }
  }

  stapl_print("test pArray iterator...\n");
  pArray<int>::iterator it1 = p_ax.local_begin();
  cout <<"...begin"<<*it1<<endl;
  it1 = it1 + 5;
  cout<<"...plus 2:"<<*it1<<endl;
  it1 = it1 - 2;
  cout<<"...minus 2:"<<*it1<<endl;
  stapl_print("calling local sort ... ");
  sort(p_ax.local_begin(),p_ax.local_end());
  stapl_print("Passed\n");

  stapl_print("testing specific distribution\n");
  //vector<ArrayDistributionInfo> user_dist;
  pArray<int>::ArrayDistributionInfo_type userdist;
  ArrayPartsDistributionInfo temp;
  
  for (i=0; i<stapl::get_num_threads(); i++){
    if (i == (stapl::get_num_threads()-1)){
      temp.SetSize(N - (N/stapl::get_num_threads())*(stapl::get_num_threads() - 1));
      temp.SetGID(i * (N/stapl::get_num_threads()));
      Location l(i,0);
      temp.SetLocation(l);
      userdist.push_back(temp);
    }
    else{
      temp.SetSize(N/stapl::get_num_threads());
      temp.SetGID(i*(N/stapl::get_num_threads()));
      Location l(i,0);
      temp.SetLocation(l);
      userdist.push_back(temp);
    }
  }
  rmi_fence();

  pArray<int> p_az(N,userdist);
  rmi_fence();
 
  if(myid ==0){
    for (i=0; i<N; ++i){
      p_az.SetElement(i,i);
    }
  }  
  stapl::rmi_fence();
  stapl_print("Passed\n");
  stapl_print("testing array array multiplication r[i] = s1[i] * s2[i]; also assignment op ...");

  pArray<int> w;
  stapl::rmi_fence();
    
  //testing multiplying 2 pArrays
  w = p_az * p_az;
  if (myid==0){
    bool passed = true;
    for (i=0; i<N; ++i){
      if (w[i] != p_az[i]*p_az[i]) {
        cout<<"Error in multiplying 2 pArrays"<<endl;
        passed = false;
      }
    }
    if (passed)
      stapl_print("Passed\n");
  }

  stapl_print("testing array array addition...");  
  //testing adding 2 pArrays
  p_az = w + w;
  if (myid==0){
    bool passed = true;
    for (i=0; i < N; ++i){
      if (2*w[i] != p_az[i]) {
        cout<<"Error in adding 2 pArrays"<<endl;
        passed = false;
      }
    }
    if (passed)
      stapl_print("Passed\n");
  }

  stapl_print("testing array array subtraction...");  
  w = p_az - w;
  if (myid==0){
    bool passed = true;
    for (i=0; i<N; ++i){
      if (2*w[i] != p_az[i]) {
        cout<<"Error in adding 2 pArrays"<<endl;
        passed = false;
      }
    }
    if (passed)
      stapl_print("Passed\n");
  }

  //operations with vectors
  vector<int> v_az(p_az.local_size(),10);
  vector<int>::iterator vit;
  pArray<int>::Part_type* part;
  pArray<int>::part_iterator pi;
  int current_index;

  vector<vector<int> > twodv(10);
  pArray<int> u;
  stapl::rmi_fence();
  if(myid ==0){
    for (i=0; i<N; ++i){
      p_az.SetElement(i,i);
    }
  }  
  stapl::rmi_fence();
  
  //here vector - pArray
  stapl_print("testing vector array subtraction...");  
  u = v_az - p_az;

  bool vec_minus_parray_passed = true;
  //from local begin to local end all processors should check the correctness
  vit = v_az.begin();
  //for all parts
  for (i=0; i < u.GetPartsCount();i++){
    current_index = u.GetPart(i)->start_index;
    part = u.GetPart(i);
    
    for (pi = part->begin(); pi != part->end(); ++pi,++vit){
      if(*pi != *vit - p_az.GetElement(current_index)){
        cout<<*pi<<":"<<*vit<<":"<< p_az.GetElement(current_index)<<endl;
        cout<<"Error while performing vector - parray on thread"<<myid<<" index:"<<current_index<<endl;
        vec_minus_parray_passed = false;
      }
      current_index++;
    }
  }

  if (vec_minus_parray_passed)
    stapl_print("Passed\n");

  //here pArray - vector
  stapl_print("testing array vector subtraction...");  
  u = p_az - v_az;

  vec_minus_parray_passed = true;
  //from local begin to local end all processors should check the correctness
  vit = v_az.begin();
  //for all parts
  for (i=0; i < u.GetPartsCount();i++){
    current_index = u.GetPart(i)->start_index;
    part = u.GetPart(i);
    
    for (pi = part->begin(); pi != part->end(); ++pi,++vit){
      if(*pi != p_az.GetElement(current_index) - *vit){
        cout<<*pi<<":"<<*vit<<":"<< p_az.GetElement(current_index)<<endl;
        cout<<"Error while performing parray - vector on thread"<<myid<<" index:"<<current_index<<endl;
        vec_minus_parray_passed = false;
      }
      current_index++;
    }
  }

  if (vec_minus_parray_passed)
    stapl_print("Passed\n");

  //testing the iterator for pArray

  //u = twodv * p_az;
  //u = p_az * twodv;
  //done
  stapl_print("done with the specific distribution\n");
  stapl_print("Parray test driver has successfully completed\n");    
  rmi_fence();
}
