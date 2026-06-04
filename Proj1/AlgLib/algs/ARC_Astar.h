#pragma once

#include "../Algorithm.h"
#include <vector>
#include <functional>

class ARC_Astar : public Algorithm
{
private:
    const Graph* graph_;
    std::vector<double> distances_;
    std::vector<int> predecessors_;
    std::vector<bool> visited_;
    std::function<double(int,int)> heuristic_;

    std::vector<int> regions_;         
    std::vector<std::vector<bool>> flags_; 

public:
    explicit ARC_Astar(std::function<double(int,int)> h = nullptr);

    void Preprocess(const Graph& graph) override;
    std::vector<int> Query(int source, int target) override;
};