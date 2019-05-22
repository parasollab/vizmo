#include "Basic.h"
#include "EulerAngle.h"
#include "GaussianRand.h"
#include "Quaternion.h"
#include "Transformation.h"
#include "Vector.h"

using namespace mathtool;

int main(int argc, char ** argv ) {

  GaussianRand g;
  double g1 = g(2, 0.1), g2 = g(2, 0.1);
  std::cout << "Random Gaussian numbers from N~(2, 0.1):: " << g1 << " " << g2 << std::endl;

  int sec_num =102;
  Point2d p1(-51.8, 40.2);
  Point2d p2(-57.5, 38.6);
  Point2d p3(-55.3, 68.2);
  Point2d p4(-72.5, 65  );
  int max_rows =  8;
  double row_spacing = 1.02;
  double col_spacing = 4;

  Vector2d row_dir = row_spacing * (p2-p1).normalized();
  Vector2d col_dir = col_spacing * (p3-p1).normalized();

  std::cout << "row dir: " << row_dir << " col dir: " << col_dir << std::endl;
  double row_dist1 = (p1-p2).norm();
  double row_dist2 = (p3-p4).norm();
  for(int row=0; row<max_rows; row++) {

    Point2d start_pt = p1 + (1.0*row) * col_dir;
    double row_dist_i = row_dist1 + row*(row_dist2-row_dist1)/max_rows;
    double dist_traveled=0;
    int seat_num=0;
    while(dist_traveled < row_dist_i) {
      Point2d Npt = start_pt + (1.0*seat_num)*row_dir;
      std::cout << sec_num << "0"<<row;
      if(seat_num < 10 ) {std::cout << "0";}
      std::cout << seat_num << " pos " << Npt << " num 1" << std::endl;
      dist_traveled+=row_spacing;
      seat_num++;
    }
  }//endfor row

  //vector3d example
  Vector3d n1(2,2);
  std::cout << " n1: " << n1 << std::endl;
  Vector3d n2 = n1.normalized();
  std::cout << " n2(normed): " << n2 << std::endl;
  n1.normalize();
  std::cout << " n1(normed): " << n1 << std::endl;

  return 1;
}
