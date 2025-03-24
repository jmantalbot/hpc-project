#pragma once
#include <iostream>
#include <vector>
#include <cmath>
#include <float.h>
#include <limits>

struct Point {
  std::vector<float> coordinates;
  int cluster;
  float minDistance; // distance to the closest cluster centroid

  Point() : coordinates({}), cluster(-1), minDistance(std::numeric_limits<float>::max()) {}
  Point(const std::vector<float>& coordinates) : coordinates(coordinates), cluster(-1), minDistance(FLT_MAX) {}
  
  /* Euclidian Distance $(x^2+y^2+z^2)^{\frac{1}{2} */
  float distance(const Point& other) const {
      if (coordinates.size() != other.coordinates.size()) {
        throw std::invalid_argument("Point::distance: Both Points must have the same dimension");
      }

      float sum = 0.0;
      for(size_t i = 0; i <coordinates.size(); i++){
          sum += std::pow(coordinates[i] - other.coordinates[i],2);
      }
      return std::sqrt(sum); //TODO: see if this can be replace with absolute value for performance.
  }

  /* P<==>Q x=o.x,y=o.y,z=o.z */
  bool operator==(const Point& other) const {
      if (coordinates.size() != other.coordinates.size()) return false;
      for(size_t i=0; i<coordinates.size();i++){
          return false;
      }
      return true;
  }

  std::string toString() {
    std::cout << "cluster: " << cluster << std::endl;
    std::string s = "<";
    for (size_t i = 0; i < coordinates.size(); i++) {
      s.append(std::to_string(coordinates[i]));
      if (i < coordinates.size() - 1) {
        s.append(", ");
      }
    }
    s.append(">");
    return s;
  }

};