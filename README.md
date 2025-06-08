# Delivery Route Optimization [WORK IN PROGRESS]

## Overview
This project implements an optimized delivery routing system that finds efficient paths for delivering packages to houses through their respective hubs while managing fuel costs and station refueling constraints.

### Key Features
- Finds optimal delivery paths using Floyd-Warshall algorithm
- Handles hub-based delivery requirements
- Manages fuel station constraints
- Optimizes total fuel cost
- Merges additional delivery paths for undelivered houses

## Problem Description
The system solves a complex routing problem with the following constraints:
- Each house must be delivered through its corresponding hub
- Vehicles have limited fuel capacity (F)
- Fuel stations are available for refueling
- The goal is to minimize total fuel cost while maximizing deliveries

### Input Parameters
- N: Number of hub-house pairs
- T: Total number of nodes
- M: Number of edges
- K: Number of fuel stations
- F: Fuel capacity

### Node Types
1. Fuel Stations (Type 1)
2. Hubs (Type 2)
3. Houses (Type 3)

## Implementation Details

### Core Components

1. **Path Finding**
   - Uses modified Floyd-Warshall algorithm
   - Considers fuel constraints between nodes
   - Maintains state information for visited hubs and delivered houses

2. **State Management**
   - Tracks delivered and undelivered houses
   - Monitors fuel costs and path optimization values
   - Maintains visited hub information

3. **Path Merging**
   - Handles overlapping path segments
   - Removes consecutive duplicates
   - Preserves path validity while combining routes

### Key Algorithms

1. **Initial Path Finding**
   ```cpp
   State findOptimalPath(int x, int y)
   ```
   - Finds initial delivery path using optimization parameters
   - Uses backtracking with bounded exploration
   - Prioritizes maximizing house deliveries

2. **Path Pair Finding**
   ```cpp
   pair<vector<PairPath>, vector<PairPath>> findOptimalPairPath(int hub, int house)
   ```
   - Finds optimal paths between hubs and houses
   - Considers both direct and hub-based routes
   - Returns best paths for each starting fuel station

3. **Path Merging**
   ```cpp
   vector<int> merge_with_overlap(const vector<int>& main_path, const vector<int>& sub_path, size_t insert_pos)
   ```
   - Merges additional delivery paths into main path
   - Handles overlapping segments efficiently
   - Maintains path validity

## Optimization Value (opti_value) Details
The system uses a custom optimization value to evaluate and compare paths. The opti_value is calculated as follows:

### Calculation Formula:
```cpp
if (delivered > x) {
    opti_value = fuel_cost * (visited_hubs/delivered_houses + 1)
} else {
    opti_value = fuel_cost / visited_hubs
}
```

Where:
- delivered: Number of houses delivered
- x: Threshold parameter (default: 10)
- fuel_cost: Total fuel cost of the path
- visited_hubs: Number of hubs visited
- delivered_houses: Number of houses delivered

### Key Properties:
1. Lower opti_value indicates a better path
2. Balances between:
   - Maximizing house deliveries
   - Minimizing fuel costs
   - Optimizing hub visits

### Current Limitations:
- May not find globally optimal solution
- Sensitive to parameter x
- Can be trapped in local optima

## Usage

### Compilation
```bash
g++ opti.cc -o opti.exe
```

### Input Format
The input file should contain:
1. First line: N T M K F
2. Next N lines: Hub nodes
3. Next N lines: House nodes
4. Next K lines: Fuel station nodes
5. Next M lines: Edges (u v c) where c is the cost

### Output
The program outputs:
1. Main delivery path
2. Houses delivered in main path
3. Undelivered houses
4. Merged path including additional deliveries
5. Total fuel cost

## Optimization Strategy
1. Find initial path maximizing deliveries
2. Identify undelivered houses
3. Find optimal additional paths at fuel stations
4. Merge additional paths efficiently
5. Remove redundant segments
6. Calculate final optimized cost

## Performance Considerations
- Time Complexity: O(T³) for Floyd-Warshall algorithm
- Space Complexity: O(T²) for distance matrices
- Path merging: O(n*m) where n, m are path lengths

## Future Improvements
1. Implement parallel path finding for large graphs
2. Add dynamic fuel cost adaptation
3. Optimize memory usage for large networks
4. Add support for time window constraints
5. Implement real-time path updates

## Project Status: Work in Progress

### Completed Features:
- Basic path finding implementation
- Fuel constraint management
- Hub-based delivery tracking
- Path merging logic

### Pending Improvements:
1. **Algorithm Optimization**
   - Better local optima avoidance
   - Improved path merging strategy
   - Dynamic parameter adjustment
   - All houses are not visited - need to integrate findOptimalPairPath function which exists but is not yet integrated into the main path finding logic

2. **Performance Enhancements**
   - Memory usage optimization
   - Computation speed improvements
   - Better scaling for large graphs

3. **Additional Features Needed**
   - Multiple vehicle support
   - Time window constraints
   - Dynamic cost updates
   - Real-time path recalculation

4. **Code Quality**
   - More comprehensive testing
   - Better error handling
   - Code documentation
   - Performance profiling

Please note that this is an active development project and the implementation is not yet complete. Contributions and suggestions are welcome.
