#include <bits/stdc++.h>
using namespace std;

// Global variables for storing input
int N, T, M, K, F;  // Removed X from global variables as it will be passed as parameter
vector<vector<pair<int, int>>> adj; // adjacency list: {node, fuel_cost}

// Structure to hold state for each cell in dp matrix
struct State {
    vector<int> path;
    vector<int> visited_hubs;
    vector<int> delivered_houses;
    vector<int> undelivered_houses;
    array<int, 3> fuel_costs; // {total, initial, last}
    double opti_value;

    State() : fuel_costs({INT_MAX, INT_MAX, INT_MAX}), opti_value(INT_MAX) {}
};

// Structure for simplified brute force approach
struct BruteState {
    vector<int> path;
    array<int, 3> fuel_costs; // {total, initial, last}
    double opti_value;

    BruteState() : fuel_costs({INT_MAX, INT_MAX, INT_MAX}), opti_value(INT_MAX) {}
};

// Update PairPath struct to include starting fuel station
struct PairPath {
    vector<int> path;
    int totalFuel;
    int startFs;  // Starting fuel station index
    
    PairPath() : totalFuel(INT_MAX), startFs(-1) {}
};

// Structure to track a complete path attempt
struct PathAttempt {
    State current_state;
    set<int> covered_houses;
    vector<int> sequence; // sequence of nodes chosen (e.g., i, j, k, ...)

    PathAttempt() {}
    PathAttempt(const State& s, const set<int>& c, const vector<int>& seq)
        : current_state(s), covered_houses(c), sequence(seq) {}

    // Comparator for priority queue (prioritize more delivered houses)
    bool operator>(const PathAttempt& other) const {
        return covered_houses.size() < other.covered_houses.size();
    }
};

vector<vector<State>> dp;
vector<vector<BruteState>> dpbrute;
vector<PairPath> brutePairs;  // Store optimal paths for each hub-house pair
map<int, int> nodeType; // 1: fuel station, 2: hub, 3: house
map<int, int> hubToHouse; // maps hub to its corresponding house
map<int, int> houseToHub; // maps house to its corresponding hub
vector<int> fuelStations;
vector<int> hubs;
vector<int> houses;

bool isFuelStation(int node) {
    return nodeType[node] == 1;
}

// Helper function to calculate optimization value
double calculateOptiValue(const State& state, int x) {
    int delivered = state.delivered_houses.size();
    int visited = state.visited_hubs.size();
    
    if (delivered == 0 || visited == 0) return INT_MAX;
    
    if (delivered > x) {
        return state.fuel_costs[0] * (double(visited) / delivered + 1);
    } else {
        return double(state.fuel_costs[0]) / visited;
    }
}

// Helper function to update delivered and undelivered houses
void updateDeliveryStatus(State& result, const State& a, const State& b) {
    // Start with a's delivered houses
    result.delivered_houses = a.delivered_houses;
    result.undelivered_houses = b.undelivered_houses;
    
    // Check b's undelivered houses against a's visited hubs
    for(int house : b.undelivered_houses) {
        int hub = houseToHub[house];
        if(find(a.visited_hubs.begin(), a.visited_hubs.end(), hub) != a.visited_hubs.end()) {
            result.delivered_houses.push_back(house);
        } else {
            result.undelivered_houses.push_back(house);
        }
    }
    
    // Add b's delivered houses
    for(int house : b.delivered_houses) {
        result.delivered_houses.push_back(house);
    }
    
    // Remove duplicates
    sort(result.delivered_houses.begin(), result.delivered_houses.end());
    result.delivered_houses.erase(
        unique(result.delivered_houses.begin(), result.delivered_houses.end()),
        result.delivered_houses.end()
    );
    
    sort(result.undelivered_houses.begin(), result.undelivered_houses.end());
    result.undelivered_houses.erase(
        unique(result.undelivered_houses.begin(), result.undelivered_houses.end()),
        result.undelivered_houses.end()
    );
}

// Helper function for Floyd-Warshall
State floydUpdate(const State& ij, const State& ik, const State& kj, int x) {
    State result;
    const int INF = INT_MAX;
    
    // If either path doesn't exist, return invalid state
    if (ik.fuel_costs[0] == INF || kj.fuel_costs[0] == INF)
        return ij;
    
    // Check if connecting through k exceeds fuel capacity
    if (ik.fuel_costs[2] + kj.fuel_costs[1] > F)
        return ij;
        
    // Calculate new fuel costs
    result.fuel_costs = {
        ik.fuel_costs[0] + kj.fuel_costs[0],
        ik.fuel_costs[1],
        kj.fuel_costs[2]
    };
    
    // Combine paths
    result.path = ik.path;
    result.path.insert(result.path.end(), kj.path.begin() + 1, kj.path.end());
    
    // Combine visited hubs
    result.visited_hubs = ik.visited_hubs;
    for(int hub : kj.visited_hubs) {
        result.visited_hubs.push_back(hub);
    }
    sort(result.visited_hubs.begin(), result.visited_hubs.end());
    result.visited_hubs.erase(
        unique(result.visited_hubs.begin(), result.visited_hubs.end()),
        result.visited_hubs.end()
    );
    
    // Update delivery status
    updateDeliveryStatus(result, ik, kj);
    
    // Calculate optimization value with parameter x
    result.opti_value = calculateOptiValue(result, x);
    
    // Return better path based on optimization value
    return (result.opti_value < ij.opti_value) ? result : ij;
}

void initializeFloydWarshall(int x) {
    const int INF = INT_MAX;
    // Initialize dp matrix
    dp.resize(T, vector<State>(T));
    
    // Initialize with direct edges
    for(int i = 0; i < T; i++) {
        // Path to self
        dp[i][i].path = {i};
        dp[i][i].fuel_costs = {0, 0, 0};
        if(nodeType[i] == 2) { // if it's a hub
            dp[i][i].visited_hubs.push_back(i);
        }
        
        for(int j = 0; j < adj[i].size(); j++) {
            pair<int, int> edge = adj[i][j];
            int v = edge.first;
            int cost = edge.second;
            
            // Calculate fuel costs based on whether nodes are fuel stations
            int firstCost = isFuelStation(i) ? 0 : cost;
            int lastCost = isFuelStation(v) ? 0 : cost;
            
            // Initialize path and costs
            dp[i][v].path = {i, v};
            dp[i][v].fuel_costs = {cost, firstCost, lastCost};
            
            // Initialize visited hubs
            if(nodeType[i] == 2) {
                dp[i][v].visited_hubs.push_back(i);
            }
            if(nodeType[v] == 2) {
                dp[i][v].visited_hubs.push_back(v);
            }
            
            // Initialize houses
            if(nodeType[v] == 3) {
                if(find(dp[i][v].visited_hubs.begin(), dp[i][v].visited_hubs.end(), houseToHub[v]) != dp[i][v].visited_hubs.end()) {
                    dp[i][v].delivered_houses.push_back(v);
                } else {
                    dp[i][v].undelivered_houses.push_back(v);
                }
            }
            
            dp[i][v].opti_value = calculateOptiValue(dp[i][v], x);
        }
    }
    
    // Floyd-Warshall algorithm
    for(int k = 0; k < T; k++) {
        for(int i = 0; i < T; i++) {
            for(int j = 0; j < T; j++) {
                dp[i][j] = floydUpdate(dp[i][j], dp[i][k], dp[k][j], x);
            }
        }
    }
}

// Helper function for merging BruteStates
BruteState mergeBruteStates(const BruteState& a, const BruteState& b) {
    BruteState result;
    const int INF = INT_MAX;
    
    // Check if connecting paths exceeds fuel capacity
    if (a.fuel_costs[2] + b.fuel_costs[1] > F) {
        result.fuel_costs = {INF, INF, INF};
        result.opti_value = INF;
        return result;
    }
    
    // Merge paths
    result.path = a.path;
    if(!b.path.empty()) {
        result.path.insert(result.path.end(), b.path.begin() + 1, b.path.end());
    }
    
    // Update fuel costs
    result.fuel_costs = {
        a.fuel_costs[0] + b.fuel_costs[0],  // total cost
        a.fuel_costs[1],                     // initial cost remains from first path
        b.fuel_costs[2]                      // last cost comes from second path
    };
    
    // Set optimization value to total fuel cost
    result.opti_value = result.fuel_costs[0];
    
    return result;
}

// Helper function for Floyd-Warshall for brute approach
BruteState bruteFloydUpdate(const BruteState& ij, const BruteState& ik, const BruteState& kj) {
    // If either path doesn't exist, return ij
    if (ik.fuel_costs[0] == INT_MAX || kj.fuel_costs[0] == INT_MAX)
        return ij;
    
    BruteState merged = mergeBruteStates(ik, kj);
    
    // Return path with minimum total fuel cost
    return (merged.opti_value < ij.opti_value) ? merged : ij;
}

void initializeBruteFloydWarshall() {
    const int INF = INT_MAX;
    // Initialize dpbrute matrix
    dpbrute.resize(T, vector<BruteState>(T));
    
    // Initialize with direct edges
    for(int i = 0; i < T; i++) {
        // Path to self
        dpbrute[i][i].path = {i};
        dpbrute[i][i].fuel_costs = {0, 0, 0};
        dpbrute[i][i].opti_value = 0;
        
        for(int j = 0; j < adj[i].size(); j++) {
            pair<int, int> edge = adj[i][j];
            int v = edge.first;
            int cost = edge.second;
            
            // Calculate fuel costs based on whether nodes are fuel stations
            int firstCost = isFuelStation(i) ? 0 : cost;
            int lastCost = isFuelStation(v) ? 0 : cost;
            
            // Initialize path and costs
            dpbrute[i][v].path = {i, v};
            dpbrute[i][v].fuel_costs = {cost, firstCost, lastCost};
            dpbrute[i][v].opti_value = cost;
        }
    }
    
    // Floyd-Warshall algorithm
    for(int k = 0; k < T; k++) {
        for(int i = 0; i < T; i++) {
            for(int j = 0; j < T; j++) {
                dpbrute[i][j] = bruteFloydUpdate(dpbrute[i][j], dpbrute[i][k], dpbrute[k][j]);
            }
        }
    }
}

void takeInput() {
    // Read N, T, M, K, F
    cin >> N >> T >> M >> K >> F;

    // Initialize adjacency list
    adj.resize(T);

    // Read hubs
    for(int i = 0; i < N; i++) {
        int hub;
        cin >> hub;
        hubs.push_back(hub);
        nodeType[hub] = 2; // Mark as hub
    }

    // Read houses
    for(int i = 0; i < N; i++) {
        int house;
        cin >> house;
        houses.push_back(house);
        nodeType[house] = 3; // Mark as house
        // Map hub to house and vice versa
        hubToHouse[hubs[i]] = house;
        houseToHub[house] = hubs[i];
    }

    // Read fuel stations
    for(int i = 0; i < K; i++) {
        int station;
        cin >> station;
        fuelStations.push_back(station);
        nodeType[station] = 1; // Mark as fuel station
    }

    // Read edges
    for(int i = 0; i < M; i++) {
        int u, v, c;
        cin >> u >> v >> c;
        // Add edges to adjacency list (undirected)
        adj[u].push_back({v, c});
        adj[v].push_back({u, c});
    }
}

State mergeStates(const State& a, const State& b, int x) {
    State result;
    const int INF = INT_MAX;
    
    // Check if connecting paths exceeds fuel capacity
    if (a.fuel_costs[2] + b.fuel_costs[1] > F) {
        result.fuel_costs = {INF, INF, INF};
        result.opti_value = INF;
        return result;
    }
    
    // Merge paths
    result.path = a.path;
    if(!b.path.empty()) {
        result.path.insert(result.path.end(), b.path.begin() + 1, b.path.end());
    }
    
    // Update fuel costs
    result.fuel_costs = {
        a.fuel_costs[0] + b.fuel_costs[0],  // total cost
        a.fuel_costs[1],                     // initial cost remains from first path
        b.fuel_costs[2]                      // last cost comes from second path
    };
    
    // Merge visited hubs (same as before)
    result.visited_hubs = a.visited_hubs;
    for(int hub : b.visited_hubs) {
        result.visited_hubs.push_back(hub);
    }
    sort(result.visited_hubs.begin(), result.visited_hubs.end());
    result.visited_hubs.erase(
        unique(result.visited_hubs.begin(), result.visited_hubs.end()),
        result.visited_hubs.end()
    );
    
    // Update delivery status using the existing function
    updateDeliveryStatus(result, a, b);
    
    // Calculate optimization value
    result.opti_value = calculateOptiValue(result, x);
    
    return result;
}

State findOptimalPath(int x, int y) {
    State best_path_found;
    int max_houses_covered = -1;

    // Find initial i,j with maximum delivered houses as starting point
    int start_i = -1, start_j = -1;
    int initial_max_delivered = -1;

    cout << "\n=== Finding Initial Path ===\n";
    for (int i = 0; i < T; i++) {
        for (int j = 0; j < T; j++) {
            if (dp[i][j].fuel_costs[0] != INT_MAX) {
                int current_delivered = dp[i][j].delivered_houses.size();
                if (current_delivered > initial_max_delivered) {
                    initial_max_delivered = current_delivered;
                    start_i = i;
                    start_j = j;
                    cout << "Found better initial path from " << i << " to " << j << "\n";
                    cout << "Delivered houses (" << current_delivered << "): ";
                    for(int house : dp[i][j].delivered_houses) cout << house << " ";
                    cout << "\n";
                }
            }
        }
    }

    if (start_i == -1) return State();

    // Initialize the first attempt
    PathAttempt initial_attempt;
    initial_attempt.current_state = dp[start_i][start_j];
    for (int house : initial_attempt.current_state.delivered_houses) {
        initial_attempt.covered_houses.insert(house);
    }
    initial_attempt.sequence = {start_i, start_j};

    best_path_found = initial_attempt.current_state;
    max_houses_covered = initial_attempt.covered_houses.size();

    cout << "\n=== Starting Path Exploration ===\n";
    stack<PathAttempt> exploration_stack;
    exploration_stack.push(initial_attempt);

    int backtrack_triggers = 0;
    int step = 1;

    while (!exploration_stack.empty() && backtrack_triggers <= y) {
        PathAttempt current_attempt = exploration_stack.top();
        exploration_stack.pop();

        cout << "\nStep " << step++ << ":\n";
        cout << "Current sequence: ";
        for(int node : current_attempt.sequence) cout << node << " ";
        cout << "\n";
        cout << "Houses covered (" << current_attempt.covered_houses.size() << "): ";
        for(int house : current_attempt.covered_houses) cout << house << " ";
        cout << "\nFuel cost: " << current_attempt.current_state.fuel_costs[0] << "\n";

        // Update best path if current attempt is better
        if (current_attempt.covered_houses.size() > max_houses_covered) {
            max_houses_covered = current_attempt.covered_houses.size();
            best_path_found = current_attempt.current_state;
            cout << "*** New best path found! ***\n";
        }

        if (current_attempt.covered_houses.size() == houses.size()) {
            cout << "All houses covered! Continuing to check other paths...\n";
            continue;
        }

        int curr_j = current_attempt.sequence.back();
        bool found_improvement = false;
        vector<pair<int, State>> possible_next_states;

        // Find possible next moves
        for (int k = 0; k < T; k++) {
            if (dp[curr_j][k].fuel_costs[0] == INT_MAX) continue;

            State potential_next_state = mergeStates(current_attempt.current_state, dp[curr_j][k], x);
            if (potential_next_state.fuel_costs[0] == INT_MAX) continue;

            bool delivers_new = false;
            for (int house : potential_next_state.delivered_houses) {
                if (current_attempt.covered_houses.find(house) == current_attempt.covered_houses.end()) {
                    delivers_new = true;
                    break;
                }
            }

            if (delivers_new) {
                possible_next_states.push_back({k, potential_next_state});
                found_improvement = true;
            }
        }

        if (found_improvement) {
            cout << "Found " << possible_next_states.size() << " possible improvements.\n";
            for(auto& move : possible_next_states) {
                int next_k = move.first;
                State next_state = move.second;

                PathAttempt next_attempt = current_attempt;
                next_attempt.current_state = next_state;
                next_attempt.sequence.push_back(next_k);
                next_attempt.covered_houses.clear();
                for(int house : next_state.delivered_houses) {
                    next_attempt.covered_houses.insert(house);
                }

                exploration_stack.push(next_attempt);
            }
        } else {
            backtrack_triggers++;
            cout << "Dead end reached. Backtrack trigger " << backtrack_triggers << "/" << y << "\n";
        }
    }

    cout << "\n=== Search Complete ===\n";
    cout << "Final best path delivers " << best_path_found.delivered_houses.size() << " houses\n";
    cout << "Houses: ";
    for(int house : best_path_found.delivered_houses) cout << house << " ";
    cout << "\nTotal fuel cost: " << best_path_found.fuel_costs[0] << "\n";

    return best_path_found;
}

// Modified function to return a vector of K best paths, one for each starting fuel station
pair<vector<PairPath>, vector<PairPath>> findOptimalPairPath(int hub, int house) {
    vector<PairPath> bestPaths(K);
    vector<PairPath> lonelypairs(K);
    
    // Initialize both path vectors with correct starting fuel station index and INT_MAX cost
    for(int start = 0; start < K; start++) {
        bestPaths[start].startFs = fuelStations[start];
        bestPaths[start].totalFuel = INT_MAX;
        lonelypairs[start].startFs = fuelStations[start];
        lonelypairs[start].totalFuel = INT_MAX;
    }
    
    // First calculate direct paths (fuel station -> house -> same fuel station)
    for(int start = 0; start < K; start++) {
        int fs = fuelStations[start];
        BruteState toHouse = dpbrute[fs][house];    // fs to house
        BruteState fromHouse = dpbrute[house][fs];   // house back to fs
        
        // Check if path exists and meets fuel constraints
        if(toHouse.fuel_costs[0] != INT_MAX && fromHouse.fuel_costs[0] != INT_MAX) {
            if(toHouse.fuel_costs[2] + fromHouse.fuel_costs[1] <= F) {
                int totalFuel = toHouse.fuel_costs[0] + fromHouse.fuel_costs[0];
                if(totalFuel < lonelypairs[start].totalFuel) {
                    vector<int> fullPath = toHouse.path;
                    fullPath.insert(fullPath.end(), fromHouse.path.begin() + 1, fromHouse.path.end());
                    
                    lonelypairs[start].path = fullPath;
                    lonelypairs[start].totalFuel = totalFuel;
                }
            }
        }
    }
    
    // Then calculate paths that visit the hub
    for(int start = 0; start < K; start++) {
        int fs1 = fuelStations[start];
        
        // Try all possible intermediate fuel stations
        for(int mid = 0; mid < K; mid++) {
            int fs2 = fuelStations[mid];
            
            // Get path segments for pattern: fs1-hub-fs2-house-fs1
            BruteState path1 = dpbrute[fs1][hub];      // fs1 to hub
            BruteState path2 = dpbrute[hub][fs2];      // hub to fs2
            BruteState path3 = dpbrute[fs2][house];    // fs2 to house
            BruteState path4 = dpbrute[house][fs1];    // house back to fs1
            
            // Skip if any segment is invalid
            if(path1.fuel_costs[0] == INT_MAX || path2.fuel_costs[0] == INT_MAX || 
               path3.fuel_costs[0] == INT_MAX || path4.fuel_costs[0] == INT_MAX)
                continue;
            
            // Check fuel constraints between consecutive segments
            if(path1.fuel_costs[2] + path2.fuel_costs[1] > F) continue;  // fs1->hub->fs2
            if(path2.fuel_costs[2] + path3.fuel_costs[1] > F) continue;  // hub->fs2->house
            if(path3.fuel_costs[2] + path4.fuel_costs[1] > F) continue;  // fs2->house->fs1
            
            // Calculate total fuel cost
            int totalFuel = path1.fuel_costs[0] + path2.fuel_costs[0] + 
                           path3.fuel_costs[0] + path4.fuel_costs[0];
            
            // Update if better than current best for this starting fuel station
            if(totalFuel < bestPaths[start].totalFuel) {
                vector<int> fullPath = path1.path;
                fullPath.insert(fullPath.end(), path2.path.begin() + 1, path2.path.end());
                fullPath.insert(fullPath.end(), path3.path.begin() + 1, path3.path.end());
                fullPath.insert(fullPath.end(), path4.path.begin() + 1, path4.path.end());
                
                bestPaths[start].path = fullPath;
                bestPaths[start].totalFuel = totalFuel;
            }
        }
    }
    
    return {bestPaths, lonelypairs};
}

int main() {
    ios_base::sync_with_stdio(false);
    cin.tie(NULL);
    
    // Redirect input to read from input_1.txt
    freopen("drive-download-20250420T215205Z-001/input_1.txt", "r", stdin);
    // Redirect output to write to output.txt
    freopen("output.txt", "w", stdout);
    
    takeInput();
    int x = 10;
    int y = 5;  // Allow 5 backtrack attempts
    
    initializeFloydWarshall(x);
    initializeBruteFloydWarshall();
    
    State final_path = findOptimalPath(x, y);
    
    // Output path length and nodes
    cout << final_path.path.size() << "\n";
    for(int i = 0; i < final_path.path.size(); i++) {
        cout << final_path.path[i];
        if(i < final_path.path.size() - 1) cout << " ";
    }
    cout << "\n";
    
    // Output delivered houses information
    cout << "\nDelivered Houses: " << final_path.delivered_houses.size() << "/" << houses.size() << "\n";
    cout << "Houses delivered: ";
    for(int house : final_path.delivered_houses) {
        cout << house << " ";
    }
    cout << "\n";
    cout << "Total fuel cost: " << final_path.fuel_costs[0] << "\n";
    
    return 0;
}