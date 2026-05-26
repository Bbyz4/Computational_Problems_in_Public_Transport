#include <limits>
#include <queue>
#include <algorithm>
#include "Astar.h"

Astar::Astar(std::function<double(int,int)> h) : heuristic_(h ? h : [](int,int) {return 0.0; })
{

}

void Astar::Preprocess(const Graph& graph)
{
    graph_ = &graph;
}

std::vector<int> Astar::Query(int source, int target)
{
    const int n = graph_->num_nodes();
    const double INF = std::numeric_limits<double>::infinity();

    distances_.assign(n, INF);
    predecessors_.assign(n, -1);
    visited_.assign(n, false);

    using NodeF = std::pair<double, int>;
    std::priority_queue<NodeF, std::vector<NodeF>, std::greater<>> pq;

    distances_[source] = 0.0;
    double f_start = distances_[source] + heuristic_(source, target);
    pq.emplace(f_start, source);

    while(!pq.empty())
    {
        NodeF top = pq.top();
        pq.pop();

        double f_u = top.first;
        int u = top.second;

        if(visited_[u])
        {
            continue;
        }

        visited_[u] = true;

        if(u == target)
        {
            break;
        }

        for(uint32_t e = graph_->offset_begin(u); e < graph_->offset_end(u); e++)
        {
            int v = graph_->edges()[e];
            double w = graph_->weights()[e];
            double new_dist = distances_[u] + w;

            if(new_dist < distances_[v])
            {
                distances_[v] = new_dist;
                predecessors_[v] = u;
                double f_v = new_dist + heuristic_(v, target);
                pq.emplace(f_v, v);
            }
        }
    }

    std::vector<int> path;
    
    if(distances_[target] == INF)
    {
        return path;
    }

    for(int v = target; v != -1; v = predecessors_[v])
    {
        path.push_back(v);
    }

    std::reverse(path.begin(), path.end());
    return path;
}