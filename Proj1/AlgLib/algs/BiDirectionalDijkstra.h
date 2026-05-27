#pragma once

#include "../Algorithm.h"
#include <vector>
#include <queue>
#include <limits>

class BiDirectionalDijkstra : public Algorithm
{
    private:
        const Graph* graph_; 
        std::vector<double> distances_f, distances_b;
        std::vector<int> predecessors_f, predecessors_b;
        std::vector<bool> visited_f, visited_b;

        std::vector<uint32_t> rev_offsets;
        std::vector<int> rev_edges;
        std::vector<double> rev_weights;

        void BuildReverseGraph_(const Graph& graph);

    public:
        void Preprocess(const Graph& graph) override;
        std::vector<int> Query(int source, int target) override;
};