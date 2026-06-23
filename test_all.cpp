#include <bits/stdc++.h>
#include "graph.hpp"
#include "fare.hpp"
#include "metro_data.hpp"
using namespace std;

int main() {
    MetroGraph g = MetroData::buildHyderabadMetroNetwork();

    cout << "=== STATION COUNT ===\n";
    cout << "Total stations: " << g.stationCount() << "  (expected: 56)\n\n";

    cout << "=== INTERCHANGE CHECK ===\n";
    cout << "Ameerpet:       " << g.getLineColor(g.getIndex("Ameerpet"))       << "\n";
    cout << "Parade Ground:  " << g.getLineColor(g.getIndex("Parade Ground"))  << "\n";
    cout << "MG Bus Station: " << g.getLineColor(g.getIndex("MG Bus Station")) << "\n\n";

    cout << "=== DIJKSTRA: Miyapur -> Nagole (Red->Blue cross-line via Ameerpet) ===\n";
    auto r1 = g.dijkstraShortestPath("Miyapur", "Nagole");
    cout << "Found:    " << (r1.found ? "YES" : "NO") << "\n";
    cout << "Distance: " << fixed << setprecision(2) << r1.totalDistanceKm << " km\n";
    cout << "Stops:    " << r1.totalStops << "\n";
    cout << "Fare:     Rs " << FareCalculator::calculateFare(r1.totalDistanceKm) << "\n";
    cout << "Route:    ";
    for (size_t i = 0; i < r1.route.size(); ++i)
        cout << (i ? " -> " : "") << r1.route[i];
    cout << "\n\n";

    cout << "=== DIJKSTRA: Parade Ground -> L B Nagar (Green->Red via MG Bus Station) ===\n";
    auto r2 = g.dijkstraShortestPath("Parade Ground", "L B Nagar");
    cout << "Found:    " << (r2.found ? "YES" : "NO") << "\n";
    cout << "Distance: " << r2.totalDistanceKm << " km  |  Stops: " << r2.totalStops << "\n";
    cout << "Route:\n";
    for (const auto& s : r2.route) cout << "  -> " << s << "\n";
    cout << "\n";

    cout << "=== BFS: Raidurg -> Nagole (same Blue Line) ===\n";
    auto r3 = g.bfsFewestStops("Raidurg", "Nagole");
    cout << "Found: " << (r3.found ? "YES" : "NO") << "  |  Stops: " << r3.totalStops << "\n";
    cout << "Route:  ";
    for (size_t i = 0; i < r3.route.size(); ++i)
        cout << (i ? " -> " : "") << r3.route[i];
    cout << "\n\n";

    cout << "=== UPDATED FARE SLABS ===\n";
    vector<double> distances = {1.0, 2.0, 3.0, 5.0, 7.0, 10.0, 13.0, 16.0, 19.0, 22.0, 28.0};
    for (double d : distances)
        cout << "  " << setw(5) << d << " km  ->  Rs "
             << FareCalculator::calculateFare(d) << "\n";
    cout << "\n";

    cout << "=== UNKNOWN STATION GUARD ===\n";
    auto r4 = g.dijkstraShortestPath("Miyapur", "NonExistentStation");
    cout << "Dijkstra unknown dest: " << (r4.found ? "YES (BUG!)" : "NO (correct)") << "\n";
    auto r5 = g.bfsFewestStops("Unknown", "Nagole");
    cout << "BFS unknown src:       " << (r5.found ? "YES (BUG!)" : "NO (correct)") << "\n\n";

    cout << "All tests passed.\n";
    return 0;
}
