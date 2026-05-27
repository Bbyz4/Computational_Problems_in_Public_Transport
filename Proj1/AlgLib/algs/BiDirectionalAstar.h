#pragma once

#include "../Algorithm.h"
#include <vector>
#include <functional>

class BiDirectionalAstar : public Algorithm
{
    private:
        const Graph* graph_; 
        std::vector<double> distances_f, distances_b;
        std::vector<int> predecessors_f, predecessors_b;
        std::vector<bool> visited_f, visited_b;

        std::vector<uint32_t> rev_offsets;
        std::vector<int> rev_edges;
        std::vector<double> rev_weights;

        std::function<double(int,int)> heuristic_;

        void BuildReverseGraph_(const Graph& graph);

    public:
        explicit BiDirectionalAstar(std::function<double(int,int)> h = nullptr);

        void Preprocess(const Graph& graph) override;
        std::vector<int> Query(int source, int target) override;
};