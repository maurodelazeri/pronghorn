#ifndef ALGO_GRAPH_CHILDREN_H
#define ALGO_GRAPH_CHILDREN_H

#include <iostream>
#include <vector>
#include <unordered_map>
#include <queue>
#include <set>

using ll = long long;
using namespace std;

#define INF ll(1ull<<60u)
using Node = unordered_map<ll, ll>;

class [[maybe_unused]] Graph_parent {
private:
    bool directed;
    ll nb_nodes;
    vector<Node> children;

private:
    bool distances_computed = false;
    vector<vector<ll>> all_distances = {};

    void reset_distances() {
        all_distances = {};
        distances_computed = false;
    }

public:
    /**
     * Creates an undirected graph with nb_nodes nodes
     * @param nb_nodes the number of nodes
     */
    [[maybe_unused]] explicit Graph_parent(ll nb_nodes) : directed(false), nb_nodes(nb_nodes),
                                                          children(vector<Node>(nb_nodes)) {}

    /**
     * Creates an empty graph
     * @param directed specifies if the graph is directed
     */
    [[maybe_unused]] explicit Graph_parent(bool directed) : directed(directed), nb_nodes(0),
                                                            children(vector<Node>(0)) {}

    /**
     * Creates a graph
     * @param directed specifies if the graph is directed
     * @param nb_nodes the number of nodes
     */
    [[maybe_unused]] explicit Graph_parent(bool directed, ll nb_nodes) : directed(directed), nb_nodes(nb_nodes),
                                                                         children(vector<Node>(nb_nodes, Node())) {}

    [[nodiscard]] ll size() const {
        return nb_nodes;
    }

    /**
     * Enlarge the graph
     */
    [[maybe_unused]] void add_node() {
        nb_nodes++;
        children.emplace_back();
        reset_distances();
    }

    /**
     * Get a node children
     * @param node the index of the requested node
     * @return a Node containing the children
     */
    [[maybe_unused]] [[nodiscard]] Node get_children(ll node) const {
        return children[node];
    }

    /**
     * Add a child to a node (distance 1), and reciprocally if undirected
     * @param parent the parent
     * @param child the child
     */
    [[maybe_unused]] void add_child(ll parent, ll child) {
        add_child(parent, child, 1);
        reset_distances();
    }

    /**
     * Add a child to a node, and reciprocally if undirected. If the child already exists, it keeps the shortest distance
     * @param parent the parent
     * @param child the child
     * @param dist the distance between the nodes
     */
    void add_child(ll parent, ll child, ll dist) {
        if (has_child(parent, child)) {
            children[parent][child] = min(children[parent][child], dist);
            if (!directed)
                children[child][parent] = min(children[child][parent], dist);
        } else {
            children[parent][child] = dist;
            if (!directed)
                children[child][parent] = dist;
        }
        reset_distances();
    }

    /**
     * Remove ALL vertices between two nodes
     * @param parent the parent
     * @param child the child
     */
    [[maybe_unused]] void remove_children(ll parent, ll child) {
        reset_distances();
        if (has_child(parent, child)) {
            children[parent].erase(child);
            if (!directed)
                children[child].erase(parent);
            return;
        }
    }

    /**
     * Finds if n1 has n2 as a direct child
     * @param n1 the first node
     * @param n2 the child
     * @return true if there are vertices between n1 and n2
     */
    [[nodiscard]] bool has_child(ll n1, ll n2) const {
        return children[n1].find(n2) != children[n1].end();
    }

    /**
     * Finds the distance between n1 and n2, INF if n2 is not a direct child of n1
     * @param n1 the first node
     * @param n2 the child
     * @return the distance between n1 and n2 if n2 is a child of n1, INF else.
     */
    [[nodiscard]] ll child_dist(ll n1, ll n2) const {
        if (has_child(n1, n2))
            return children[n1].at(n2);
        return INF;
    }

    /**
     * Finds via a DFS if two node are connected
     * @param n1 the first node
     * @param n2 the goal
     * @return true if n1 and n2 are connected, false else
     */
    [[maybe_unused]] [[nodiscard]] bool connected(ll n1, ll n2) const {
        vector<ll> ignored;
        return connected(n1, n2, ignored);
    }

    /**
     * Finds via a DFS if two node are connected
     * @param n1 the first node
     * @param n2 the goal
     * @param path the path from n1 to n2 (modified)
     * @return true if n1 and n2 are connected, false else
     */
    [[maybe_unused]] bool connected(ll n1, ll n2, vector<ll> &path) const {
        vector<bool> visited(nb_nodes, false);
        return path_dfs(n1, n2, visited, path);
    }

    /**
     * Find the connected components in an undirected graph
     * @return a vector of nodes (long), in which each node is in a different component
     */
    [[maybe_unused]] [[nodiscard]] vector<ll> find_connected_components() const {
        if (!directed) {
            vector<ll> components;
            vector<bool> visits(nb_nodes, false);
            for (ll i = 0; i < nb_nodes; i++) {
                if (!visits[i]) {
                    components.push_back(i);
                    visit_dfs(i, visits);
                }
            }
            return components;
        } else {
            cerr << "Not available for directed graphs, see strongly connected components" << endl;
            exit(100);
        }
    }

    /**
     * Determines if an undirected graph has cycles
     * @return true if the graph has a cycle, false else
     */
    [[maybe_unused]] [[nodiscard]] bool has_cycles() const {
        if (!directed) {
            vector<ll> components = find_connected_components();

            bool has_cycle = false;
            for (ll &c : components) {
                has_cycle = has_cycle || find_cycle_dfs(c);
            }
            return has_cycle;
        } else {
            cerr << "Not implemented yet" << endl;
            exit(100);
        }
    }

    /**
     * Return true if the graph can be bicolored, and a color distribution, or else false
     * @return a pair of the result and a vector of the colors
     */
    [[maybe_unused]] [[nodiscard]] pair<bool, vector<int>> is_bicolor() const {
        if (!directed) {
            bool is_bic = true;
            vector<int> colors(nb_nodes, -1);
            for (int i = 0; i < nb_nodes; i++) {
                if (colors[i] == -1) {
                    is_bic = is_bic && bicolor_dfs(i, 0, colors);
                }
            }
            return make_pair(is_bic, colors);
        } else {
            cerr << "Not available for directed graphs" << endl;
            exit(100);
        }
    }

    /**
     * Computes all distances from a node, and finds negative cycles (from this node)
     * @param start the first node
     * @return a vector of distances and parents node
     */
    [[maybe_unused]] [[nodiscard]] pair<set<ll>, vector<pair<ll, ll>>> bellman_ford(ll start) const {
        vector<pair<ll, ll>> distances(nb_nodes);
        for (ll i = 0; i < nb_nodes; i++) {
            distances[i] = {INF, i};
        }
        distances[start] = {0, start};
        bool changed = true;
        for (ll i = 0; changed && i < nb_nodes; i++) {
            changed = false;
            for (ll j = 0; j < nb_nodes; j++) {
                for (auto &arc: children[j]) {
                    pair<ll, ll> new_dist = {distances[j].first + arc.second, j};
                    pair<ll, ll> &old_dist = distances[arc.first];
                    if (new_dist.first < old_dist.first) {
                        old_dist = new_dist;
                        changed = true;
                    }
                }
            }
        }
        set<ll> negative_nodes;
        for (ll j = 0; j < nb_nodes; j++) {
            for (auto &arc: children[j]) {
                pair<ll, ll> new_dist = {distances[j].first + arc.second, j};
                pair<ll, ll> &old_dist = distances[arc.first];
                if (new_dist.first < old_dist.first) {
                    old_dist = new_dist;
                    negative_nodes.insert(j);
                }

            }
        }

        return {negative_nodes, distances};
    }

    [[maybe_unused]] static vector<ll> simple_bellman_ford(ll start, const vector<tuple<ll, ll, ll>> &arcs) {
        ll nb_nodes = arcs.size();
        vector<ll> distances(nb_nodes);
        for (ll i = 0; i < nb_nodes; i++) {
            distances[i] = INF;
        }
        distances[start] = 0;
        bool changed = true;
        ll a, b, w;
        for (ll i = 0; changed && i < nb_nodes; i++) {
            changed = false;
            for (auto &arc: arcs) {
                tie(a, b, w) = arc;
                if (distances[a] == INF) continue;
                ll new_dist = distances[a] + w;
                if (new_dist < distances[b]) {
                    distances[b] = new_dist;
                    changed = true;
                }

            }
        }
        return distances;
    }

    /**
     * Computes all distances from a node, only with positive weights
     * @param start the first node
     * @return a vector of distances and parents node
     */
    [[maybe_unused]] [[nodiscard]] vector<pair<ll, ll>> dijkstra(ll start) const {
        vector<pair<ll, ll>> distances(nb_nodes);
        vector<bool> visited(nb_nodes, false);
        for (ll i = 0; i < nb_nodes; i++) {
            distances[i] = {INF, i};
        }
        distances[start] = {0, start};
        priority_queue<pair<ll, ll>> q;
        q.push({0, start});
        while (!q.empty()) {
            int a = q.top().second;
            q.pop();
            if (visited[a]) continue;
            visited[a] = true;
            for (auto &child: children[a]) {
                ll b = child.first, w = child.second;
                pair<ll, ll> new_dist = {distances[a].first + w, a};
                pair<ll, ll> &old_dist = distances[b];
                if (new_dist.first < old_dist.first) {
                    old_dist = new_dist;
                    q.push({-new_dist.first, b});
                }
            }
        }
        return distances;
    }

    /**
     * Computes all distances from a node, only with positive weights
     * @param start the first node
     * @param k the k shortest paths
     * @return a vector of the k shortest distances
     */
    [[maybe_unused]] [[nodiscard]] vector<vector<ll>> kdijkstra(ll start, uint k) const {
        vector<vector<ll>> distances(nb_nodes);
        priority_queue<pair<ll, ll>> q;
        q.push({0, start});
        while (!q.empty()) {
            ll d = -q.top().first;
            ll a = q.top().second;
            q.pop();
            if (distances[a].size() >= k)
                continue;
            distances[a].push_back(d);
            for (const auto &child: children[a]) {
                const ll b = child.first, w = child.second;
                ll new_dist = d + w;
                q.push({-new_dist, b});
            }
        }
        return distances;
    }

    /**
     * classic BFS, but slower due to the data strucuture (actually a dijkstra with weights 1)
     * @param start the first node
     * @param end the goal
     * @return a vector of the computed distances and the parents node
     */
    [[maybe_unused]] [[nodiscard]] vector<pair<ll, ll>> bfs(ll start, ll end) const {
        vector<pair<ll, ll>> distances(nb_nodes);
        vector<bool> visited(nb_nodes, false);
        for (ll i = 0; i < nb_nodes; i++) {
            distances[i] = {INF, i};
        }
        distances[start] = {0, start};
        queue<pair<ll, ll>> q;
        q.push({0, start});
        while (!q.empty()) {
            int a = q.front().second;
            if (a == end) return distances;
            q.pop();
            if (visited[a]) continue;
            visited[a] = true;
            for (auto &child: children[a]) {
                ll b = child.first;
                pair<ll, ll> new_dist = {distances[a].first + 1, a};
                pair<ll, ll> &old_dist = distances[b];
                if (new_dist.first < old_dist.first) {
                    old_dist = new_dist;
                    q.push({-new_dist.first, b});
                }
            }
        }
        return distances;
    }

    [[maybe_unused]] vector<vector<ll>> get_all_distances() {
        if (!distances_computed) {
            all_distances = vector<vector<ll>>(nb_nodes, vector<ll>(nb_nodes));
            floyd_warshall();
            distances_computed = true;
        }

        return all_distances;
    }

private:
    bool path_dfs(ll start, ll end, vector<bool> &visited, vector<ll> &path) const {
        if (start == end) return true;
        visited[start] = true;
        for (auto &child: children[start]) {
            if (!visited[child.first]) {
                path.push_back(child.first);
                if (path_dfs(child.first, end, visited, path))
                    return true;
                path.pop_back();
            }
        }
        return false;
    }

    void visit_dfs(ll node, vector<bool> &visited) const {
        visited[node] = true;
        for (auto &child: children[node]) {
            if (!visited[child.first])
                visit_dfs(child.first, visited);
        }
    }

    [[nodiscard]] bool find_cycle_dfs(ll node) const {
        vector<bool> v(nb_nodes, false);
        return find_cycle_dfs(node, -1, v);
    }

    bool find_cycle_dfs(ll node, ll parent, vector<bool> &visited) const {
        visited[node] = true;
        bool found_cycle = false;
        for (auto &child: children[node]) {
            if (!visited[child.first])
                found_cycle = found_cycle || find_cycle_dfs(child.first, node, visited);
            else if (child.first != parent)
                return true;
        }
        return found_cycle;
    }

    bool bicolor_dfs(ll node, ll color, vector<int> &colors) const {
        colors[node] = color;
        bool is_bic = true;
        for (auto &child : children[node]) {
            ll c_col = colors[child.first];
            if (c_col == color) {
                return false;
            } else if (c_col == -1) {
                is_bic = is_bic && bicolor_dfs(child.first, 1 - color, colors);
            }
        }
        return is_bic;
    }

    void floyd_warshall() {
        // init
        for (ll i = 0; i < nb_nodes; i++) {
            for (ll j = 0; j < nb_nodes; j++) {
                if (i == j) {
                    all_distances[i][j] = 0;
                } else {
                    all_distances[i][j] = child_dist(i, j);
                }
            }
        }
        // real f-w
        for (ll k = 0; k < nb_nodes; k++)
            for (ll i = 0; i < nb_nodes; i++)
                for (ll j = 0; j < nb_nodes; j++)
                    all_distances[i][j] = min(all_distances[i][j], all_distances[i][k] + all_distances[k][j]);
    }
};

#endif //ALGO_GRAPH_CHILDREN_H