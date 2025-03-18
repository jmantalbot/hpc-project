#include <iostream>
#include <vector>
#include <iomanip>

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










std::vector<int> choose_centroids(std::vector<int> centroids){
    return centroids;
}


int main(int argc, char* argv[]){
    if(argc != 3){
        std::cerr << "Usage: " << argv[0] << " <some integer k where k>0> <file continaing points>" << std::endl;
    }
    int k = std::__cxx11::stoi(argv[1]);
    std::vector<int> centroids(k);
    centroids = choose_centroids(centroids);
    
    
    
    bool converged = false;
    while(!converged){
        std::vector<std::vector<float>> clusters(k);
    }
}



