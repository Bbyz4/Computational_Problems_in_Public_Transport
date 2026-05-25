#pragma once

#include "../Algorithm.h"
#include <vector>

class Dijkstra : public Algorithm
{
    private:
        const Graph* graph_; 
        std::vector<double> distances_;
        std::vector<int> predecessors_;
        std::vector<bool> visited_;

    public:
        void Preprocess(const Graph& graph) override;
        std::vector<int> Query(int source, int target) override;
};