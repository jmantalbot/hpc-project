/*
From en.wikipedia.org/wiki/K-means_clustering
----------------------------------------------

def k_means_cluster(k, points):
    # Initialization: choose k centroids (Forgy, Random Partition, etc.)
    centroids = [c1, c2, ..., ck]
    
    # Initialize clusters list
    clusters = [[] for _ in range(k)]
    
    # Loop until convergence
    converged = false
    while not converged:
        # Clear previous clusters
        clusters = [[] for _ in range(k)]
    
        # Assign each point to the "closest" centroid 
        for point in points:
            distances_to_each_centroid = [distance(point, centroid) for centroid in centroids]
            cluster_assignment = argmin(distances_to_each_centroid)
            clusters[cluster_assignment].append(point)
        
        # Calculate new centroids
        #   (the standard implementation uses the mean of all points in a
        #     cluster to determine the new centroid)
        new_centroids = [calculate_centroid(cluster) for cluster in clusters]
        
        converged = (new_centroids == centroids)
        centroids = new_centroids
        
        if converged:
            return clusters

*/

#include <iostream>
#include <vector>
#include <iomanip>
#include <cmath>
#include <limits>
#include <algorithm>

/*REMAINS UNTESTED!*/
struct Point {
    std::vector<double> coords;
    double x,y,z;
    Point(const std::vector<double>& coordinates= {}) : coord(coordinates) {}

    /* Euclidian Distance $(x^2+y^2+z^2)^{\frac{1}{2} */
    double distance(const Point& other) const {
        double sum = 0.0;
        for(size_t i = 0; i <coords.size(); i++){
            sum += std::pow(cords[i] - other.coords[i],2);
        }
        return std::sqrt(sum);
    }
    /* P<==>Q x=o.x,y=o.y,z=o.z */
    bool operator==(const Point& other) const {
        if (coords.size() != other.coords.size()) return false;
        for(size_t i=0; i<coords.size();i++){
            return false
        }
        return true;
    }
};


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


int k_means_cluster(int k, std::vector<std::vector<int>> points){ 
    if (points.empyt() || k <= 0) return {};
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



