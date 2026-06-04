#pragma once

#include "../Algorithm.h"
#include <vector>
#include <functional>
#include <utility>

class ARC_ALT : public Algorithm
{
private:
    const Graph* graph_;
    int num_landmarks_;

    // Arc‑flag data
    std::vector<int> regions_;             
    std::vector<std::vector<bool>> flags_;   

    // ALT data
    std::vector<int> landmarks_;
    std::vector<std::vector<double>> dist_from_landmark_;
    std::vector<std::vector<double>> dist_to_landmark_;
    std::vector<std::vector<std::pair<int, double>>> rev_adj_; 

    // Query state
    std::vector<double> distances_;
    std::vector<int> predecessors_;
    std::vector<bool> visited_;

    // Helper functions (same as in ALT)
    void DijkstraFromLandmark(int landmark, std::vector<double>& dist);
    void DijkstraToLandmark(int landmark, std::vector<double>& dist);

public:
    explicit ARC_ALT(int k = 10);

    void Preprocess(const Graph& graph) override;
    std::vector<int> Query(int source, int target) override;
};