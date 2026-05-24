#include "Graph.h"
#include <cassert>

void Graph::add_node(double lat, double lon)
{
    assert(!built_ && "Nodes cannot be added after building");
    lats_.push_back(lat);
    lons_.push_back(lon);
    temp_adj_.emplace_back();
    temp_rev_adj_.emplace_back();
}

void Graph::add_edge(int from, int to, double weight)
{
    assert(!built_ && "Edges cannot be added after building");
    assert(from >= 0 && from < num_nodes());
    assert(to >= 0 && to < num_nodes());
    temp_adj_[from].emplace_back(to, weight);
    temp_rev_adj_[to].emplace_back(from, weight);
}

void Graph::build()
{
    assert(!built_ && "Graph was already built");

    const int n = num_nodes();
    const int m = [&]()
    {
        int cnt = 0;
        for(const auto& lst : temp_adj_) cnt += lst.size();
        return cnt;
    }();

    offsets_.resize(n+1, 0);
    edges_.resize(m);
    weights_.resize(m);

    for(int v=0; v<n; v++)
    {
        offsets_[v+1] = offsets_[v] + static_cast<uint32_t>(temp_adj_[v].size());
    }

    std::vector<uint32_t> current_pos = offsets_;
    for(int v=0; v<n; v++)
    {
        uint32_t pos = current_pos[v];
        for (const auto& edge : temp_adj_[v])
        {
            edges_[pos] = static_cast<uint32_t>(edge.first);
            weights_[pos] = edge.second;
            pos++;
        }
    }

    rev_offsets_.resize(n+1, 0);
    rev_edges_.resize(m);
    rev_weights_.resize(m);

    for(int v=0; v<n; v++)
    {
        rev_offsets_[v+1] = rev_offsets_[v] + static_cast<uint32_t>(temp_rev_adj_[v].size());
    }

    std::vector<uint32_t> rev_current_pos = rev_offsets_;
    for(int v=0; v<n; v++)
    {
        uint32_t pos = rev_current_pos[v];
        for (const auto& edge : temp_rev_adj_[v])
        {
            rev_edges_[pos] = static_cast<uint32_t>(edge.first);
            rev_weights_[pos] = edge.second;
            pos++;
        }
    }

    temp_adj_.clear();
    temp_adj_.shrink_to_fit();
    temp_rev_adj_.clear();
    temp_rev_adj_.shrink_to_fit();

    built_ = true;
}