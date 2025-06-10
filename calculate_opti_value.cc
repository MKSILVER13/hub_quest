#include <bits/stdc++.h>
using namespace std;

struct State {
    vector<int> path;
    set<int> visited_hubs;
    set<int> delivered_houses;
    array<int, 3> fuel_costs; // {total, initial, last}
    bool has_fuel_station = false;

    State() : fuel_costs({0, 0, 0}) {}
};

map<int, int> nodeType; // 1: fuel station, 2: hub, 3: house
map<int, int> houseToHub; // maps house to its corresponding hub
vector<vector<pair<int, int>>> adj; // adjacency list: {node, fuel_cost}
int numHouses = 0;  // global to hold number of houses

// Function to calculate optimization value
double calculateOptiValue(const State& state, int x) {
    int delivered = state.delivered_houses.size();
    int visited = state.visited_hubs.size();
    int cost = state.fuel_costs[0] + state.fuel_costs[1] + state.fuel_costs[2];
    set<int> unique_nodes(state.path.begin(), state.path.end());
    int revisit_penalty = state.path.size() - unique_nodes.size(); // Penalize revisits

    if (delivered > x) {
        return cost * (double(visited) / delivered) + revisit_penalty * 10;
    } else if (visited > 0) {
        return double(cost) / visited + revisit_penalty * 10;
    } else {
        return cost + revisit_penalty * 10; // Penalize revisits heavily
    }
}

void readGraph(const string& filename) {
    ifstream infile(filename);
    int N, T, M, K, F;
    infile >> N >> T >> M >> K >> F;

    numHouses = N;  // store house count
    adj.resize(T); // Adjust size to handle nodes starting from 0

    for (int i = 0; i < N; i++) {
        int hub;
        infile >> hub;
        nodeType[hub] = 2; // Mark as hub
    }

    for (int i = 0; i < N; i++) {
        int house;
        infile >> house;
        nodeType[house] = 3; // Mark as house
        int hub;
        infile >> hub;
        houseToHub[house] = hub;
    }

    for (int i = 0; i < K; i++) {
        int station;
        infile >> station;
        nodeType[station] = 1; // Mark as fuel station
    }

    for (int i = 0; i < M; i++) {
        int u, v, w;
        infile >> u >> v >> w;
        adj[u].push_back({v, w});
        adj[v].push_back({u, w}); // Assuming undirected graph
    }

    infile.close();
}

void calculateFuelCosts(State& state) {
    const int INF = INT_MAX;
    int total_cost = 0;
    int first_fuel_station_cost = INF;
    int last_fuel_station_cost = INF;

    // Calculate total cost and cost to first fuel station
    int current_cost = 0;
    for (int i = 0; i < state.path.size(); i++) {
        int node = state.path[i];
        if (i > 0) {
            int prev_node = state.path[i - 1];
            for (const auto& edge : adj[prev_node]) {
                if (edge.first == node) {
                    current_cost += edge.second;
                    break;
                }
            }
        }
        if (nodeType[node] == 1 && first_fuel_station_cost == INF) {
            first_fuel_station_cost = current_cost;
        }
    }

    // Calculate cost to last fuel station
    current_cost = 0;
    for (int i = state.path.size() - 1; i >= 0; i--) {
        int node = state.path[i];
        if (i < state.path.size() - 1) {
            int next_node = state.path[i + 1];
            for (const auto& edge : adj[node]) {
                if (edge.first == next_node) {
                    current_cost += edge.second;
                    break;
                }
            }
        }
        if (nodeType[node] == 1 && last_fuel_station_cost == INF) {
            last_fuel_station_cost = current_cost;
        }
    }

    total_cost = first_fuel_station_cost + last_fuel_station_cost;
    state.fuel_costs = {total_cost, first_fuel_station_cost, last_fuel_station_cost};
}

// Update processPaths to calculate fuel costs dynamically
void processPaths(const string& filename, int x) {
    ifstream infile(filename);
    int t;
    infile >> t;

    for (int i = 0; i < t; i++) {
        int n;
        infile >> n;
        State state;
        state.path.resize(n);

        for (int j = 0; j < n; j++) {
            infile >> state.path[j];
            int node = state.path[j];

            if (nodeType[node] == 2) {
                state.visited_hubs.insert(node);
            } else if (nodeType[node] == 3) {
                int hub = houseToHub[node];
                if (state.visited_hubs.find(hub) != state.visited_hubs.end()) {
                    state.delivered_houses.insert(node);
                }
            }
        }

        // Calculate fuel costs dynamically
        calculateFuelCosts(state);

        // Detailed metrics for analysis
        int delivered = state.delivered_houses.size();
        int visited = state.visited_hubs.size();
        set<int> unique_nodes(state.path.begin(), state.path.end());
        int revisit_penalty = state.path.size() - unique_nodes.size();
        int total_cost = state.fuel_costs[0];
        int initial_cost = state.fuel_costs[1];
        int last_cost = state.fuel_costs[2];

        cout << "Path: ";
        for (int node : state.path) {
            cout << node << " ";
        }
        cout << "\nVisited Hubs: " << visited
             << ", Delivered Houses: " << delivered
             << "\nFuel Costs (Total, Initial, Last): "
             << total_cost << ", " << initial_cost << ", " << last_cost
             << "\nRevisit Penalty: " << revisit_penalty << "\n";

        double opti_value = calculateOptiValue(state, x);
        cout << "OptiValue: " << opti_value << "\n\n";
    }

    infile.close();
}

int main() {
    string graph_file = "graph_input.txt";
    string paths_file = "paths_input.txt";

    readGraph(graph_file);

    // Redirect all output to analysis_output.txt
    freopen("analysis_output.txt", "w", stdout);

    for (int x = 0; x <= numHouses; ++x) {
        cout << "=== x = " << x << " ===\n";
        processPaths(paths_file, x);
    }

    return 0;
}
