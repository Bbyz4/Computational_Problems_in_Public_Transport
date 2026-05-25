#include "Graph.h"
#include "Algorithm.h"
#include "algs/Dijkstra.h"

#include<iostream>
#include<fstream>
#include<sstream>
#include<vector>
#include<queue>
#include<limits>
#include<algorithm>
#include<memory>
#include<chrono>

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

        for(const auto& Q : queries)
        {
            std::vector<int> path =
                algo->Query(Q.first, Q.second);

            // potentially do something with the results
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