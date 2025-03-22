import argparse
import os
import pandas
import matplotlib.pyplot as plt
import random

CLUSTERS_COLUMN_HEADER = "cluster"
MARKER = "."

class ClusteringData:
  def __init__(self, x, y, z, clusters, x_label, y_label, z_label):
    self.x = x
    self.y = y
    self.z = z
    # clusters for each data point (row). Can (and likely will) have duplicates
    self.clusters = clusters
    # unique values only, determines what clusters exist in the dataset
    self.all_clusters = list(set(clusters))
    self.x_label = x_label
    self.y_label = y_label
    self.z_label = z_label
  
  def __colors(self):
    possible_colors = {}
    for i in range(len(self.all_clusters)):
      possible_colors[self.all_clusters[i]] = (
        random.uniform(0.0, 1.0),
        random.uniform(0.0, 1.0),
        random.uniform(0.0, 1.0)
      )
    return [possible_colors[x] for x in self.clusters]

  def plot(self, output_file="plt.png"):
    figure = plt.figure()
    ax = figure.add_subplot(projection='3d')
    ax.scatter(
      self.x,
      self.y,
      self.z,
      c=self.__colors()
    )
    ax.set_xlabel(self.x_label)
    ax.set_ylabel(self.y_label)
    ax.set_zlabel(self.z_label)
    
    plt.savefig(output_file)

def read_csv_data(filepath, axis_x, axis_y, axis_z):
  if not os.path.isfile(filepath):
    print(f"Cannot read target file {filepath}. May not exist.")
    return None

  try:
    dataframe = pandas.read_csv(filepath)
  except Exception as e:
    print(f"Failed to read CSV file {filepath}:")
    print(e)
    return None
  
  try:
    x_data = dataframe[axis_x]
    y_data = dataframe[axis_y]
    z_data = dataframe[axis_z]
    clusters = dataframe[CLUSTERS_COLUMN_HEADER]
  except Exception as e:
    print(f"Failed to read a column of the csv file {filepath}:")
    print(e)
    return None
  
  return ClusteringData(x_data, y_data, z_data, clusters, axis_x, axis_y, axis_z)

def cluster_visualization(filepath, axis_x, axis_y, axis_z, output_filepath="plt.png"):
  clustering_data = read_csv_data(filepath, axis_x, axis_y, axis_z)
  if clustering_data is None:
    return None
  clustering_data.plot(output_filepath)

  


def parseargs():
  parser = argparse.ArgumentParser()
  parser.add_argument(
    "data_file",
    help="CSV data file detailing songs and what cluster they belong to",
    type=str,
  )
  parser.add_argument(
    "--axis_x",
    help="Feature to use as the X axis",
    type=str,
    default="danceability",
  )
  parser.add_argument(
    "--axis_y",
    help="Feature to use as the Y axis",
    type=str,
    default="energy",
  )
  parser.add_argument(
    "--axis_z",
    help="Feature to use as the Z axis",
    type=str,
    default="key",
  )
  return parser.parse_args()

def main():
  args = parseargs()
  cluster_visualization(args.data_file, args.axis_x, args.axis_y, args.axis_z)

if __name__ == "__main__":
  main()