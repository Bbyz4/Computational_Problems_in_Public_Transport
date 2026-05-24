#include "Graph.h"
#include <iostream>
#include <iomanip>

//AI generated main

int main() {
    Graph g;

    // Dodajemy węzły (miasta) z przykładowymi współrzędnymi
    g.add_node(50.061, 19.937);   // 0
    g.add_node(50.072, 19.933);   // 1
    g.add_node(50.067, 19.945);   // 2
    g.add_node(50.058, 19.930);   // 3

    // Dodajemy krawędzie (drogi)
    g.add_edge(0, 1, 0.5);
    g.add_edge(0, 2, 0.8);
    g.add_edge(1, 2, 0.3);
    g.add_edge(2, 3, 0.6);
    g.add_edge(1, 3, 0.9);

    // Kończymy budowę – powstają struktury CSR
    g.build();

    std::cout << "Liczba węzłów: " << g.num_nodes() << "\n";
    std::cout << "Liczba krawędzi: " << g.num_edges() << "\n\n";

    // Wyświetlenie sąsiadów (forward) dla każdego węzła
    for (int v = 0; v < g.num_nodes(); ++v) {
        std::cout << "Węzeł " << v << " (lat=" << g.lat(v) << ", lon=" << g.lon(v) << "):\n";
        uint32_t beg = g.offset_begin(v);
        uint32_t end = g.offset_end(v);
        for (uint32_t i = beg; i < end; ++i) {
            int to = g.edges()[i];
            double w = g.weights()[i];
            std::cout << "  -> " << to << " (waga=" << std::fixed << std::setprecision(2) << w << ")\n";
        }
    }

    // Wyświetlenie sąsiadów wchodzących (reverse)
    std::cout << "\nKrawędzie wchodzące (reverse):\n";
    for (int v = 0; v < g.num_nodes(); ++v) {
        std::cout << "Do węzła " << v << ":\n";
        uint32_t beg = g.rev_offset_begin(v);
        uint32_t end = g.rev_offset_end(v);
        for (uint32_t i = beg; i < end; ++i) {
            int from = g.rev_edges()[i];
            double w = g.rev_weights()[i];
            std::cout << "  <- " << from << " (waga=" << w << ")\n";
        }
    }

    return 0;
}