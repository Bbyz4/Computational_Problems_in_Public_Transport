#!/bin/bash

set -e

echo "Compiling..."
g++ -o test main.cpp Graph.cpp algs/Dijkstra.cpp algs/Astar.cpp algs/BiDirectionalDijkstra.cpp algs/BiDirectionalAstar.cpp algs/ALT.cpp algs/ARC_Dijkstra.cpp algs/ARC_Astar.cpp algs/ARC_ALT.cpp

echo "Running test..."
./test graph_data.txt queries.txt