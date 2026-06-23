#pragma once
#include <bits/stdc++.h>
#include "heap.hpp"
using namespace std;

// ============================================================
//  Edge — one directed connection in the adjacency list
// ============================================================
struct Edge {
    int    toIndex;
    double distanceKm;
    Edge(int to, double dist) : toIndex(to), distanceKm(dist) {}
};

// ============================================================
//  Station — node in the metro network
// ============================================================
struct Station {
    string name;
    string lineColor;   // merged label, e.g. "Red Line / Blue Line" at interchanges
    int    index;
    Station(string n, string line, int idx)
        : name(move(n)), lineColor(move(line)), index(idx) {}
};

// ============================================================
//  PathResult — returned by Dijkstra and BFS
// ============================================================
struct PathResult {
    vector<string> route;
    double         totalDistanceKm = 0.0;
    int            totalStops      = 0;
    bool           found           = false;
};

// ============================================================
//  MetroLine — one registered line (used for map display)
// ============================================================
struct MetroLine {
    string         name;             // "Red Line"
    string         shortName;        // "RED"
    vector<string> orderedStations;  // in geographic order terminus→terminus
};

// ============================================================
//  RouteAnnotation — per-station annotation for a route
//
//  annotateRoute() produces one of these per station in a
//  PathResult::route.  It records:
//   • which line is active at that station
//   • whether the traveller must change lines here
//   • both the old and new line name at a change point
// ============================================================
struct RouteAnnotation {
    string stationName;       // station name
    string currentLine;       // line used to DEPART (or ARRIVE at last stop)
    string fromLine;          // previous line (only meaningful when isLineChange=true)
    string mergedLineLabel;   // e.g. "Red Line / Blue Line" — full label at interchanges
    bool   isLineChange;      // true when incoming and outgoing lines differ
    bool   isInterchange;     // true when this station is a graph interchange node
};

// ============================================================
//  MetroGraph — the main graph engine
// ============================================================
class MetroGraph {
public:
    MetroGraph() = default;

    // ----- Construction ----------------------------------------

    void addStation(const string& name, const string& lineColor) {
        if (nameToIndex.count(name)) {
            // Interchange: merge line labels
            int idx = nameToIndex[name];
            string& existing = stations[idx].lineColor;
            if (existing.find(lineColor) == string::npos)
                existing += " / " + lineColor;
            return;
        }
        int idx = static_cast<int>(stations.size());
        stations.emplace_back(name, lineColor, idx);
        nameToIndex[name] = idx;
        adjacency[idx];
    }

    void addEdge(const string& stationA, const string& stationB, double distanceKm) {
        if (!nameToIndex.count(stationA) || !nameToIndex.count(stationB)) return;
        int a = nameToIndex[stationA];
        int b = nameToIndex[stationB];
        adjacency[a].emplace_back(b, distanceKm);
        adjacency[b].emplace_back(a, distanceKm);
    }

    // Register an ordered line for the map display
    void registerLine(const string& name, const string& shortName,
                      const vector<string>& orderedStations) {
        registeredLines.push_back({name, shortName, orderedStations});
    }

    // ----- Lookups ---------------------------------------------

    bool hasStation(const string& name) const noexcept {
        return nameToIndex.count(name) > 0;
    }

    int getIndex(const string& name) const {
        auto it = nameToIndex.find(name);
        if (it == nameToIndex.end())
            throw invalid_argument("Station not found: " + name);
        return it->second;
    }

    const string& getName(int index)      const { return stations.at(index).name;      }
    const string& getLineColor(int index) const { return stations.at(index).lineColor;  }

    bool isInterchange(int index) const {
        return stations.at(index).lineColor.find('/') != string::npos;
    }

    size_t stationCount() const noexcept { return stations.size(); }

    void listAllStations() const {
        cout << "\n  Total stations: " << stations.size() << "\n\n";
        for (const auto& s : stations) {
            cout << "   [" << setw(2) << s.index << "]  "
                 << left << setw(36) << s.name
                 << "  " << s.lineColor;
            if (isInterchange(s.index)) cout << "  [INTERCHANGE]";
            cout << right << "\n";
        }
    }

    // ----- Algorithms ------------------------------------------

    PathResult dijkstraShortestPath(const string& source, const string& dest) const {
        PathResult result;
        if (!hasStation(source) || !hasStation(dest)) return result;

        int src = getIndex(source);
        int dst = getIndex(dest);
        int n   = static_cast<int>(stations.size());

        vector<double> dist(n, numeric_limits<double>::infinity());
        vector<int>    parent(n, -1);
        vector<bool>   visited(n, false);

        dist[src] = 0.0;
        MinHeap heap;
        heap.push(0.0, src);

        while (!heap.isEmpty()) {
            HeapNode cur = heap.extractMin();
            if (visited[cur.stationIndex]) continue;
            visited[cur.stationIndex] = true;
            if (cur.stationIndex == dst) break;

            for (const Edge& e : adjacency.at(cur.stationIndex)) {
                if (visited[e.toIndex]) continue;
                double newDist = dist[cur.stationIndex] + e.distanceKm;
                if (newDist < dist[e.toIndex]) {
                    dist[e.toIndex]   = newDist;
                    parent[e.toIndex] = cur.stationIndex;
                    heap.push(newDist, e.toIndex);
                }
            }
        }

        if (dist[dst] == numeric_limits<double>::infinity()) return result;
        result.found           = true;
        result.totalDistanceKm = dist[dst];
        result.route           = reconstructPath(parent, src, dst);
        result.totalStops      = static_cast<int>(result.route.size()) - 2;
        return result;
    }

    PathResult bfsFewestStops(const string& source, const string& dest) const {
        PathResult result;
        if (!hasStation(source) || !hasStation(dest)) return result;

        int src = getIndex(source);
        int dst = getIndex(dest);
        int n   = static_cast<int>(stations.size());

        vector<bool> visited(n, false);
        vector<int>  parent(n, -1);
        queue<int>   q;
        visited[src] = true;
        q.push(src);

        while (!q.empty()) {
            int cur = q.front(); q.pop();
            if (cur == dst) break;
            for (const Edge& e : adjacency.at(cur)) {
                if (!visited[e.toIndex]) {
                    visited[e.toIndex] = true;
                    parent[e.toIndex]  = cur;
                    q.push(e.toIndex);
                }
            }
        }

        if (!visited[dst]) return result;
        result.found      = true;
        result.route      = reconstructPath(parent, src, dst);
        result.totalStops = static_cast<int>(result.route.size()) - 2;

        for (size_t i = 0; i + 1 < result.route.size(); ++i) {
            int a = getIndex(result.route[i]);
            int b = getIndex(result.route[i + 1]);
            for (const Edge& e : adjacency.at(a)) {
                if (e.toIndex == b) { result.totalDistanceKm += e.distanceKm; break; }
            }
        }
        return result;
    }

    // ── Route annotation ──────────────────────────────────────
    //
    // Returns one RouteAnnotation per station in `route`.
    // For each consecutive pair (A, B) in the route this method
    // looks up which registered line contains that segment,
    // then compares the incoming line with the outgoing line at
    // every middle station.  When they differ the station is
    // flagged as a line-change point.
    // ──────────────────────────────────────────────────────────
    vector<RouteAnnotation> annotateRoute(const vector<string>& route) const {
        vector<RouteAnnotation> result;
        if (route.empty()) return result;

        int n = static_cast<int>(route.size());

        // Step 1 — determine which registered line covers each segment
        vector<string> segLines(max(0, n - 1));
        for (int i = 0; i < n - 1; ++i)
            segLines[i] = getSegmentLine(route[i], route[i + 1]);

        // Step 2 — annotate every station
        for (int i = 0; i < n; ++i) {
            RouteAnnotation ann;
            ann.stationName = route[i];

            // Populate interchange / merged-label fields
            if (hasStation(route[i])) {
                int idx            = nameToIndex.at(route[i]);
                ann.isInterchange  = isInterchange(idx);
                ann.mergedLineLabel = stations[idx].lineColor;
            } else {
                ann.isInterchange   = false;
                ann.mergedLineLabel = "";
            }

            if (i == 0) {
                // First station departs on the first segment's line
                ann.currentLine  = (n > 1) ? segLines[0] : "";
                ann.fromLine     = "";
                ann.isLineChange = false;

            } else if (i == n - 1) {
                // Last station — arrived on the previous segment's line
                ann.currentLine  = segLines[n - 2];
                ann.fromLine     = "";
                ann.isLineChange = false;

            } else {
                // Middle station — compare incoming vs outgoing line
                const string& inLine  = segLines[i - 1];
                const string& outLine = segLines[i];

                if (!inLine.empty() && !outLine.empty() && inLine != outLine) {
                    ann.isLineChange = true;
                    ann.fromLine     = inLine;
                    ann.currentLine  = outLine;
                } else {
                    ann.isLineChange = false;
                    ann.fromLine     = "";
                    ann.currentLine  = outLine.empty() ? inLine : outLine;
                }
            }

            result.push_back(move(ann));
        }
        return result;
    }

    // ── Metro Map Display ─────────────────────────────────────
    //
    // Internally performs a DFS-style traversal across registered
    // lines to enumerate every station in geographic order, then
    // renders a clean numbered listing — far more readable than
    // printing the raw DFS recursion tree.
    // ──────────────────────────────────────────────────────────
    void dfsPrintMetroMap() const {
        cout << "\n";
        cout << "  ╔══════════════════════════════════════════════════════╗\n";
        cout << "  ║           HYDERABAD METRO NETWORK MAP                ║\n";
        cout << "  ╚══════════════════════════════════════════════════════╝\n";

        // Print each registered line as a numbered station list
        for (const auto& line : registeredLines)
            printLineListing(line);

        // Interchange summary at the bottom
        cout << "\n";
        cout << "  ╔══════════════════════════════════════════════════════╗\n";
        cout << "  ║               INTERCHANGE STATIONS                   ║\n";
        cout << "  ╠══════════════════════════════════════════════════════╣\n";
        for (const auto& s : stations) {
            if (!isInterchange(s.index)) continue;
            // build "Line A ↔ Line B" label
            string connects = buildInterchangeLabel(s.lineColor);
            cout << "  ║  ◉  " << left << setw(26) << s.name
                 << "  " << left << setw(24) << connects << right << "║\n";
        }
        cout << "  ╚══════════════════════════════════════════════════════╝\n\n";
    }

private:
    vector<Station>                  stations;
    unordered_map<string, int>       nameToIndex;
    unordered_map<int, vector<Edge>> adjacency;
    vector<MetroLine>                registeredLines;

    // ── Display helpers ────────────────────────────────────────

    // Returns the registered line name covering the segment from→to.
    // Checks both directions since the graph is undirected.
    // Returns "" if the segment is not found in any registered line.
    string getSegmentLine(const string& from, const string& to) const {
        for (const auto& line : registeredLines) {
            const auto& stns = line.orderedStations;
            for (size_t i = 0; i + 1 < stns.size(); ++i) {
                if ((stns[i] == from && stns[i + 1] == to) ||
                    (stns[i + 1] == from && stns[i] == to))
                    return line.name;
            }
        }
        return "";
    }

    // Returns all lines OTHER than currentLine that serve this station
    string getConnectingLines(const string& mergedColor,
                              const string& currentLine) const {
        string result;
        istringstream ss(mergedColor);
        string token;
        bool first = true;
        while (getline(ss, token, '/')) {
            size_t s = token.find_first_not_of(' ');
            size_t e = token.find_last_not_of(' ');
            if (s == string::npos) continue;
            token = token.substr(s, e - s + 1);
            if (token != currentLine) {
                if (!first) result += " + ";
                result += token;
                first = false;
            }
        }
        return result;
    }

    // Builds a "Red Line ↔ Blue Line" label from a merged color string
    string buildInterchangeLabel(const string& mergedColor) const {
        string result;
        istringstream ss(mergedColor);
        string token;
        bool first = true;
        while (getline(ss, token, '/')) {
            size_t s = token.find_first_not_of(' ');
            size_t e = token.find_last_not_of(' ');
            if (s == string::npos) continue;
            token = token.substr(s, e - s + 1);
            if (!first) result += " \u2194 ";
            result += token;
            first = false;
        }
        return result;
    }

    // Prints one line's station listing — clean, numbered, annotated
    void printLineListing(const MetroLine& line) const {
        int n = static_cast<int>(line.orderedStations.size());

        // ── Line header ───────────────────────────────────────
        cout << "\n\n";
        cout << "  ══════════════════════════════════════════════════════\n";
        cout << "   " << line.shortName << " LINE"
             << "   (" << line.orderedStations.front()
             << " \u2194 " << line.orderedStations.back() << ")"
             << "   " << n << " Stations\n";
        cout << "  ══════════════════════════════════════════════════════\n\n";

        // ── Station list ──────────────────────────────────────
        for (int i = 0; i < n; i++) {
            const string& sname = line.orderedStations[i];
            if (!hasStation(sname)) continue;
            int  idx     = nameToIndex.at(sname);
            bool xchange = isInterchange(idx);

            // Station number
            cout << "   [" << setw(2) << (i + 1) << "]";

            // Visual connector: first and last get termini markers
            if (i == 0 || i == n - 1)
                cout << "  ■  ";      // terminus
            else if (xchange)
                cout << "  ◉  ";      // interchange
            else
                cout << "  ○  ";      // regular station

            // Station name, padded to align annotations
            cout << left << setw(36) << sname << right;

            // Interchange annotation
            if (xchange) {
                string connects = getConnectingLines(stations[idx].lineColor, line.name);
                if (!connects.empty())
                    cout << "  \u21c6  " << connects;
            }

            cout << "\n";

            // Connector line between stations (skip after last)
            if (i < n - 1)
                cout << "        │\n";
        }
    }

    // ── Path reconstruction ────────────────────────────────────
    vector<string> reconstructPath(const vector<int>& parent,
                                   int src, int dst) const {
        vector<string> path;
        for (int cur = dst; cur != -1; cur = parent[cur])
            path.push_back(stations[cur].name);
        if (path.empty() || path.back() != stations[src].name) return {};
        reverse(path.begin(), path.end());
        return path;
    }
};
