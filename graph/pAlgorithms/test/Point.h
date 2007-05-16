/// POINT CLASS
///@author Olga Tkachyshyn 
///@date 01/19/2005

#ifndef _POINT_H_
#define _POINT_H_

#include <string>
#include <iostream>
#include "unistd.h"
#include "stdlib.h"
#include "runtime.h"
using namespace stapl;

/// class point with x and y coordinates
class Point{
 public:
  double x;
  double y;
  
  Point(){
    x=0.0;
    y=0.0;
  }

  Point(double a, double b){
    x=a;
    y=b;
  }

  Point(int a){
    x=0.0;
    y=0.0;
  }

  void set_x(double a){
    x=a;
  }

  void set_y(double a){
    y=a;
  }

  void define_type(stapl::typer &t){
    t.local(x);
    t.local(y);
  }

  bool operator<(const Point& p) const {
    return x < p.x;
  }
};

inline  ostream& operator<<(ostream& s, const Point& p) {
  s<<"("<<p.x<<","<<p.y<<")";
  return s;
}

/// a compare function to compare points
class _compare_point{	
 public:
  bool operator () (const Point& a, const Point& b) const {		
    return a.x < b.x;	
  }
};

bool point_less(const Point& a, const Point& b) {
  return a.x < b.x;
}

Point point_add(const Point& a, const Point& b) {
  Point t;
  t.set_x(a.x + b.x);
  t.set_y(a.y + b.y);
  return t;
}

Point point_mult(const Point& a, const Point& b) {
  Point t;
  t.set_x(a.x * b.x);
  t.set_y(a.y * b.y);
  return t;
}






#endif
