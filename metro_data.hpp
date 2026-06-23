#pragma once
#include <bits/stdc++.h>
#include "graph.hpp"
using namespace std;

// ============================================================
//  MetroData — Official Hyderabad Metro dataset
//
//  RED  Line : Miyapur ↔ L B Nagar         (27 stations)
//  BLUE Line : Raidurg ↔ Nagole            (23 stations)
//  GREEN Line: Parade Ground ↔ MG Bus Station (9 stations)
//
//  Interchanges (3 total):
//   • Ameerpet      : Red Line ↔ Blue Line
//   • Parade Ground : Blue Line ↔ Green Line
//     (also called "JBS Parade Ground" — same physical station)
//   • MG Bus Station: Red Line ↔ Green Line
//
//  Total unique stations: 27 + (23−1) + (9−2) = 56
// ============================================================
namespace MetroData {

    inline MetroGraph buildHyderabadMetroNetwork() {
        MetroGraph g;

        // ── RED LINE (27 stations, Miyapur → L B Nagar) ──────────
        // Distances: edge weight = km from the PREVIOUS station.
        vector<pair<string, double>> redLine = {
            {"Miyapur",                      0.0},
            {"JNTU College",                 1.2},
            {"KPHB Colony",                  1.4},
            {"Kukatpally",                   1.3},
            {"Dr. B. R. Ambedkar Balanagar", 1.5},
            {"Moosapet",                     1.0},
            {"Bharat Nagar",                 1.0},
            {"Erragadda",                    1.1},
            {"ESI Hospital",                 1.2},
            {"S.R. Nagar",                   0.8},
            {"Ameerpet",                     0.8},   // RED ↔ BLUE interchange
            {"Punjagutta",                   1.2},
            {"Irrum Manzil",                 1.0},
            {"Khairatabad",                  1.2},
            {"Lakdi-ka-pul",                 1.1},
            {"Assembly",                     1.0},
            {"Nampally",                     0.8},
            {"Gandhi Bhavan",                0.8},
            {"Osmania Medical College",      0.9},
            {"MG Bus Station",               0.8},   // RED ↔ GREEN interchange
            {"Malakpet",                     1.1},
            {"New Market",                   1.0},
            {"Musarambagh",                  1.1},
            {"Dilsukhnagar",                 1.5},
            {"Chaitanyapuri",                1.1},
            {"Victoria Memorial",            1.2},
            {"L B Nagar",                    1.0}
        };

        for (const auto& [name, _] : redLine)
            g.addStation(name, "Red Line");
        for (size_t i = 0; i + 1 < redLine.size(); ++i)
            g.addEdge(redLine[i].first, redLine[i+1].first, redLine[i+1].second);

        // ── BLUE LINE (23 stations, Raidurg → Nagole) ────────────
        vector<pair<string, double>> blueLine = {
            {"Raidurg",                  0.0},
            {"HITEC City",               1.5},
            {"Durgam Cheruvu",           1.1},
            {"Madhapur",                 1.3},
            {"Peddamma Gudi",            1.0},
            {"Jubilee Hills Check Post", 0.9},
            {"Road No.5 Jubilee Hills",  0.9},
            {"Yusufguda",                1.2},
            {"Madhura Nagar",            1.2},
            {"Ameerpet",                 0.9},   // BLUE ↔ RED interchange
            {"Begumpet",                 1.4},
            {"Prakash Nagar",            1.2},
            {"Rasoolpura",               1.1},
            {"Paradise",                 1.2},
            {"Parade Ground",            1.2},   // BLUE ↔ GREEN interchange
            {"Secunderabad East",        1.3},
            {"Mettuguda",                1.7},
            {"Tarnaka",                  1.4},
            {"Habsiguda",                1.3},
            {"NGRI",                     1.1},
            {"Stadium",                  1.1},
            {"Uppal",                    1.1},
            {"Nagole",                   1.7}
        };

        for (const auto& [name, _] : blueLine)
            g.addStation(name, "Blue Line");
        for (size_t i = 0; i + 1 < blueLine.size(); ++i)
            g.addEdge(blueLine[i].first, blueLine[i+1].first, blueLine[i+1].second);

        // ── GREEN LINE (9 stations, Parade Ground → MG Bus Station)
        // "JBS Parade Ground" and "Parade Ground" are the same physical
        // station — we use "Parade Ground" as the canonical name so it
        // merges correctly with the Blue Line node.
        vector<pair<string, double>> greenLine = {
            {"Parade Ground",     0.0},   // GREEN ↔ BLUE interchange
            {"Secunderabad West", 1.2},
            {"Gandhi Hospital",   1.3},
            {"Musheerabad",       1.1},
            {"RTC X Roads",       1.4},
            {"Chikkadpally",      1.0},
            {"Narayanaguda",      1.4},
            {"Sultan Bazaar",     1.8},
            {"MG Bus Station",    1.8}    // GREEN ↔ RED interchange
        };

        for (const auto& [name, _] : greenLine)
            g.addStation(name, "Green Line");
        for (size_t i = 0; i + 1 < greenLine.size(); ++i)
            g.addEdge(greenLine[i].first, greenLine[i+1].first, greenLine[i+1].second);

        // ── REGISTER LINES FOR MAP DISPLAY ───────────────────────
        auto stationNames = [](const vector<pair<string, double>>& line) {
            vector<string> ns;
            ns.reserve(line.size());
            for (const auto& [n, _] : line) ns.push_back(n);
            return ns;
        };

        g.registerLine("Red Line",   "RED",   stationNames(redLine));
        g.registerLine("Blue Line",  "BLUE",  stationNames(blueLine));
        g.registerLine("Green Line", "GREEN", stationNames(greenLine));

        return g;
    }

} // namespace MetroData
