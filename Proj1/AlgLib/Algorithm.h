#pragma once

#include<vector>
#include "Graph.h"

class Algorithm
{
    public:
        virtual ~Algorithm() = default;
        virtual void Preprocess(const Graph& graph) = 0;
        virtual std::vector<int> Query(int source, int target) = 0;
};