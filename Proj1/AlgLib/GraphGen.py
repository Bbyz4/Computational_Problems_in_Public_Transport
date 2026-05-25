#!/usr/bin/env python3
"""
Generate a geometric random graph with Poisson‑disk node placement.
Output format: node coordinates followed by directed edges (from, to, weight).
Optionally generate query pairs for algorithm testing.
"""

import numpy as np
import random
from scipy.spatial import KDTree
import sys

def bridson_poisson_disk(width, height, min_dist, k=30, seed=None):
    """
    Bridson's algorithm for Poisson disk sampling.
    Returns a list of (x, y) points.
    """
    if seed is not None:
        random.seed(seed)
        np.random.seed(seed)

    # Cell size = min_dist / sqrt(2)
    cell_size = min_dist / np.sqrt(2)
    nx = int(np.ceil(width / cell_size)) + 1
    ny = int(np.ceil(height / cell_size)) + 1
    grid = [[None for _ in range(ny)] for _ in range(nx)]

    points = []
    active = []

    # Initial point
    x0 = random.uniform(0, width)
    y0 = random.uniform(0, height)
    points.append((x0, y0))
    active.append((x0, y0))
    ix = int(x0 / cell_size)
    iy = int(y0 / cell_size)
    grid[ix][iy] = (x0, y0)

    while active:
        idx = random.randint(0, len(active) - 1)
        px, py = active[idx]
        found = False
        for _ in range(k):
            # Random angle and radius in [min_dist, 2*min_dist]
            angle = random.uniform(0, 2 * np.pi)
            r = random.uniform(min_dist, 2 * min_dist)
            x = px + r * np.cos(angle)
            y = py + r * np.sin(angle)

            if x < 0 or x > width or y < 0 or y > height:
                continue

            # Check neighbouring cells
            cx = int(x / cell_size)
            cy = int(y / cell_size)
            cell_min_x = max(0, cx - 2)
            cell_max_x = min(nx, cx + 3)
            cell_min_y = max(0, cy - 2)
            cell_max_y = min(ny, cy + 3)

            conflict = False
            for i in range(cell_min_x, cell_max_x):
                for j in range(cell_min_y, cell_max_y):
                    if grid[i][j] is not None:
                        dx = x - grid[i][j][0]
                        dy = y - grid[i][j][1]
                        if dx * dx + dy * dy < min_dist * min_dist:
                            conflict = True
                            break
                if conflict:
                    break

            if not conflict:
                points.append((x, y))
                active.append((x, y))
                grid[cx][cy] = (x, y)
                found = True
                break

        if not found:
            active.pop(idx)

    return points


def generate_poisson_points(num_nodes, width, height, seed=None):
    """
    Generate exactly num_nodes points using Poisson disk sampling.
    Adjusts min_dist if needed to match the requested count.
    """
    area = width * height
    # Start with an estimate based on hexagonal packing density (approx 0.9)
    min_dist = np.sqrt(area / (num_nodes * 0.9))
    points = bridson_poisson_disk(width, height, min_dist, seed=seed)

    # If we have too many, truncate; if too few, decrease radius and retry
    if len(points) < num_nodes:
        # Try a few times with gradually smaller radius
        for factor in [0.9, 0.8, 0.7]:
            points = bridson_poisson_disk(width, height, min_dist * factor, seed=seed)
            if len(points) >= num_nodes:
                break
    if len(points) > num_nodes:
        # Randomly subsample
        indices = np.random.choice(len(points), num_nodes, replace=False)
        points = [points[i] for i in indices]
    elif len(points) < num_nodes:
        # Still too few – pad with random points that respect min distance
        # (this should rarely happen)
        kdt = KDTree(points) if points else None
        while len(points) < num_nodes:
            x = np.random.uniform(0, width)
            y = np.random.uniform(0, height)
            if kdt is None or kdt.query([x, y])[0] >= min_dist * 0.9:
                points.append((x, y))
                if kdt is None:
                    kdt = KDTree(points)
                else:
                    kdt = KDTree(points)   # rebuild – inefficient but rare
    return points


def generate_graph(num_nodes, avg_degree, width=1.0, height=1.0,
                   weight_noise=0.1, edge_drop_prob=0.0, directed=True, seed=42,
                   num_queries=0, query_seed=None):
    """
    Generate node coordinates and directed edges.

    Parameters:
        num_nodes      : number of nodes
        avg_degree     : target average out‑degree (for directed graph)
        width, height  : bounding box for node placement
        weight_noise   : relative noise added to edge weight (0 = deterministic distance)
        edge_drop_prob : probability to omit an edge (0 = all geometric edges kept)
        directed       : if True, both directions are added; if False, only one direction (undirected)
        seed           : random seed for reproducibility
        num_queries    : number of (source, target) query pairs to generate
        query_seed     : optional separate seed for query generation (if None, uses seed+1)

    Returns:
        lats : list of x‑coordinates (treated as longitude)
        lons : list of y‑coordinates (treated as latitude)
        edges: list of (from, to, weight) tuples
        queries: list of (source, target) tuples (empty if num_queries==0)
    """
    random.seed(seed)
    np.random.seed(seed)

    # 1. Place nodes with Poisson disk sampling
    points = generate_poisson_points(num_nodes, width, height, seed=seed)
    points = np.array(points)  # shape (n, 2)

    # 2. Compute distance threshold R to achieve expected avg_degree
    area = width * height
    # Expected number of neighbours inside radius R for uniform points:
    #   expected_neighbours = (n-1) * (π R^2) / area
    # Solve for R given expected_neighbours = avg_degree
    R = np.sqrt(avg_degree * area / (num_nodes * np.pi))

    # 3. Build symmetric graph (both directions for each undirected edge)
    symmetric_edges = set()
    kdt = KDTree(points)
    for i in range(num_nodes):
        indices = kdt.query_ball_point(points[i], R)
        for j in indices:
            if i < j:
                if random.random() < edge_drop_prob:
                    continue
                dist = np.linalg.norm(points[i] - points[j])
                noise = 1.0 + weight_noise * (2.0 * random.random() - 1.0)
                weight = dist * noise
                symmetric_edges.add((i, j, weight))

    edges = []
    for i, j, w in symmetric_edges:
        edges.append((i, j, w))
        edges.append((j, i, w))

    # 4. Generate query pairs (if requested)
    queries = []
    if num_queries > 0:
        # Use separate seed for reproducibility of queries independent of graph
        if query_seed is None:
            query_seed = seed + 1
        random.seed(query_seed)
        np.random.seed(query_seed)

        # Generate unique pairs (source != target)
        # For large num_queries, we may allow duplicates, but we'll try to keep unique
        max_pairs = num_nodes * (num_nodes - 1)
        if num_queries > max_pairs:
            print(f"Warning: requested {num_queries} queries but only {max_pairs} possible unique pairs. "
                  f"Generating {max_pairs} unique pairs and then repeating.", file=sys.stderr)
            # We'll generate with replacement after exhausting uniqueness
            unique_pairs = set()
            while len(unique_pairs) < min(num_queries, max_pairs):
                s = random.randint(0, num_nodes - 1)
                t = random.randint(0, num_nodes - 1)
                if s != t:
                    unique_pairs.add((s, t))
            queries = list(unique_pairs)
            # If more queries needed, repeat existing ones (with possible new randomness)
            while len(queries) < num_queries:
                queries.append(random.choice(queries))
        else:
            # Generate exactly num_queries unique pairs
            pairs_set = set()
            while len(pairs_set) < num_queries:
                s = random.randint(0, num_nodes - 1)
                t = random.randint(0, num_nodes - 1)
                if s != t:
                    pairs_set.add((s, t))
            queries = list(pairs_set)

    # Convert points to lat/lon (here we simply treat x as longitude, y as latitude)
    lats = points[:, 1].tolist()   # y coordinate -> latitude
    lons = points[:, 0].tolist()   # x coordinate -> longitude
    return lats, lons, edges, queries


def write_graph_to_file(filename, lats, lons, edges):
    """
    Write graph data to a text file.
    Format:
        NODE id lat lon
        EDGE from to weight
    """
    with open(filename, 'w') as f:
        for i, (lat, lon) in enumerate(zip(lats, lons)):
            f.write(f"NODE {i} {lat} {lon}\n")
        for frm, to, w in edges:
            f.write(f"EDGE {frm} {to} {w}\n")


def write_queries_to_file(filename, queries):
    """
    Write query pairs to a text file.
    Format:
        QUERY from to
    """
    with open(filename, 'w') as f:
        for s, t in queries:
            f.write(f"QUERY {s} {t}\n")


if __name__ == "__main__":
    # Example: generate 1000 nodes, average out-degree = 10,
    # with 5% weight noise, 5% chance to drop an edge, and 100 query pairs.
    num_nodes = 1000
    avg_degree = 100
    num_queries = 10000

    lats, lons, edges, queries = generate_graph(num_nodes, avg_degree,
                                                 width=100.0, height=100.0,
                                                 weight_noise=0.05, edge_drop_prob=0.05,
                                                 seed=1234,
                                                 num_queries=num_queries,
                                                 query_seed=5678)

    print(f"Generated {len(lats)} nodes and {len(edges)} directed edges.")
    write_graph_to_file("graph_data.txt", lats, lons, edges)
    print("Graph saved to graph_data.txt")

    if queries:
        write_queries_to_file("queries.txt", queries)
        print(f"Generated {len(queries)} query pairs. Saved to queries.txt")