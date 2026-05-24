#pragma once

#include <cstdint>
#include <vector>
#include <utility>

/**
 * @brief CSR directed graph
 *
 * add_node() and add_edge() are for construction
 * 
 * build() freezes the structure and, well, builds it
*/
class Graph
{
    public:
        Graph() = default;

        void add_node(double lat = 0.0, double lon = 0.0);
        void add_edge(int from, int to, double weight);
        void build();

        int num_nodes() const { return static_cast<int>(lats_.size());}
        int num_edges() const { return static_cast<int>(edges_.size());}
        bool is_built() const { return built_; }

        uint32_t offset_begin(int node) const { return offsets_[node]; }
        uint32_t offset_end(int node) const {return offsets_[node+1]; }
        const std::vector<uint32_t>& edges() const {return edges_; }
        const std::vector<double>& weights() const {return weights_; }
        const std::vector<uint32_t>& offsets() const {return offsets_; }

        uint32_t rev_offset_begin(int node) const {return rev_offsets_[node]; }
        uint32_t rev_offset_end(int node) const {return rev_offsets_[node + 1]; }
        const std::vector<uint32_t>& rev_edges() const {return rev_edges_; }
        const std::vector<double>& rev_weights() const {return rev_weights_; }
        const std::vector<uint32_t>& rev_offsets() const {return rev_offsets_; }

        double lat(int node) const { return lats_[node]; }
        double lon(int node) const { return lons_[node]; }

    private:

        std::vector<uint32_t> offsets_;
        std::vector<uint32_t> edges_;
        std::vector<double> weights_;

        std::vector<uint32_t> rev_offsets_;
        std::vector<uint32_t> rev_edges_;
        std::vector<double> rev_weights_;

        std::vector<double> lats_;
        std::vector<double> lons_;

        std::vector<std::vector<std::pair<int,double>>> temp_adj_;
        std::vector<std::vector<std::pair<int,double>>> temp_rev_adj_;

        bool built_ = false;
};