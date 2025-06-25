#include <bits/stdc++.h>
using namespace std;

void check() {
    // Read graph & delivery data from file
    ifstream fin("../project_general_inputs/input_2.txt");
    if (!fin) {
        cerr << "Error: cannot open input.txt\n";
        return;
    }
    int N, T, M, K, F;
    fin >> N >> T >> M >> K >> F;

    vector<int> hubs(N), houses(N), stations(K);
    for (int i = 0; i < N; i++) fin >> hubs[i];
    for (int i = 0; i < N; i++) fin >> houses[i];
    for (int i = 0; i < K; i++) fin >> stations[i];

    vector<vector<pair<int,int>>> adj(T);
    for (int i = 0, u, v, c; i < M; i++) {
        fin >> u >> v >> c;
        adj[u].push_back({v, c});
        adj[v].push_back({u, c});
    }
    fin.close();

    // Read proposed path from stdin
    int P;
    cin >> P;
    vector<int> path(P);
    for (int i = 0; i < P; i++) {
        cin >> path[i];
    }

    // Prepare lookup sets/maps
    unordered_set<int> stationSet(stations.begin(), stations.end());
    unordered_map<int,int> house_to_hub;
    for (int i = 0; i < N; i++) {
        house_to_hub[houses[i]] = hubs[i];
    }

    // 1. Check adjacency & fuel constraints
    int fuel = F;
    for (int i = 0; i + 1 < P; i++) {
        int u = path[i], v = path[i+1];
        bool edgeFound = false;
        int cost = 0;
        for (auto &e : adj[u]) {
            if (e.first == v) {
                edgeFound = true;
                cost = e.second;
                break;
            }
        }
        if (!edgeFound) {
            cout << "Invalid: no edge between " << u << " and " << v << "\n";
            return;
        }
        fuel -= cost;
        if (fuel < 0) {
            cout << "Invalid: ran out of fuel going from " << u << " to " << v << "\n";
            return;
        }
        if (stationSet.count(v)) {
            fuel = F;  // refuel
        }
    }

    // 2. Check hub visits and delivered houses
    unordered_set<int> hubSet(hubs.begin(), hubs.end());
    vector<bool> hubVisited(T, false);
    unordered_set<int> delivered;
    for (int node : path) {
        if (hubSet.count(node)) {
            hubVisited[node] = true;
        }
        auto itH = house_to_hub.find(node);
        if (itH != house_to_hub.end()) {
            int reqHub = itH->second;
            if (hubVisited[reqHub]) {
                delivered.insert(node);
            }
        }
    }
    // report delivered count
    cout << "Delivered houses: " << delivered.size() << "/" << houses.size() << "\n";
    if (delivered.size() != houses.size()) {
        cout << "Invalid: not all houses delivered\n";
        // List undelivered houses
        vector<int> undelivered;
        for (int h : houses) {
            if (!delivered.count(h)) {
                undelivered.push_back(h);
            }
        }
        cout << "Undelivered houses:";
        for (int h : undelivered) cout << " " << h << " " << house_to_hub[h];
        cout << "\n";
        return;
    }

    cout << "Valid solution\n";
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    check();
    return 0;
}