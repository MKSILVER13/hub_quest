# Delivery Route Optimization

## Overview
This project implements an optimized delivery routing system that finds efficient paths for delivering packages to houses through their respective hubs while managing fuel costs and station refueling constraints. It is designed to solve logistics problems on a city graph with hubs, houses, fuel stations, and limited fuel capacity.

## Features
- Finds optimal delivery paths using a modified Floyd-Warshall algorithm
- Handles hub-based delivery requirements (each house must be delivered after visiting its corresponding hub)
- Manages fuel station constraints and limited fuel tank
- Optimizes total fuel cost while maximizing deliveries
- Supports brute-force and dynamic programming approaches
- Modular and extensible C++ codebase

## Problem Description
Given:
- A city as an undirected graph (nodes: hubs, houses, fuel stations, intersections)
- N hub-house pairs (each package starts at a hub and must be delivered to a unique house)
- M roads (edges with fuel costs)
- K fuel stations
- Vehicle with fuel tank capacity F

### Constraints
- Each house must be delivered only after visiting its corresponding hub
- The vehicle must never run out of fuel (can refuel at stations)
- The goal is to maximize the number of delivered houses and minimize total fuel cost

## Input Format
- First line: N T M K F
- Next N lines: hub node indices
- Next N lines: house node indices
- Next K lines: fuel station node indices
- Next M lines: u v c (edge from u to v with cost c)

## Output
- The optimal path (sequence of node indices)
- Number of delivered houses and their indices
- Total fuel cost
- Time taken for computation

## How It Works
1. **Input Parsing:** Reads the city graph, hubs, houses, and fuel stations.
2. **State Representation:** Uses a custom `State` struct to track path, visited hubs, delivered/undelivered houses, fuel costs, and more.
3. **Floyd-Warshall Initialization:** Precomputes shortest paths and delivery states between all pairs of nodes, considering fuel and delivery constraints.
4. **Path Exploration:** Uses a stack-based DFS to explore and merge paths, maximizing deliveries and minimizing cost.
5. **Result Output:** Prints the best path, delivered houses, and fuel cost.

## Code Structure
- `module_main_optimizer/opti.cc`: Main algorithm and entry point
- `module_opti_value_calculator/`: Utility for opti value calculation
- `project_general_inputs/`: Example input files

## How to Build & Run
1. **Build:**
   ```sh
   g++ -std=c++17 -O2 -o opti.exe module_main_optimizer/opti.cc
   ```
2. **Run:**
   ```sh
   ./opti.exe < project_general_inputs/input.txt
   ```
   Output will be written to `output.txt`.

## Example
Input (see `project_general_inputs/input.txt`):
```
N T M K F
... (hubs)
... (houses)
... (fuel stations)
... (edges)
```

Output:
```
<path length>
<path nodes>
Delivered Houses: X/Y
Houses delivered: ...
Total fuel cost: ...
Time taken: ... s
```

## Authors
- Mayukh Kundu

## License
MIT License
