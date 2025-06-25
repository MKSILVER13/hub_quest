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

## Customizing I/O Paths

If you need to change which files the code reads or writes, update the `freopen` calls in the source:

-- In **module_main_optimizer/opti.cc** (lines 625–631):
```cpp
if (freopen("../project_general_inputs/input_2.txt", "r", stdin) == NULL) { /* line 625 */
    perror("Error opening input file");
    return 1;
}
if (freopen("module_main_optimizer/output.txt", "w", stdout) == NULL) { /* line 631 */
    perror("Error opening output file");
    return 1;
}
```
-- In **module_opti_value_calculator/calculate_opti_value.cc** (line 191):
```cpp
if (freopen("module_opti_value_calculator/analysis_output.txt", "w", stdout) == NULL) { /* line 191 */
    perror("Error redirecting stdout");
    return 1;
}
readGraph("module_opti_value_calculator/graph_input.txt");
// ...
std::ifstream paths_file("module_opti_value_calculator/paths_input.txt");
```
Adjust the file paths or names as needed to point to your desired input and output files.

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

## Performance Summary

Combined results for `input_1.txt` (25h/25 hubs), `input_2.txt` (50h/50 hubs), and `input_3.txt` (5h/5 hubs):

| Input   | Variant                            | Delivered       | Fuel Cost | Time (s) | Δ Fuel vs V1 | Δ Time vs V1 | Δ Fuel vs V2 |
|---------|------------------------------------|-----------------|-----------|----------|--------------|--------------|--------------|
| **In0** | 1) Multi-factor `optiValue`        | 10 / 10 (100%)  |   9,177  | 0        | —            | —            | —            |
| **In0** | 2) Pure fuel minimization          | 10 / 10 (100%)  |   4,851  | 0        | –47.1%       | —            | —            |
| **In0** | 3) + “large-fuel” surcharge        | 10 / 10 (100%)  |   5,947  | 0        | –35.2%       | —            | +22.6%       |
| **In0** | 4) Max segment cost                | 10 / 10 (100%)  |  5,455    | 0        | –40.6%       | —            | +12.4%       |
| **In1** | 1) Multi-factor `optiValue`        | 24 / 25 (96%)   | 78,605    | 9        | —            | —            | —            |
| **In1** | 2) Pure fuel minimization          | 25 / 25 (100%)  | 22,872    | 3        | –70.9%       | –66.7%       | —            |
| **In1** | 3) + “large-fuel” surcharge        | 25 / 25 (100%)  | 22,874    | 3        | –70.8%       | –66.7%       | +0.009%      |
| **In1** | 4) Max segment cost                | 25 / 25 (100%)  | 23,355    | 3        | –70.3%       | –66.7%       | +2.1%        |
| **In2** | 1) Multi-factor `optiValue`        | 50 / 50 (100%)  | 416,328   | 544      | —            | —            | —            |
| **In2** | 2) Pure fuel minimization          | 50 / 50 (100%)  |  90,647   | 152      | –78.2%       | –72.1%       | —            |
| **In2** | 3) Fuel Cost + Light Penalty (F/x) | 50 / 50 (100%)  |  89,485   | 151      | –78.5%       | –72.2%       | –1.3%        |
| **In2** | 4) Max segment cost                | 50 / 50 (100%)  | 88,333    | 108      | –78.8%       | –80.2%       | –2.6%        |
| **In** | 1) Multi-factor `optiValue`        |  5 /  5 (100%) |    38   | 0        | —            | —            | —            |
| **In** | 2) Pure fuel minimization          |  5 /  5 (100%) |    35   | 0        | –7.9%    | —        | —         |
| **In** | 3) + “large-fuel” surcharge        |  5 /  5 (100%) |    34   | 0        | –10.5%   | —        | –2.9%     |
| **In3** | 4) Max segment cost                |  5 /  5 (100%)  |    33     | 0        | –13.2%       | —        | –5.7%        |

## Directory Structure

As organized in `DIRECTORY_GUIDE.md`:
- `module_main_optimizer/` – main algorithm source (`opti.cc`) and its output
- `module_opti_value_calculator/` – heuristic calculator (`calculate_opti_value.cc`) and related I/O
- `project_general_inputs/` – shared input files (`input_0.txt`, etc.)
- `executables_bin/` – compiled binaries (`opti.exe`, `calculate_opti_value.exe`)

## Setup & Run

```powershell
# 1. Clone repository
git clone https://github.com/MKSILVER13/hub_quest.git
cd hub_quest

# 2. Build executables
g++ -std=c++17 -O2 module_main_optimizer/opti.cc -o executables_bin/opti.exe
g++ -std=c++17 -O2 module_opti_value_calculator/calculate_opti_value.cc -o executables_bin/calculate_opti_value.exe

# 3. Run main optimizer
executables_bin\opti.exe < project_general_inputs\input.txt
# Output written to module_main_optimizer\output.txt

# 4. Run OptiValue calculator
executables_bin\calculate_opti_value.exe
# Analysis output in module_opti_value_calculator\analysis_output.txt
```

## Authors
- Mayukh Kundu

## License
MIT License
