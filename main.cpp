#include <bits/stdc++.h>
#include "graph.hpp"
#include "fare.hpp"
#include "metro_data.hpp"
using namespace std;

// ============================================================
//  UI Helpers
// ============================================================

static void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

static void printSeparator(const string& seg = "─", int width = 58) {
    cout << "  ";
    for (int i = 0; i < width; ++i) cout << seg;
    cout << "\n";
}

static void printHeader() {
    clearScreen();
    cout << "\n";
    cout << "  ╔══════════════════════════════════════════════════════╗\n";
    cout << "  ║      HYDERABAD METRO NAVIGATION ENGINE v1.0          ║\n";
    cout << "  ║        Red · Blue · Green Lines  |  56 Stations      ║\n";
    cout << "  ╚══════════════════════════════════════════════════════╝\n";
    cout << "\n";
}

static void printMenu() {
    cout << "  ┌──────────────────────────────────────────────────────┐\n";
    cout << "  │                     MAIN MENU                        │\n";
    cout << "  ├──────────────────────────────────────────────────────┤\n";
    cout << "  │  1.  Display Metro Map        (DFS Network View)     │\n";
    cout << "  │  2.  Shortest Path by Distance(Dijkstra + Fare)      │\n";
    cout << "  │  3.  Fewest Stops Route       (BFS)                  │\n";
    cout << "  │  4.  List All Stations                               │\n";
    cout << "  │  5.  Exit                                            │\n";
    cout << "  └──────────────────────────────────────────────────────┘\n";
    cout << "\n  Enter your choice (1-5): ";
}

// ============================================================
//  Station name prompt with basic validation
// ============================================================
static string promptStation(const MetroGraph& g, const string& label) {
    string name;
    while (true) {
        cout << "  " << label << ": ";
        getline(cin, name);

        // trim leading/trailing whitespace
        size_t start = name.find_first_not_of(" \t\r\n");
        size_t end   = name.find_last_not_of(" \t\r\n");
        if (start == string::npos) {
            cout << "  [!] Input cannot be empty. Try again.\n";
            continue;
        }
        name = name.substr(start, end - start + 1);

        if (g.hasStation(name)) return name;

        cout << "  [!] Station \"" << name << "\" not found.\n";
        cout << "      (Tip: use option 4 to list all stations with exact names)\n\n";
    }
}

// ============================================================
//  Prints a PathResult with per-station line annotations and
//  a prominent visual marker wherever the traveller must change
//  lines at an interchange station.
// ============================================================
static void printRoute(const MetroGraph& g,
                       const PathResult& res,
                       const string& algo) {
    if (!res.found) {
        cout << "\n  [✗] No route found between the specified stations.\n";
        return;
    }

    // Annotate the route with per-station line information
    auto annotations = g.annotateRoute(res.route);

    // Collect line-change events for the summary footer
    int lineChanges = 0;
    vector<pair<string,string>> changeEvents;  // (stationName, "From → To")
    for (const auto& ann : annotations) {
        if (ann.isLineChange) {
            ++lineChanges;
            changeEvents.push_back({ann.stationName,
                                    ann.fromLine + "  →  " + ann.currentLine});
        }
    }

    cout << "\n";
    printSeparator();
    cout << "  Route found via " << algo << ":\n";
    printSeparator();
    cout << "\n";

    for (size_t i = 0; i < annotations.size(); ++i) {
        const auto& ann  = annotations[i];
        bool isFirst = (i == 0);
        bool isLast  = (i == annotations.size() - 1);

        // Connector between stations
        if (!isFirst) cout << "        │\n";

        if (isFirst) {
            // ── Departure ──
            cout << "   🚉 START  →  "
                 << left << setw(32) << ann.stationName << right;
            if (!ann.currentLine.empty())
                cout << "  [" << ann.currentLine << "]";
            cout << "\n";

        } else if (isLast) {
            // ── Arrival ──
            cout << "  🏁 DEST   →  "
                 << left << setw(32) << ann.stationName << right;
            if (!ann.currentLine.empty())
                cout << "  [" << ann.currentLine << "]";
            cout << "\n";

        } else if (ann.isLineChange) {
            // ── Interchange: change lines here ──
            //  Show the station, then a prominent change banner
            cout << "  ○  "
                 << left << setw(34) << ann.stationName << right;
            if (!ann.mergedLineLabel.empty())
                cout << "  [" << ann.mergedLineLabel << "]";
            cout << "\n";
            cout << "        │\n";
            // Banner ─ fits inside 56-char separator width
            cout << "  ┌── CHANGE LINE ───────────────────────────────────────┐\n";
            cout << "  │  Alight : " << ann.fromLine    << "\n";
            cout << "  │  Board  : " << ann.currentLine << "\n";
            cout << "  └──────────────────────────────────────────────────────┘\n";

        } else {
            // ── Regular station ──
            cout << "         ·  "
                 << left << setw(32) << ann.stationName << right;
            if (!ann.currentLine.empty())
                cout << "  [" << ann.currentLine << "]";
            cout << "\n";
        }
    }

    // Footer summary
    cout << "\n";
    printSeparator();
    cout << "  Intermediate stops  : " << max(0, res.totalStops) << "\n";
    cout << "  Total distance      : "
         << fixed << setprecision(2) << res.totalDistanceKm << " km\n";
    if (lineChanges == 0) {
        cout << "  Line changes        : 0  (single-line direct route)\n";
    } else {
        cout << "  Line changes        : " << lineChanges << "\n";
        for (const auto& [stn, change] : changeEvents)
            cout << "    ○  " << stn << "  [" << change << "]\n";
    }
    printSeparator();
}

// ============================================================
//  Handler: DFS Metro Map
// ============================================================
static void handleDisplayMap(const MetroGraph& g) {
    printHeader();
    g.dfsPrintMetroMap();
    cout << "  Legend:  ★ = Interchange station\n\n";
    printSeparator();
    cout << "  Press ENTER to return to menu...";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

// ============================================================
//  Handler: Dijkstra Shortest Path + Fare
// ============================================================
static void handleShortestPath(const MetroGraph& g) {
    printHeader();
    cout << "  ── SHORTEST PATH BY DISTANCE (Dijkstra) ──\n\n";

    string src = promptStation(g, "Source Station     ");
    string dst = promptStation(g, "Destination Station");

    if (src == dst) {
        cout << "\n  [i] Source and destination are the same station.\n";
        cout << "  Press ENTER to return to menu...";
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return;
    }

    PathResult result = g.dijkstraShortestPath(src, dst);
    printRoute(g, result, "Dijkstra's Algorithm");

    if (result.found)
        FareCalculator::printFareBreakdown(result.totalDistanceKm);

    cout << "\n  Press ENTER to return to menu...";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

// ============================================================
//  Handler: BFS Fewest Stops
// ============================================================
static void handleFewestStops(const MetroGraph& g) {
    printHeader();
    cout << "  ── FEWEST STOPS ROUTE (BFS) ──\n\n";

    string src = promptStation(g, "Source Station     ");
    string dst = promptStation(g, "Destination Station");

    if (src == dst) {
        cout << "\n  [i] Source and destination are the same station.\n";
        cout << "  Press ENTER to return to menu...";
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return;
    }

    PathResult result = g.bfsFewestStops(src, dst);
    printRoute(g, result, "BFS (Fewest Stops)");

    cout << "\n  Press ENTER to return to menu...";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

// ============================================================
//  Handler: List All Stations
// ============================================================
static void handleListStations(const MetroGraph& g) {
    printHeader();
    cout << "  ── ALL STATIONS ──\n";
    g.listAllStations();
    cout << "\n";
    printSeparator();
    cout << "  Press ENTER to return to menu...";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

// ============================================================
//  main — CLI loop
// ============================================================
int main() {
    // Build the real Hyderabad Metro network
    MetroGraph metro = MetroData::buildHyderabadMetroNetwork();

    string choiceStr;
    bool running = true;

    while (running) {
        printHeader();
        printMenu();

        getline(cin, choiceStr);
        if (choiceStr.empty()) continue;
        char choice = choiceStr[0];

        switch (choice) {
            case '1': handleDisplayMap(metro);    break;
            case '2': handleShortestPath(metro);  break;
            case '3': handleFewestStops(metro);   break;
            case '4': handleListStations(metro);  break;
            case '5':
                printHeader();
                cout << "  Thank you for using Hyderabad Metro Navigation Engine.\n";
                cout << "  Goodbye!\n\n";
                running = false;
                break;
            default:
                cout << "\n  [!] Invalid choice. Please enter 1–5.\n";
                cout << "  Press ENTER to continue...";
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                break;
        }
    }

    return 0;
}
