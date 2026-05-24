import osmnx as ox
import random
import networkx as nx
import matplotlib.pyplot as plt
from typing import Tuple, List, Dict, Any, Optional
import pickle
import os
import alt


def main():
        
    while True:

        # 1. Load or download graph (Krakow drive network)
        graph_file = "krakow_drive_graph.pkl"

        if os.path.exists(graph_file):
            print("Loading graph from disk...")
            with open(graph_file, "rb") as f:
                G = pickle.load(f)
        else:
            print("Downloading graph from OSMnx...")
            G = ox.graph_from_place("Krakow, Poland", network_type="drive")

            with open(graph_file, "wb") as f:
                pickle.dump(G, f)

        print(f"Graph loaded: {len(G.nodes)} nodes, {len(G.edges)} edges")

        G = G.to_undirected()

        # ------------------------------------------------------------
        # 2. Preprocess ALT
        # ------------------------------------------------------------
        print("Preprocessing ALT landmarks...")
        preprocessed = alt.preprocess(
            G,
            num_landmarks=16,
            method="random"
        )

        # ------------------------------------------------------------
        # 3. Select random source and target
        # ------------------------------------------------------------
        nodes = list(G.nodes)

        while True:
            source = random.choice(nodes)
            target = random.choice(nodes)

            if source != target:
                try:
                    # Ensure path exists
                    nx.shortest_path(G, source, target)
                    break
                except nx.NetworkXNoPath:
                    continue

        print(f"Source: {source}")
        print(f"Target: {target}")

        # ------------------------------------------------------------
        # 4. Run ALT query
        # ------------------------------------------------------------
        print("Running ALT query...")

        distance, path, visited_nodes = alt.query(
            preprocessed,
            source,
            target
        )

        print(f"Shortest distance: {distance}")
        print(f"Path length (nodes): {len(path)}")
        print(f"Visited vertices: {len(visited_nodes)}")

        if distance < float("inf"):

            # ------------------------------------------------------------
            # 5. Prepare node colors
            # ------------------------------------------------------------
            visited_set = set(visited_nodes)
            landmark_set = set(preprocessed["landmarks"])

            node_colors = []

            for node in G.nodes:

                if node == source:
                    node_colors.append("green")

                elif node == target:
                    node_colors.append("blue")

                elif node in landmark_set:
                    node_colors.append("yellow")

                elif node in visited_set:
                    node_colors.append("red")

                else:
                    node_colors.append("black")

            # ------------------------------------------------------------
            # 6. Draw graph
            # ------------------------------------------------------------
            print("Drawing graph...")

            fig, ax = ox.plot_graph(
                G,
                node_color=node_colors,
                node_size=6,
                edge_color="lightgray",
                edge_linewidth=0.3,
                bgcolor="white",
                show=False,
                close=False
            )

            # ------------------------------------------------------------
            # 7. Draw shortest path
            # ------------------------------------------------------------
            route_edges = list(zip(path[:-1], path[1:]))

            ox.plot_graph_route(
                G,
                path,
                route_color="yellow",
                route_linewidth=3,
                orig_dest_size=0,
                ax=ax,
                show=False,
                close=False
            )

            plt.title("ALT Search Visualization")
            plt.show()


if __name__ == "__main__":
    main()