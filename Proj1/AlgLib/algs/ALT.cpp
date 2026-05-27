#include <limits>
#include <queue>
#include <algorithm>
#include <cstdlib>
#include <ctime>

#include "ALT.h"

ALT::ALT(int k) : num_landmarks_(k)
{
    if(num_landmarks_ < 1) 
    {
        num_landmarks_ = 1;
    }
}

void ALT::Preprocess(const Graph& graph)
{
    graph_ = &graph;

    const int n = graph.num_nodes();
    const double INF = std::numeric_limits<double>::infinity();

    std::srand(static_cast<unsigned>(std::time(0)));
    std::vector<int> indices(n);

    for(int i = 0; i < n; i++)
    {
        indices[i] = i;
    }

    int k = std::min(num_landmarks_, n);
    for(int i = 0; i < k; i++)
    {
        int r = i + (std::rand() % (n - i));
        std::swap(indices[i], indices[r]);
    }
    landmarks_.assign(indices.begin(), indices.begin() + k);

    rev_adj_.assign(n, std::vector<std::pair<int, double>>());
    for(int u = 0; u < n; u++)
    {
        for(uint32_t e = graph_->offset_begin(u); e < graph_->offset_end(u); e++)
        {
            int v = graph_->edges()[e];
            double w = graph_->weights()[e];
            rev_adj_[v].emplace_back(u, w);
        }
    }

    dist_from_landmark_.resize(k, std::vector<double>(n, INF));
    dist_to_landmark_.resize(k, std::vector<double>(n, INF));

    for(int i = 0; i < k; i++)
    {
        int lm = landmarks_[i];
        DijkstraFromLandmark(lm, dist_from_landmark_[i]);
        DijkstraToLandmark(lm, dist_to_landmark_[i]);
    }
}

void ALT::DijkstraFromLandmark(int landmark, std::vector<double>& dist)
{
    const int n = graph_->num_nodes();
    const double INF = std::numeric_limits<double>::infinity();
    std::fill(dist.begin(), dist.end(), INF);
    std::vector<bool> visited(n, false);

    using NodeDist = std::pair<double, int>;
    std::priority_queue<NodeDist, std::vector<NodeDist>, std::greater<NodeDist>> pq;

    dist[landmark] = 0.0;
    pq.emplace(0.0, landmark);

    while(!pq.empty())
    {
        double d_u = pq.top().first;
        int u = pq.top().second;
        pq.pop();

        if(visited[u])
        {
            continue;
        }
        visited[u] = true;

        for(uint32_t e = graph_->offset_begin(u); e < graph_->offset_end(u); e++)
        {
            int v = graph_->edges()[e];
            double w = graph_->weights()[e];
            double nd = d_u + w;
            if(nd < dist[v])
            {
                dist[v] = nd;
                pq.emplace(nd, v);
            }
        }
    }
}

void ALT::DijkstraToLandmark(int landmark, std::vector<double>& dist)
{
    const int n = static_cast<int>(rev_adj_.size());
    const double INF = std::numeric_limits<double>::infinity();
    std::fill(dist.begin(), dist.end(), INF);
    std::vector<bool> visited(n, false);

    using NodeDist = std::pair<double, int>;
    std::priority_queue<NodeDist, std::vector<NodeDist>, std::greater<NodeDist>> pq;

    dist[landmark] = 0.0;
    pq.emplace(0.0, landmark);

    while(!pq.empty())
    {
        double d_u = pq.top().first;
        int u = pq.top().second;
        pq.pop();

        if(visited[u])
        {
            continue;
        }
        visited[u] = true;

        for(size_t i = 0; i < rev_adj_[u].size(); i++)
        {
            int prev = rev_adj_[u][i].first;
            double w = rev_adj_[u][i].second;
            double nd = d_u + w;
            if(nd < dist[prev])
            {
                dist[prev] = nd;
                pq.emplace(nd, prev);
            }
        }
    }
}

std::vector<int> ALT::Query(int source, int target)
{
    const int n = graph_->num_nodes();
    const double INF = std::numeric_limits<double>::infinity();

    distances_.assign(n, INF);
    predecessors_.assign(n, -1);
    visited_.assign(n, false);

    using NodeF = std::pair<double, int>;
    std::priority_queue<NodeF, std::vector<NodeF>, std::greater<NodeF>> pq;

    distances_[source] = 0.0;

    double h_s = 0.0;
    for(size_t l = 0; l < landmarks_.size(); l++)
    {
        double d_xt = dist_from_landmark_[l][target];
        double d_xv = dist_from_landmark_[l][source];
        if(d_xt < INF && d_xv < INF)
        {
            h_s = std::max(h_s, d_xt - d_xv);
        }
        double d_vx = dist_to_landmark_[l][source];
        double d_tx = dist_to_landmark_[l][target];
        if(d_vx < INF && d_tx < INF)
        {
            h_s = std::max(h_s, d_vx - d_tx);
        }
    }
    pq.emplace(distances_[source] + h_s, source);

    while(!pq.empty())
    {
        double f_u = pq.top().first;
        int u = pq.top().second;
        pq.pop();

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

                double h_v = 0.0;
                for(size_t l = 0; l < landmarks_.size(); l++)
                {
                    double d_xt = dist_from_landmark_[l][target];
                    double d_xv = dist_from_landmark_[l][v];
                    if(d_xt < INF && d_xv < INF)
                    {
                        h_v = std::max(h_v, d_xt - d_xv);
                    }
                    double d_vx = dist_to_landmark_[l][v];
                    double d_tx = dist_to_landmark_[l][target];
                    if(d_vx < INF && d_tx < INF)
                    {
                        h_v = std::max(h_v, d_vx - d_tx);
                    }
                }
                double f_v = new_dist + h_v;
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