import osmnx as ox

G = ox.graph_from_place("Krakow, Poland", network_type="drive")


print(len(G.nodes))
print(len(G.edges))