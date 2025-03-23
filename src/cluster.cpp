#include <iostream>
#include <vector>
#include <iomanip>
#include <cmath>
#include <limits>
#include <algorithm>
#include <cstdlib>

#include "point.hpp"


Point calc_centroids(const std::vector<Point>& cluster){
    if(cluster.empty()) return Point();
    std::vector<double> sums(cluster[0].coords.size(), 0.0);
    for(const auto& point: cluster) {
        for(size_t i=0; i<sums.size(); i++){
            sums[i] += point.coords[i];
        }
    }
    for(auto& sum:sums){
        sum /= cluster.size();
    }
    return Point(sums);
}


std::vector<std::vector<Point>> k_means_cluster(int k, std::vector<Point>& points){ 
    if (points.empty() || k <= 0) return {};
    size_t dim = points[0].coords.size();
    for(const auto& p: points){
        if(p.coords.size() != dim){
            throw std::invalid_argument("All points must have the same dimension.");
        }
    }

    std::vector<Point> centroids(k);
    for (int i = 0; i<k; i++){
        std::vector<double> coords(dim);
        for(size_t j = 0; j<dim; j++){
            coords[j] = static_cast<double>(rand()) / RAND_MAX;
        }
        centroids[i] = Point(coords);
    }
    std::vector<std::vector<Point>> clusters(k);

    bool converged = false;
    while(!converged){
        for(auto& cluster: clusters){
            cluster.clear();
        }
        for(const auto& point: points) {
            double minD = std::numeric_limits<double>::max();
            int closest = 0;
            for(int i=0; i<k; i++){
                double d = point.distance(centroids[i]);
                if(d < minD){
                    minD = d;
                    closest = i;
                }
            }
            clusters[closest].push_back(point);
        }
        std::vector<Point> newCentroids(k);
        for(int i=0; i<k;i++){
            newCentroids[i] = clusters[i].empty() ? centroids[i] : calc_centroids(clusters[i]);
        }
        converged = (newCentroids == centroids);
        centroids = newCentroids;
    }
    return clusters;
}
