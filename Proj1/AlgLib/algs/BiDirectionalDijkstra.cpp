#include<limits>
#include<queue>
#include<vector>
#include<algorithm>

#include "BiDirectionalDijkstra.h"

void BiDirectionalDijkstra::BuildReverseGraph_(const Graph& graph)
{
    int n = graph.num_nodes();

    std::vector<int> indeg(n, 0);
    
    for(int u=0; u<n; u++)
    {
        for(uint32_t e = graph.offset_begin(u); e < graph.offset_end(u); e++)
        {
            int v = graph.edges()[e];
            indeg[v]++;
        }
    }

    rev_offsets.resize(n+1, 0);
    for(int i=0; i<n; i++)
    {
        rev_offsets[i+1] = rev_offsets[i] + indeg[i];
    }

    int m = rev_offsets[n];
    rev_edges.resize(m);
    rev_weights.resize(m);

    std::vector<uint32_t> pos = rev_offsets;

    for(int u=0; u<n; u++)
    {
        for(uint32_t e = graph.offset_begin(u); e < graph.offset_end(u); e++)
        {
            int v = graph.edges()[e];
            double w = graph.weights()[e];
            int idx = pos[v]++;
            rev_edges[idx] = u;
            rev_weights[idx] = w;
        }
    }
}

void BiDirectionalDijkstra::Preprocess(const Graph& graph)
{
    graph_ = &graph;
    BuildReverseGraph_(graph);
}

std::vector<int> BiDirectionalDijkstra::Query(int source, int target)
{
    const int n = graph_->num_nodes();
    const double INF = std::numeric_limits<double>::infinity();

    distances_f.assign(n, INF);
    distances_b.assign(n, INF);
    predecessors_f.assign(n, -1);
    predecessors_b.assign(n, -1);
    visited_f.assign(n, false);
    visited_b.assign(n, false);

    distances_f[source] = 0.0;
    distances_b[target] = 0.0;

    using NodeDist = std::pair<double, int>;
    std::priority_queue<NodeDist, std::vector<NodeDist>, std::greater<>> pq_f, pq_b;

    pq_f.emplace(0.0, source);
    pq_b.emplace(0.0, target);

    double mu = INF;
    int best_meeting = -1;

    while(!pq_f.empty() && !pq_b.empty())
    {
        double top_f = pq_f.top().first;
        double top_b = pq_b.top().first;

        if(top_f + top_b >= mu)
        {
            break;
        }

        if(top_f <= top_b)
        {
            double dist_u = pq_f.top().first;
            int u = pq_f.top().second;
            pq_f.pop();

            if(visited_f[u])
            {
                continue;
            }

            visited_f[u] = true;

            for(uint32_t e = graph_->offset_begin(u); e < graph_->offset_end(u); e++)
            {
                int v = graph_->edges()[e];
                double w = graph_->weights()[e];
                double new_dist = distances_f[u] + w;

                if(new_dist < distances_f[v])
                {
                    distances_f[v] = new_dist;
                    predecessors_f[v] = u;
                    pq_f.emplace(new_dist, v);
                }

                if(distances_f[u] + w + distances_b[v] < mu)
                {
                    mu = distances_f[u] + w + distances_b[v];
                    best_meeting = v;
                }
            }
        }
        else
        {
            double dist_u = pq_b.top().first;
            int u = pq_b.top().second;
            pq_b.pop();

            if(visited_b[u])
            {
                continue;
            }

            visited_b[u] = true;

            for(uint32_t e = rev_offsets[u]; e < rev_offsets[u+1]; e++)
            {
                int v = rev_edges[e];
                double w = rev_weights[e];
                double new_dist = distances_b[u] + w;

                if(new_dist < distances_b[v])
                {
                    distances_b[v] = new_dist;
                    predecessors_b[v] = u;
                    pq_b.emplace(new_dist, v);
                }

                if(distances_b[u] + w + distances_f[v] < mu)
                {
                    mu = distances_b[u] + w + distances_f[v];
                    best_meeting = v;
                }
            }
        }
    }

    std::vector<int> path;
    if(best_meeting == -1 || mu == INF)
    {
        return path;
    }

    std::vector<int> forward_path;
    for(int v = best_meeting; v != -1; v = predecessors_f[v])
    {
        forward_path.push_back(v);
    }
    std::reverse(forward_path.begin(), forward_path.end());

    std::vector<int> backward_path;
    for(int v = predecessors_b[best_meeting]; v != -1; v = predecessors_b[v])
    {
        backward_path.push_back(v);
    }

    if(backward_path.empty() && best_meeting != target)
    {
        return path;
    }

    path = forward_path;
    path.insert(path.end(), backward_path.begin(), backward_path.end());

    if(path.back() != target)
    {
        path.push_back(target);
    }

    return path;
}