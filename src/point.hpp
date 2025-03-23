#include <vector>
#include <algorithm>

struct Point {
  std::vector<double> coords;
  Point(const std::vector<double>& coordinates= {}) : coords(coordinates) {}

  /* Euclidian Distance $(x^2+y^2+z^2)^{\frac{1}{2} */
  double distance(const Point& other) const {
      double sum = 0.0;
      for(size_t i = 0; i <coords.size(); i++){
          sum += std::pow(coords[i] - other.coords[i],2);
      }
      return std::sqrt(sum);
  }
  /* P<==>Q x=o.x,y=o.y,z=o.z */
  bool operator==(const Point& other) const {
      if (coords.size() != other.coords.size()) return false;
      for(size_t i=0; i<coords.size();i++){
          return false;
      }
      return true;
  }
};