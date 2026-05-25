#include<limits>
#include<queue>
#include<vector>
#include<algorithm>

#include "Algorithm.cpp"

class Dijkstra : public Algorithm
{
    private:
        Graph& graph_; 
        std::vector<double> distances_;
        std::vector<int> predecessors_;
        std::vector<bool> visited_;

    public:
        void Preprocess(const Graph& graph) override
        {
            graph_ = graph;
        }

        std::vector<int> Query(int source, int target) override
        {
            const int n = graph_.num_nodes();
            const double INF = std::numeric_limits<double>::infinity();

            distances_.assign(n, INF);
            predecessors_.assign(n, -1);
            visited_.assign(n, false);

            using NodeDist = std::pair<double, int>;
            std::priority_queue<NodeDist, std::vector<NodeDist>, std::greater<>> pq;

            distances_[source] = 0.0;
            pq.emplace(0.0, source);

            while(!pq.empty())
            {
                NodeDist pqtop = pq.top();
                pq.pop();

                double dist_u = pqtop.first;
                int u = pqtop.second;

                if(visited_[u])
                {
                    continue;
                }

                visited_[u] = true;

                if(u == target)
                {
                    break;
                }

                for(uint32_t e = graph_.offset_begin(u); e < graph_.offset_end(u); e++)
                {
                    int v = graph_.edges()[e];
                    double w = graph_.weights()[e];
                    double new_dist = dist_u + w;

                    if(new_dist < distances_[v])
                    {
                        distances_[v] = new_dist;
                        predecessors_[v] = u;
                        pq.emplace(new_dist, v);
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
};