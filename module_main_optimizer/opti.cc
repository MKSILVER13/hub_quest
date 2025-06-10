#include <bits/stdc++.h>
using namespace std;

// Global variables for storing input
int N, T, M, K, F;  // Removed X from global variables as it will be passed as parameter
vector<vector<pair<int, int>>> adj; // adjacency list: {node, fuel_cost}

// Structure to hold state for each cell in dp matrix
struct State {
    vector<int> path;
    vector<int> visited_hubs; // map: visited_hubs[i] == 1 if hubs[i] visited
    vector<int> listed_hubs; // list of visited hubs (indices in hubs[])
    int visited_hubs_count = 0;
    vector<int> delivered_houses; // map: delivered_houses[i] == 1 if houses[i] delivered
    vector<int> list_delivered_houses; // list of delivered houses (indices in houses[])
    int delivered_houses_count = 0;
    vector<int> undelivered_houses; // map: undelivered_houses[i] == 1 if houses[i] undelivered
    vector<int> list_undelivered_houses; // list of undelivered houses (indices in houses[])
    int undelivered_houses_count = 0;
    array<int, 3> fuel_costs; // {total, initial, last}
    bool has_fuel_station = false; // Flag to check if path has a fuel station
    double opti_value;

    State() : path(), visited_hubs(), listed_hubs(), visited_hubs_count(0),
              delivered_houses(), list_delivered_houses(), delivered_houses_count(0),
              undelivered_houses(), list_undelivered_houses(), undelivered_houses_count(0),
              fuel_costs({INT_MAX, INT_MAX, INT_MAX}), opti_value(INT_MAX) {}

};

// Structure for simplified brute force approach
struct BruteState {
    vector<int> path;
    array<int, 3> fuel_costs; // {total, initial, last}
    double opti_value;
    bool has_fuel_station = false; // Flag to check if path has a fuel station

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
    int delivered = state.delivered_houses_count;
    int visited = state.visited_hubs_count;
    int cost = state.fuel_costs[0] + state.fuel_costs[1] + state.fuel_costs[2];
    std::set<int> unique_nodes(state.path.begin(), state.path.end());
    int revisit_penalty = state.path.size() - unique_nodes.size(); // Penalize revisits

    if (delivered > x) {
        return cost * (double(visited) / delivered) + revisit_penalty * 10;
    } else if (visited > 0) {
        return double(cost) / visited + revisit_penalty * 10;
    } else {
        return cost + revisit_penalty * 10; // Penalize revisits heavily
    }
}

// Helper function to update delivered and undelivered houses
void updateDeliveryStatus(State& result, const State& a, const State& b) {
    // Reset maps and lists
    int n_hubs = hubs.size();
    int n_houses = houses.size();
    result.visited_hubs.assign(n_hubs, 0);
    result.listed_hubs.clear();
    result.visited_hubs_count = 0;
    result.delivered_houses.assign(n_houses, 0);
    result.list_delivered_houses.clear();
    result.delivered_houses_count = 0;
    result.undelivered_houses.assign(n_houses, 0);
    result.list_undelivered_houses.clear();
    result.undelivered_houses_count = 0;

    // Merge visited hubs from a and b
    for (int i = 0; i < n_hubs; ++i) {
        if ((a.visited_hubs.size() > i && a.visited_hubs[i]) || (b.visited_hubs.size() > i && b.visited_hubs[i])) {
            result.visited_hubs[i] = 1;
            result.listed_hubs.push_back(i);
            result.visited_hubs_count++;
        }
    }

    // Merge delivered houses from a and b
    for (int i = 0; i < n_houses; ++i) {
        bool delivered = (a.delivered_houses.size() > i && a.delivered_houses[i]) || (b.delivered_houses.size() > i && b.delivered_houses[i]);
        // Check if b's undelivered house can be delivered by a's visited hub
        if (!delivered && b.undelivered_houses.size() > i && b.undelivered_houses[i]) {
            int house_node = houses[i];
            int hub_node = houseToHub[house_node];
            auto it = std::find(hubs.begin(), hubs.end(), hub_node);
            if (it != hubs.end()) {
                int hub_idx = std::distance(hubs.begin(), it);
                if (a.visited_hubs.size() > hub_idx && a.visited_hubs[hub_idx]) {
                    delivered = true;
                }
            }
        }
        if (delivered) {
            result.delivered_houses[i] = 1;
            result.list_delivered_houses.push_back(i);
            result.delivered_houses_count++;
        } else {
            // If not delivered, check if undelivered in a or b
            if ((a.undelivered_houses.size() > i && a.undelivered_houses[i]) || (b.undelivered_houses.size() > i && b.undelivered_houses[i])) {
                result.undelivered_houses[i] = 1;
                result.list_undelivered_houses.push_back(i);
                result.undelivered_houses_count++;
            }
        }
    }
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
        (ik.has_fuel_station ? ik.fuel_costs[1] : ik.fuel_costs[0] + kj.fuel_costs[1]),
        (kj.has_fuel_station ? kj.fuel_costs[2] : ik.fuel_costs[2] + kj.fuel_costs[0])
    };
    result.has_fuel_station = ik.has_fuel_station || kj.has_fuel_station;
    // Combine paths
    result.path = ik.path;
    result.path.insert(result.path.end(), kj.path.begin() + 1, kj.path.end());

    // Merge visited hubs, delivered and undelivered houses using the new structure
    updateDeliveryStatus(result, ik, kj);

    // Calculate optimization value with parameter x
    result.opti_value = calculateOptiValue(result, x);

    // Return better path based on optimization value
    return (result.opti_value < ij.opti_value) ? result : ij;
}

void initializeFloydWarshall(int x) {
    const int INF = INT_MAX;
    int n_hubs = hubs.size();
    int n_houses = houses.size();
    // Initialize dp matrix
    dp.resize(T, vector<State>(T));
    
    // Initialize with direct edges
    for(int i = 0; i < T; i++) {
        // Path to self
        dp[i][i].path = {i};
        dp[i][i].fuel_costs = {0, 0, 0};
        dp[i][i].has_fuel_station = (nodeType[i] == 1);
        dp[i][i].visited_hubs.assign(n_hubs, 0);
        dp[i][i].listed_hubs.clear();
        dp[i][i].visited_hubs_count = 0;
        dp[i][i].delivered_houses.assign(n_houses, 0);
        dp[i][i].list_delivered_houses.clear();
        dp[i][i].delivered_houses_count = 0;
        dp[i][i].undelivered_houses.assign(n_houses, 0);
        dp[i][i].list_undelivered_houses.clear();
        dp[i][i].undelivered_houses_count = 0;
        if(nodeType[i] == 2) { // if it's a hub
            auto it = std::find(hubs.begin(), hubs.end(), i);
            if (it != hubs.end()) {
                int idx = std::distance(hubs.begin(), it);
                dp[i][i].visited_hubs[idx] = 1;
                dp[i][i].listed_hubs.push_back(idx);
                dp[i][i].visited_hubs_count = 1;
            }
        }
        if(nodeType[i] == 3) { // if it's a house
            auto it = std::find(houses.begin(), houses.end(), i);
            if (it != houses.end()) {
                int idx = std::distance(houses.begin(), it);
                dp[i][i].undelivered_houses[idx] = 1;
                dp[i][i].list_undelivered_houses.push_back(idx);
                dp[i][i].undelivered_houses_count = 1;
            }
        }
        for(int j = 0; j < adj[i].size(); j++) {
            pair<int, int> edge = adj[i][j];
            int v = edge.first;
            int cost = edge.second;
            int firstCost = isFuelStation(i) ? 0 : cost;
            int lastCost = isFuelStation(v) ? 0 : cost;
            dp[i][v].path = {i, v};
            dp[i][v].fuel_costs = {cost, firstCost, lastCost};
            dp[i][v].has_fuel_station = isFuelStation(i) || isFuelStation(v);
            dp[i][v].visited_hubs.assign(n_hubs, 0);
            dp[i][v].listed_hubs.clear();
            dp[i][v].visited_hubs_count = 0;
            dp[i][v].delivered_houses.assign(n_houses, 0);
            dp[i][v].list_delivered_houses.clear();
            dp[i][v].delivered_houses_count = 0;
            dp[i][v].undelivered_houses.assign(n_houses, 0);
            dp[i][v].list_undelivered_houses.clear();
            dp[i][v].undelivered_houses_count = 0;
            if(nodeType[i] == 2) {
                auto it = std::find(hubs.begin(), hubs.end(), i);
                if (it != hubs.end()) {
                    int idx = std::distance(hubs.begin(), it);
                    dp[i][v].visited_hubs[idx] = 1;
                    dp[i][v].listed_hubs.push_back(idx);
                    dp[i][v].visited_hubs_count++;
                }
            }
            if(nodeType[v] == 2) {
                auto it = std::find(hubs.begin(), hubs.end(), v);
                if (it != hubs.end()) {
                    int idx = std::distance(hubs.begin(), it);
                    if (!dp[i][v].visited_hubs[idx]) {
                        dp[i][v].visited_hubs[idx] = 1;
                        dp[i][v].listed_hubs.push_back(idx);
                        dp[i][v].visited_hubs_count++;
                    }
                }
            }
            if(nodeType[v] == 3) {
                auto it = std::find(houses.begin(), houses.end(), v);
                if (it != houses.end()) {
                    int idx = std::distance(houses.begin(), it);
                    int hub_node = houseToHub[v];
                    auto hub_it = std::find(hubs.begin(), hubs.end(), hub_node);
                    bool delivered = false;
                    if (hub_it != hubs.end()) {
                        int hub_idx = std::distance(hubs.begin(), hub_it);
                        if (dp[i][v].visited_hubs[hub_idx]) delivered = true;
                    }
                    if (delivered) {
                        dp[i][v].delivered_houses[idx] = 1;
                        dp[i][v].list_delivered_houses.push_back(idx);
                        dp[i][v].delivered_houses_count++;
                    } else {
                        dp[i][v].undelivered_houses[idx] = 1;
                        dp[i][v].list_undelivered_houses.push_back(idx);
                        dp[i][v].undelivered_houses_count++;
                    }
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
        (a.has_fuel_station?a.fuel_costs[1]:a.fuel_costs[0]+b.fuel_costs[1]),                    // initial cost remains from first path
        (b.has_fuel_station?b.fuel_costs[2]:a.fuel_costs[2]+b.fuel_costs[0])                    // last cost comes from second path
    };
    result.has_fuel_station = a.has_fuel_station || b.has_fuel_station;
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
        dpbrute[i][i].has_fuel_station = isFuelStation(i);
        
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
            dpbrute[i][v].has_fuel_station = isFuelStation(i) || isFuelStation(v);
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
    adj.clear();
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
    int n_hubs = hubs.size();
    int n_houses = houses.size();

    // Check if connecting paths exceeds fuel capacity
    if (a.fuel_costs[2] + b.fuel_costs[1] > F) {
        result.fuel_costs = {INF, INF, INF};
        result.opti_value = INF;
        return result;
    }

    // Merge paths
    result.path = a.path;
    if (!b.path.empty()) {
        result.path.insert(result.path.end(), b.path.begin() + 1, b.path.end());
    }

    // Update fuel costs
    result.fuel_costs = {
        a.fuel_costs[0] + b.fuel_costs[0],
        (a.has_fuel_station ? a.fuel_costs[1] : a.fuel_costs[0] + b.fuel_costs[1]),
        (b.has_fuel_station ? b.fuel_costs[2] : a.fuel_costs[2] + b.fuel_costs[0])
    };

    // Update has_fuel_station flag
    result.has_fuel_station = a.has_fuel_station || b.has_fuel_station;

    // Merge visited hubs, delivered and undelivered houses using the new structure
    updateDeliveryStatus(result, a, b);

    // Calculate optimization value
    result.opti_value = calculateOptiValue(result, x);

    return result;
}


auto start = chrono::high_resolution_clock::now();
State findOptimalPath(int x, int y) {
    State best_path_found;
    int max_houses_covered = -1;
    int n_houses = houses.size();

    // Find initial i,j with maximum delivered houses as starting point
    int start_i = -1, start_j = -1;
    int initial_max_delivery = -1;
    int tot_nodes = -1;
    int cost = -1;

    cout << "\n=== Finding Initial Path ===\n";
    for (int i = 0; i < T; i++) {
        for (int j = 0; j < T; j++) {
            if (dp[i][j].fuel_costs[0] != INT_MAX) {
                int current_delivery = dp[i][j].delivered_houses_count;
                int current_tot_nodes = dp[i][j].path.size();
                int current_cost = dp[i][j].fuel_costs[0];

                // Priority order: hubs > nodes > cost
                if (current_delivery > initial_max_delivery ||
                    (current_delivery == initial_max_delivery && current_tot_nodes > tot_nodes) ||
                    (current_delivery == initial_max_delivery && current_tot_nodes == tot_nodes && current_cost < cost)) {

                    initial_max_delivery = current_delivery;
                    tot_nodes = current_tot_nodes;
                    cost = current_cost;
                    start_i = i;
                    start_j = j;

                    cout << "Found better initial path from " << i << " to " << j << "\n";
                    cout << "Delivered houses (" << dp[i][j].delivered_houses_count << "): ";
                    for (int idx : dp[i][j].list_delivered_houses) cout << houses[idx] << " ";
                    cout << "\n";
                }
            }
        }
    }

    if (start_i == -1) return State();

    // Initialize the first attempt
    PathAttempt initial_attempt;
    initial_attempt.current_state = dp[start_i][start_j];
    initial_attempt.covered_houses.clear();
    for (int idx : initial_attempt.current_state.list_delivered_houses) {
        initial_attempt.covered_houses.insert(houses[idx]);
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
        if(chrono::high_resolution_clock::now() - start > chrono::minutes(10)) {
            cout << "Time limit exceeded. Stopping exploration.\n";
        }
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
        if (current_attempt.covered_houses.size() > max_houses_covered ||
            (current_attempt.covered_houses.size() == max_houses_covered && current_attempt.current_state.visited_hubs_count > best_path_found.visited_hubs_count) ||
            (current_attempt.covered_houses.size() == max_houses_covered && current_attempt.current_state.visited_hubs_count == best_path_found.visited_hubs_count && current_attempt.current_state.fuel_costs[0] < best_path_found.fuel_costs[0])) {
            max_houses_covered = current_attempt.covered_houses.size();
            best_path_found = current_attempt.current_state;
            cout << "*** New best path found! ***\n";
        }

        if (current_attempt.covered_houses.size() == n_houses) {
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
            for (int idx : potential_next_state.list_delivered_houses) {
                if (current_attempt.covered_houses.find(houses[idx]) == current_attempt.covered_houses.end()) {
                    delivers_new = true;
                    break;
                }
            }

            if (delivers_new) {
                possible_next_states.push_back({k, potential_next_state});
                found_improvement = true;
            }
        }
        sort(possible_next_states.begin(), possible_next_states.end(),
             [x](const pair<int, State>& a, const pair<int, State>& b) {
                 int a_value = a.second.delivered_houses_count > x ? 
                               a.second.delivered_houses_count : 
                               a.second.visited_hubs_count;
                 int b_value = b.second.delivered_houses_count > x ? 
                               b.second.delivered_houses_count : 
                               b.second.visited_hubs_count;
                 return a_value > b_value;
             });
        if (found_improvement) {
            cout << "Found " << possible_next_states.size() << " possible improvements.\n";
            for(auto& move : possible_next_states) {
                int next_k = move.first;
                State next_state = move.second;

                PathAttempt next_attempt = current_attempt;
                next_attempt.current_state = next_state;
                next_attempt.sequence.push_back(next_k);
                next_attempt.covered_houses.clear();
                for (int idx : next_state.list_delivered_houses) {
                    next_attempt.covered_houses.insert(houses[idx]);
                }

                exploration_stack.push(next_attempt);
            }
        } else {
            backtrack_triggers++;
            cout << "Dead end reached. Backtrack trigger " << backtrack_triggers << "/" << y << "\n";
        }
    }

    cout << "\n=== Search Complete ===\n";
    cout << "Final best path delivers " << best_path_found.delivered_houses_count << " houses\n";
    cout << "Houses: ";
    for(int idx : best_path_found.list_delivered_houses) cout << houses[idx] << " ";
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
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(NULL);
  
    if (freopen("../project_general_inputs/input.txt", "r", stdin) == NULL) {
        perror("Error opening input file: ../project_general_inputs/input.txt");
        return 1;
    }
    
    
    if (freopen("output.txt", "w", stdout) == NULL) {
        perror("Error opening output file: output.txt");
        return 1;
    }
    
    takeInput();
    int z = 2;
    int x = 2;
    int y = 5;  
    
    initializeFloydWarshall(x);
    for(int i = 0; i < T; i++) {
        for(int j = 0; j < T; j++) {
            cout << "dp[" << i << "][" << j << "] - Path: ";
            for(int node : dp[i][j].path) cout << node << " ";
            cout << ", Fuel Costs: " << dp[i][j].fuel_costs[0] << ", " 
                 << dp[i][j].fuel_costs[1] << ", " << dp[i][j].fuel_costs[2] 
                 << ", Opti Value: " << dp[i][j].opti_value
                << ", visited Hubs: ";
            for(int hub : dp[i][j].visited_hubs) cout << hub << " ";
                cout << "\n";
        }
    }
    // initializeBruteFloydWarshall();
    
    State final_path = findOptimalPath(z, y);
    
    // Output path length and nodes
    cout << final_path.path.size() << "\n";
    for(int i = 0; i < final_path.path.size(); i++) {
        cout << final_path.path[i];
        if(i < final_path.path.size() - 1) cout << " ";
    }
    cout << "\n";
    
    // Output delivered houses information
    cout << "\nDelivered Houses: " << final_path.list_delivered_houses.size() << "/" << houses.size() << "\n";
    cout << "Houses delivered: ";
    for(int house : final_path.delivered_houses) {
        cout << house << " ";
    }
    cout << "\nHouses delivered: ";
    for(int hh: houses) {
        cout << hh << " ";
    }
    cout << "\n";
    cout << "Total fuel cost: " << final_path.fuel_costs[0] << "\n";
    cout << "Time taken: " 
         << chrono::duration_cast<chrono::seconds>(chrono::high_resolution_clock::now() - start).count() 
         << " s\n";
    return 0;
}