# Computational_Problems_in_Public_Transport
A repository for all projects made as a part of the Computational Problems in Public Transport course @ Jagiellonian University

## Project 1 Description
A library containing various efficient algorithm used in public transportation problems along with an example network generator and benchmark tests

### Main goals:
- provide a usable resource with optimised algorithms
- perform efficiency tests and present results in a readable way
- implement a generator that could create realistic-looking networks

### Implementation log:
before 24.05 - working on an old project 1 idea - comparasion between different landmark selection heuristics in A* and their performance

24.05 - definied the new project goals


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
