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
    double x,y,z;
    Point(double x=0, double y = 0, z=0): x(x), y(y), z(z) {}

    /* Euclidian Distance $(x^2+y^2+z^2)^{\frac{1}{2} */
    double distance(const Point& other) const {
        return std::sqrt(std::pow(x- other.x,2) + std::pow(y-other.y,2) + std::pow(z-other.z,2));
    }
    /* P<==>Q x=o.x,y=o.y,z=o.z */
    bool operator==(const Point& other) const {
        return x == other.x && y == other.y && z == other.z;
    }
};


Point calc_centroids(const std::vector<Point>& cluster){
    double sumX = 0, sumY = 0, sumZ = 0;
    /* For each point in the cluster on the interval [0,1]*/
    for (const auto& point: cluster) {
        sumX += point.x;
        sumY += point.y;
        sumZ += point.z; 
    /* return the new center for the cluster*/
    return point(sumX / cluster.size(), sumY / cluster.size(), sumZ / cluster.size());
}


int k_means_cluster(int k, std::vector<std::vector<int>> points){ 
    std::vector<Point> centroids(k);
    for (int i = 0; i<k; i++){
        double x = static_cast<double>(rand()) / RAND_MAX;
        double y = static_cast<double>(rand()) / RAND_MAX;
        double z = static_cast<double>(rand()) / RAND_MAX;
        centroids[i] = Point(x,y,z);
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



