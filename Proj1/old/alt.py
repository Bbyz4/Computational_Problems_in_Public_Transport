import heapq
import random
import networkx as nx
from typing import Tuple, List, Dict, Any, Optional

def select_landmarks(G: nx.DiGraph, k: int, method: str = "random") -> List[Any]:
    
    nodes = list(G.nodes)
    if k >= len(nodes):
        return nodes[:]
    
    if method == "random":
        landmarks = random.sample(nodes, k)
    else:
        raise ValueError(f"Unknown landmark selection method: {method}")

    return landmarks

def preprocess(G: nx.DiGraph, num_landmarks: int = 10, method: str = "random") -> Dict[str,Any]:
    landmarks = select_landmarks(G, num_landmarks, method)

    G_rev = G

    dist_to = {}
    dist_from = {}

    for L in landmarks:
        dist_to[L] = nx.single_source_dijkstra_path_length(G, L, weight="length")
        dist_from[L] = nx.single_source_dijkstra_path_length(G_rev, L, weight="length")

    preprocessed = {
        "G": G,
        "landmarks": landmarks,
        "dist_to": dist_to,
        "dist_from": dist_from
    }

    return preprocessed

def query(preprocessed: Dict[str, Any], source: Any, target: Any) -> Tuple[float, List[Any]]:
    G = preprocessed["G"]
    landmarks = preprocessed["landmarks"]
    dist_to = preprocessed["dist_to"]
    dist_from = preprocessed["dist_from"]

    def H(v: Any) -> float:
        h_val = 0
        for L in landmarks:
            d_Lv = dist_to[L].get(v, float("inf"))
            d_Lt = dist_to[L].get(target, float("inf"))
            d_vL = dist_from[L].get(v, float("inf"))
            d_tL = dist_from[L].get(target, float("inf"))

            if d_vL < float("inf") and d_tL < float("inf"):
                b = d_vL - d_tL
                if b > h_val:
                    h_val = b
            
            if d_Lt < float("inf") and d_Lv < float("inf"):
                b = d_Lt - d_Lv
                if b > h_val:
                    h_val = b

        return max(0.0, h_val)
    
    open_set = [(H(source), 0, source)] # 0 to tie-breaker
    heapq.heapify(open_set)
    g_score = {source: 0.0}
    came_from = {}
    counter = 1
    visited_order = []

    while open_set:
        f_val, _, current = heapq.heappop(open_set)

        if f_val - H(current) > g_score.get(current, float("inf")):
            continue

        visited_order.append(current)

        if current == target:
            path = [current]
            while current in came_from:
                current = came_from[current]
                path.append(current)

            path.reverse()
            return g_score[target], path, visited_order

        for neighbor in G.neighbors(current):
            edge_data = G.get_edge_data(current, neighbor)

            if G.is_multigraph():
                length = min(
                    attr.get("length", 1)
                    for attr in edge_data.values()
                )
            else:
                length = edge_data.get("length", 1)

            tentative_g = g_score[current] + length
            if tentative_g < g_score.get(neighbor, float("inf")):
                g_score[neighbor] = tentative_g
                came_from[neighbor] = current
                f_val = tentative_g + H(neighbor)
                heapq.heappush(open_set, (f_val, counter, neighbor))
                counter += 1

    return float("inf"), [], visited_order