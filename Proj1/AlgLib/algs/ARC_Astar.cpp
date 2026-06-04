#include <limits>
#include <queue>
#include <vector>
#include <algorithm>
#include <cmath>
#include <unordered_set>

#include "ARC_Astar.h"

ARC_Astar::ARC_Astar(std::function<double(int,int)> h)
    : heuristic_(h ? h : [](int,int) { return 0.0; })
{
}

void ARC_Astar::Preprocess(const Graph& graph)
{
    graph_ = &graph;
    const int n = graph_->num_nodes();
    const size_t num_edges = static_cast<size_t>(graph_->num_edges());

    const int reg_count_x = 4;
    const int reg_count_y = 4;
    const int region_count = reg_count_x * reg_count_y;

    double min_x = std::numeric_limits<double>::max();
    double max_x = std::numeric_limits<double>::min();
    double min_y = std::numeric_limits<double>::max();
    double max_y = std::numeric_limits<double>::min();

    for (int v = 0; v < n; ++v)
    {
        double x = graph_->lon(v);
        double y = graph_->lat(v);
        min_x = std::min(min_x, x);
        max_x = std::max(max_x, x);
        min_y = std::min(min_y, y);
        max_y = std::max(max_y, y);
    }

    double region_size_x = (max_x - min_x) / reg_count_x;
    double region_size_y = (max_y - min_y) / reg_count_y;

    regions_.assign(n, 0);
    for (int v = 0; v < n; ++v)
    {
        double x = graph_->lon(v);
        double y = graph_->lat(v);
        int rx = static_cast<int>(std::floor((x - min_x) / region_size_x));
        int ry = static_cast<int>(std::floor((y - min_y) / region_size_y));
        rx = std::min(rx, reg_count_x - 1);
        ry = std::min(ry, reg_count_y - 1);
        regions_[v] = ry * reg_count_x + rx;
    }

    flags_.assign(num_edges, std::vector<bool>(region_count, false));
    std::unordered_set<int> boundary_nodes;

    for (int u = 0; u < n; ++u)
    {
        for (uint32_t e = graph_->offset_begin(u); e < graph_->offset_end(u); ++e)
        {
            int v = static_cast<int>(graph_->edges()[e]);
            flags_[e][regions_[v]] = true;
            if (regions_[u] != regions_[v])
                boundary_nodes.insert(v);
        }
    }

    auto dijkstra_rev = [&](int s) -> std::vector<double>
    {
        const double INF = std::numeric_limits<double>::infinity();
        std::vector<double> dist(n, INF);
        using NodeDist = std::pair<double, int>;
        std::priority_queue<NodeDist, std::vector<NodeDist>, std::greater<>> pq;

        dist[s] = 0.0;
        pq.emplace(0.0, s);

        while (!pq.empty())
        {
            double d_u = pq.top().first;
            int u = pq.top().second;
            pq.pop();
            if (d_u > dist[u]) continue;
            for (uint32_t e = graph_->rev_offset_begin(u); e < graph_->rev_offset_end(u); ++e)
            {
                int v = static_cast<int>(graph_->rev_edges()[e]);
                double w = graph_->rev_weights()[e];
                double nd = d_u + w;
                if (nd < dist[v])
                {
                    dist[v] = nd;
                    pq.emplace(nd, v);
                }
            }
        }
        return dist;
    };

    for (int b : boundary_nodes)
    {
        std::vector<double> dist_rev = dijkstra_rev(b);
        for (int u = 0; u < n; ++u)
        {
            for (uint32_t e = graph_->offset_begin(u); e < graph_->offset_end(u); ++e)
            {
                int v = static_cast<int>(graph_->edges()[e]);
                double w = graph_->weights()[e];
                if (dist_rev[u] == std::numeric_limits<double>::infinity() ||
                    dist_rev[v] == std::numeric_limits<double>::infinity())
                    continue;

                if (std::abs(dist_rev[u] - dist_rev[v] - w) < 1e-9)
                    flags_[e][regions_[b]] = true;
            }
        }
    }
}

std::vector<int> ARC_Astar::Query(int source, int target)
{
    const int n = graph_->num_nodes();
    const double INF = std::numeric_limits<double>::infinity();

    distances_.assign(n, INF);
    predecessors_.assign(n, -1);
    visited_.assign(n, false);

    const int target_region = regions_[target];

    using NodeF = std::pair<double, int>; 
    std::priority_queue<NodeF, std::vector<NodeF>, std::greater<>> pq;

    distances_[source] = 0.0;
    double f_start = distances_[source] + heuristic_(source, target);
    pq.emplace(f_start, source);

    while (!pq.empty())
    {
        NodeF top = pq.top();
        pq.pop();

        double f_u = top.first;
        int u = top.second;

        if (visited_[u])
            continue;

        visited_[u] = true;

        if (u == target)
            break;

        for (uint32_t e = graph_->offset_begin(u); e < graph_->offset_end(u); ++e)
        {
            int v = static_cast<int>(graph_->edges()[e]);
            double w = graph_->weights()[e];

            if (regions_[u] != target_region && !flags_[e][target_region])
                continue;

            double new_dist = distances_[u] + w;
            if (new_dist < distances_[v])
            {
                distances_[v] = new_dist;
                predecessors_[v] = u;
                double f_v = new_dist + heuristic_(v, target);
                pq.emplace(f_v, v);
            }
        }
    }

    std::vector<int> path;
    if (distances_[target] == INF)
        return path;

    for (int v = target; v != -1; v = predecessors_[v])
        path.push_back(v);

    std::reverse(path.begin(), path.end());
    return path;
}