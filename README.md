# Computational_Problems_in_Public_Transport
A repository for all projects made as a part of the Computational Problems in Public Transport course @ Jagiellonian University

Both projects use AI as co-author or as a tool in various places

## Project 1 Description
A library containing various efficient algorithm used in public transportation problems along with an example network generator and benchmark tests

### Main goals:
- provide a usable resource with optimised algorithms
- perform efficiency tests and present results in a readable way
- implement a generator that could create realistic-looking networks

### Implementation log:
before 24.05 - working on an old project 1 idea - comparasion between different landmark selection heuristics in A* and their performance

24.05 - definied the new project goals, added an efficient class for maintaining a directed graph

25.05 - added a graph generator, a query generator, an algorithm interface, the first algorithm (Dijkstra), and a simple main cpp file used for performance testing

26.05 - added an implementation of the A* algorithm, added a simple heuristic function, added a result verification method which currently takes Dijkstra as the ground truth for the shortest path length

27.05 - added algorithms: BiDirectional Dijkstra, BiDirectional A* (currently a little malfunctioning), ALT. Added a script for automatic compilation and test launching

02.06 - 04.06 - added various algorithms with arc-flags preprocessing features: ARC-Dijkstra, ARC-Astar and ARC-ALT. Created a bunch of example graphs and query sets, started performing the benchmark tests

05.06 - finished algorithm testing, wrote a report with project description, implementation insights and results

## Project 2 Description
A project focused on visualizing and experimenting with the RAPTOR (Round-based Public Transit Routing) algorithm on public transport network. Project is built using C# along with Unity game engine.

### Main goals:
- provide a visualizer for the RAPTOR shortest-path finding algorithm
- add support for real public transport datasets (GTFS or similar), which could be imported and visualized
- route finding on actual city networks (eg. Kraków, Poland)
- experimentation with additional routing constraints, including adding walking transfers, preference to specific modes, accessibility issues for people with disabilities etc.

### Implementation log:
12.05 - definied the project goals and requirements, created an initial unity project

13.05 - implemented a basic version of the raptor algorithm, downloaded real transport network data from Kraków, performed some exploratory tests and found a few flaws to fix

18.05 - worked on automatically importing current route updates in Kraków

29.05 - UI improvements, raptor generalization

15.06 - UI/UX upgrade, added screenshots

<img width="741" height="499" alt="Zrzut ekranu 2026-06-15 145933" src="https://github.com/user-attachments/assets/c0b66325-3d6a-491e-bf94-10dfe0931dbc" />
