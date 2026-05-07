import osmnx as ox
import networkx as nx
import matplotlib.pyplot as plt
import os
import pickle


def main():
    graph_file = "krakow_drive_graph.pkl"

    # ------------------------------------------------------------
    # Load or download graph
    # ------------------------------------------------------------
    if os.path.exists(graph_file):
        print("Loading graph from disk...")
        with open(graph_file, "rb") as f:
            G = pickle.load(f)
    else:
        print("Downloading graph from OSMnx...")
        G = ox.graph_from_place("Krakow, Poland", network_type="drive")

        with open(graph_file, "wb") as f:
            pickle.dump(G, f)

    print(f"Graph: {len(G.nodes)} nodes, {len(G.edges)} edges")

    # ------------------------------------------------------------
    # Convert to undirected for cleaner visualization
    # (optional but usually looks better)
    # ------------------------------------------------------------
    G_plot = G.to_undirected()

    # ------------------------------------------------------------
    # Get positions (lat/lon)
    # ------------------------------------------------------------
    pos = {node: (data["x"], data["y"]) for node, data in G.nodes(data=True)}

    # ------------------------------------------------------------
    # Draw
    # ------------------------------------------------------------
    plt.figure(figsize=(10, 10))

    nx.draw(
        G_plot,
        pos,
        node_size=5,
        node_color="black",
        edge_color="gray",
        width=0.3,
        alpha=0.6,
        with_labels=False
    )

    plt.title("Kraków Road Network (OSMnx)")
    plt.axis("off")
    plt.show()


if __name__ == "__main__":
    main()