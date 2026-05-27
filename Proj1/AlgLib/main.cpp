#include "Graph.h"
#include "Algorithm.h"
#include "algs/Dijkstra.h"
#include "algs/Astar.h"
#include "algs/BiDirectionalDijkstra.h"
#include "algs/BiDirectionalAstar.h"

#include<iostream>
#include<fstream>
#include<sstream>
#include<vector>
#include<queue>
#include<limits>
#include<algorithm>
#include<memory>
#include<chrono>
#include<cmath>

double ComputePathCost(const Graph& graph, const std::vector<int>& path)
{
    if(path.size() < 2)
    {
        return 0.0;
    }

    double total = 0.0;

    for(int i = 0; i < path.size() - 1; i++)
    {
        int u = path[i];
        int v = path[i+1];
        bool found = false;

        for(uint32_t e = graph.offset_begin(u); e < graph.offset_end(u); e++)
        {
            if(graph.edges()[e] == v)
            {
                total += graph.weights()[e];
                found = true;
                break;
            }
        }

        if(!found)
        {
            return std::numeric_limits<double>::infinity();
        }
    }

    return total;
}

double SimplifiedDistance(const Graph& graph, int a, int b, double scaleFactor)
{
    double lat_a = graph.lat(a);
    double lon_a = graph.lon(a);
    double lat_b = graph.lat(b);
    double lon_b = graph.lon(b);

    double dx = (lon_b - lon_a);
    double dy = (lat_b - lat_a);
    return scaleFactor * std::sqrt(dx*dx + dy*dy);
}

int main(int argc, char* argv[])
{
    if(argc != 3)
    {
        std::cerr << "Wrong arg number\n";
        return 1;
    }

    Graph graph;
    std::ifstream graphFile(argv[1]);

    if(!graphFile)
    {
        std::cerr << "Graph file cannot be opened\n";
        return 1;
    }

    std::cout << "Building the graph\n";

    std::string line;
    while(std::getline(graphFile, line))
    {
        if(line.empty())
        {
            continue;
        }

        std::istringstream iss(line);
        std::string type;
        iss >> type;
        if(type == "NODE")
        {
            int id;
            double lat, lon;
            iss >> id >> lat >> lon;

            graph.add_node(lat, lon);
        }
        else if(type == "EDGE")
        {
            int from, to;
            double weight;
            iss >> from >> to >> weight;
            
            graph.add_edge(from, to, weight);
        }
    }

    std::ifstream queryFile(argv[2]);
    if(!queryFile)
    {
        std::cerr << "Query file cannot be opened\n";
        return 1;
    }

    graph.build();
    std::cout << "Graph built\n";

    std::vector<std::pair<int, int>> queries;
    while(std::getline(queryFile, line))
    {
        if(line.empty())
        {
            continue;
        }

        std::istringstream iss(line);
        std::string type;
        iss >> type;

        if(type == "QUERY")
        {
            int src, tgt;
            iss >> src >> tgt;
            queries.emplace_back(src, tgt);
        }
    }

    // LIST OF ALGORITHMS TO CHECK
    std::vector<std::unique_ptr<Algorithm>> algorithms;

    algorithms.push_back(std::make_unique<Dijkstra>());

    auto heuristics = [&graph](int node, int target)
    {
        return SimplifiedDistance(graph, node, target, 0.95);
    };

    algorithms.push_back(std::make_unique<Astar>(heuristics));

    algorithms.push_back(std::make_unique<BiDirectionalDijkstra>());

    auto heuristics2 = [&graph](int node, int target)
    {
        return SimplifiedDistance(graph, node, target, 0.5);
    };

    algorithms.push_back(std::make_unique<BiDirectionalAstar>(heuristics2));

    //Ground truth is built using Dijkstra -------------------------------
    std::vector<double> ground_truth_costs(queries.size(), 0.0);

    Dijkstra dijkstra;
    dijkstra.Preprocess(graph);

    std::cout << "Running first Dijkstra...\n";

    for(int i=0; i<queries.size(); i++)
    {
        int src = queries[i].first;
        int tgt = queries[i].second;
        std::vector<int> path = dijkstra.Query(src, tgt);
        double cost = ComputePathCost(graph, path);
        ground_truth_costs[i] = cost;
    }

    std::cout << "Ground truth established!\n";

    for(auto& algo : algorithms)
    {
        // PREPROCESS TIMING
        auto preprocess_start =
            std::chrono::high_resolution_clock::now();

        algo->Preprocess(graph);

        auto preprocess_end =
            std::chrono::high_resolution_clock::now();

        auto preprocess_time =
            std::chrono::duration_cast<std::chrono::microseconds>(
                preprocess_end - preprocess_start
            );

        // QUERY TIMING
        auto query_start =
            std::chrono::high_resolution_clock::now();

        for(int q=0; q<queries.size(); q++)
        {
            auto Q = queries[q];

            std::vector<int> path =
                algo->Query(Q.first, Q.second);

            double computed_cost = ComputePathCost(graph, path);
            double expected_cost = ground_truth_costs[q];
        
            const double EPS = 1e-6;
            if(std::fabs(computed_cost - expected_cost) > EPS)
            {
                std::cout << "WARNING - MISMATCH FOR QUERY NR " << q << " !!!\n";
            }
        }

        auto query_end =
            std::chrono::high_resolution_clock::now();

        auto total_query_time =
            std::chrono::duration_cast<std::chrono::microseconds>(
                query_end - query_start
            );

        double average_query_time = 0.0;

        if(!queries.empty())
        {
            average_query_time =
                static_cast<double>(total_query_time.count()) /
                static_cast<double>(queries.size());
        }

        std::cout << "Algorithm done!\n";

        std::cout
            << "Preprocessing time: "
            << preprocess_time.count()
            << " us\n";

        std::cout
            << "Total query time: "
            << total_query_time.count()
            << " us\n";

        std::cout
            << "Average query time: "
            << average_query_time
            << " us\n";
    }

    return 0;
}