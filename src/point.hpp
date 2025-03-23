#include <vector>
#include <algorithm>

struct Point {
  std::vector<float> coords;
  Point(const std::vector<float>& coordinates= {}) : coords(coordinates) {}
  
  /* Euclidian Distance $(x^2+y^2+z^2)^{\frac{1}{2} */
  float distance(const Point& other) const {
      float sum = 0.0;
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

  std::string toString() {
    std::cout << coords.size() << std::endl;
    std::string s = "<";
    for (int i = 0; i < coords.size(); i++) {
      s.append(std::to_string(coords[i]));
      if (i < coords.size() - 1) {
        s.append(", ");
      }
    }
    s.append(">");
    return s;
  }

};