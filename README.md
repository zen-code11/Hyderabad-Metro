# Hyderabad Metro Navigation Engine

A C++ console application that models the real Hyderabad Metro network (56 stations, 3 lines, 3 interchanges) as a weighted graph and provides shortest-path navigation, fare calculation, and route visualization — built using custom data structures rather than off-the-shelf STL containers.

---

## Overview

The Hyderabad Metro Navigation Engine is a graph-based pathfinding system built to demonstrate core data structures and algorithms on a real-world transportation network. It models the **Red, Blue, and Green** lines of the Hyderabad Metro (56 unique stations, including the Ameerpet, Parade Ground, and MG Bus Station interchanges) as an undirected, weighted graph, and answers two distinct navigation questions:

- *"What's the shortest route by distance?"* → solved with **Dijkstra's algorithm**, using a hand-rolled binary min-heap.
- *"What's the route with the fewest stops?"* → solved with **BFS**.

On top of pathfinding, the engine annotates routes with line-change information (telling the traveller exactly where to alight and board a new line) and calculates the official TSMRL fare for the journey.

---

## Demo / Screenshots

```
╔══════════════════════════════════════════════════════╗
║      HYDERABAD METRO NAVIGATION ENGINE v1.0          ║
║        Red · Blue · Green Lines  |  56 Stations      ║
╚══════════════════════════════════════════════════════╝

┌──────────────────────────────────────────────────────┐
│                     MAIN MENU                        │
├──────────────────────────────────────────────────────┤
│  1.  Display Metro Map        (DFS Network View)     │
│  2.  Shortest Path by Distance(Dijkstra + Fare)       │
│  3.  Fewest Stops Route       (BFS)                   │
│  4.  List All Stations                                │
│  5.  Exit                                             │
└──────────────────────────────────────────────────────┘
```

*(Replace this block with an actual terminal screenshot or GIF — recruiters respond well to visuals. A `screenshots/` folder with a `route_example.png` and `map_example.png` is recommended.)*

---

## Features

-  **Full network model** — 56 real Hyderabad Metro stations across 3 lines, with accurate inter-station distances.
-  **Dijkstra-based shortest-distance routing** using a custom binary min-heap (no `std::priority_queue`).
-  **BFS-based fewest-stops routing** for unweighted shortest paths.
-  **Automatic interchange detection** — stations served by multiple lines are merged into a single graph node with a combined line label.
-  **Route annotation engine** — tells the user exactly where to change lines, from which line to which.
-  **Official fare calculation** — maps total distance to TSMRL's published fare slabs.
-  **Interactive CLI** with input validation, formatted ASCII route diagrams, and a full network map view.
-  **Self-contained test suite** (`test_all.cpp`) covering station counts, interchange resolution, cross-line routing, and invalid-input guards.

---

## System Architecture

### Module Diagram

```
┌─────────────────┐
│   main.cpp       │  CLI loop, menus, input validation, output formatting
└────────┬─────────┘
         │ uses
         ▼
┌─────────────────┐     ┌──────────────────┐
│  metro_data.hpp  │────▶│   graph.hpp       │  MetroGraph engine:
│  (station/line    │     │   - Station/Edge  │  - addStation/addEdge
│   dataset)        │     │   - PathResult    │  - dijkstraShortestPath()
└─────────────────┘     │   - MetroGraph    │  - bfsFewestStops()
                          └────────┬──────────┘  - annotateRoute()
                                   │ uses          - dfsPrintMetroMap()
                                   ▼
                          ┌──────────────────┐
                          │   heap.hpp        │  Custom binary MinHeap
                          │   - HeapNode      │  used by Dijkstra
                          │   - MinHeap       │
                          └──────────────────┘

┌─────────────────┐
│   fare.hpp       │  Stateless fare-slab calculator, used by main.cpp
└─────────────────┘
```

| File | Responsibility |
|---|---|
| `graph.hpp` | Core engine — adjacency-list graph, Dijkstra, BFS, route annotation, map rendering |
| `heap.hpp` | Custom binary min-heap used as Dijkstra's priority queue |
| `fare.hpp` | Fare calculation against official distance-slab table |
| `metro_data.hpp` | Hardcoded dataset — Red/Blue/Green line stations, distances, interchanges |
| `main.cpp` | CLI entry point — menu loop, input handling, output rendering |
| `test_all.cpp` | Verification suite for graph construction and algorithm correctness |

### Data Flow

```
User input (menu choice + station names)
        │
        ▼
Input validation (promptStation) ──▶ rejects unknown stations, re-prompts
        │
        ▼
MetroGraph::dijkstraShortestPath()  or  bfsFewestStops()
        │
        ▼
PathResult { route, totalDistanceKm, totalStops, found }
        │
        ▼
MetroGraph::annotateRoute()  ──▶  per-station line + interchange annotations
        │
        ▼
printRoute() in main.cpp  ──▶  formatted ASCII journey view
        │
        ▼
FareCalculator::printFareBreakdown()  ──▶  fare box (Dijkstra mode only)
```

---

## Algorithms & Data Structures

### Custom Min-Heap (`heap.hpp`)

Implemented from scratch as a `vector`-backed binary heap with `heapifyUp`/`heapifyDown`, rather than using `std::priority_queue`. This was a deliberate choice to:
- Demonstrate manual heap mechanics (parent/child index math, sift-up/sift-down) rather than relying on a library black box.
- Allow full control over the node payload (`HeapNode{distance, stationIndex}`) without needing custom comparators or wrapper structs.

**Trade-off note:** This heap does **not** support `decrease-key`. Instead, Dijkstra uses the standard *lazy deletion* pattern — stale, outdated entries are simply pushed again with a better distance, and a `visited[]` array skips any node popped after it's already been finalized. This is the conventional approach when using a binary heap without an index-tracking structure, and keeps the implementation simple while remaining `O(log n)` per push/pop.

### Dijkstra's Algorithm (Lazy Deletion)

Used for shortest-distance routing (`dijkstraShortestPath`):
1. Initialize `dist[]` to infinity, `dist[source] = 0`.
2. Push `(0, source)` onto the min-heap.
3. Repeatedly extract the minimum-distance node; skip if already visited.
4. Relax all outgoing edges; push improved distances onto the heap (old, stale entries are left in place and ignored later via `visited[]`).
5. Stop early once the destination is popped.
6. Reconstruct the path via parent pointers.

### BFS for Fewest Stops

Used for unweighted shortest paths (`bfsFewestStops`) — since every edge represents exactly "one stop" regardless of distance, a standard queue-based BFS guarantees the minimum number of intermediate stations.

### Interchange Detection & Line Merging

When `addStation()` is called with a name that already exists in the graph (e.g., "Ameerpet" added once from the Red line dataset and again from the Blue line dataset), the station is **not duplicated**. Instead, its `lineColor` field is extended (e.g., `"Red Line / Blue Line"`), and the same graph node continues to be referenced by edges from both lines. This is what allows a single Dijkstra/BFS run to seamlessly cross from one line to another at a real-world interchange.

Separately, `annotateRoute()` performs a **post-processing pass** over the computed route to detect *which* interchanges were actually used in this specific journey (by comparing the registered line of each path segment), and surfaces a clear "Alight here / Board here" instruction to the user.

---

## Installation & Build

### Prerequisites
- A C++17-compatible compiler (g++ 9+ / clang 10+)
- No external dependencies — the entire project is header-only and self-contained

### Build Commands

```bash
# Build the main application
g++ -std=c++17 -O2 main.cpp -o metro_engine

# Run it
./metro_engine
```

### Running Tests

```bash
g++ -std=c++17 -O2 test_all.cpp -o test_all
./test_all
```

`test_all` exercises:
- Total station count (expected: 56)
- Correct interchange line-merging at Ameerpet, Parade Ground, and MG Bus Station
- Cross-line Dijkstra routing (Red ↔ Blue, Green ↔ Red)
- Same-line BFS routing
- Updated fare-slab correctness across all distance bands
- Invalid-station guard behavior for both Dijkstra and BFS

---

## Usage

1. Launch `./metro_engine`.
2. From the main menu, choose:
   - **1** to view the full metro map (line-by-line station listing with interchange markers)
   - **2** to get the shortest route by distance (with fare)
   - **3** to get the route with the fewest stops
   - **4** to list every station and its line
   - **5** to exit
3. For options 2 and 3, you'll be prompted for a source and destination station name. Input is trimmed and validated — invalid names are rejected with a hint to use option 4 for exact spellings.

**Example — Shortest path, Miyapur → Nagole:**

```
 START  →  Miyapur                          [Red Line]
         ·  ...
  ○  Ameerpet                          [Red Line / Blue Line]
        │
  ┌── CHANGE LINE ───────────────────────────────────────┐
  │  Alight : Red Line
  │  Board  : Blue Line
  └──────────────────────────────────────────────────────┘
         ·  ...
DEST   →  Nagole                          [Blue Line]

  Intermediate stops  : 19
  Total distance      : 24.30 km
  Line changes        : 1
    ○  Ameerpet  [Red Line  →  Blue Line]

  ┌──────────────────────────────────────┐
  │          FARE BREAKDOWN              │
  ├──────────────────────────────────────┤
  │  Distance :    24.30 km             │
  │  Fare     :   ₹ 69.00               │
  └──────────────────────────────────────┘
```

---

## Complexity Analysis

| Operation | Time Complexity | Notes |
|---|---|---|
| Station lookup (`hasStation`/`getIndex`) | O(1) average | `unordered_map<string,int>` |
| `addEdge` | O(1) amortized | adjacency-list insertion |
| Dijkstra (`dijkstraShortestPath`) | O((V + E) log V) | V = 56 stations, E ≈ 110 directed entries; binary heap push/pop is O(log V) |
| BFS (`bfsFewestStops`) | O(V + E) | each station and edge visited once |
| Route annotation (`annotateRoute`) | O(R × L) | R = route length, L = total stations across all registered lines (linear scan per segment) |
| Min-heap push / extractMin | O(log n) | n = current heap size |

**Space Complexity:** O(V + E) for the adjacency list, plus O(V) for the `dist[]`, `parent[]`, and `visited[]` arrays used by both algorithms.

> Given V = 56 and E ≈ 220 (undirected, double-counted), both algorithms run in well under a millisecond on this dataset. The complexity bounds above generalize cleanly to larger transit networks.

---

## Project Structure

```
.
├── main.cpp           # CLI entry point and menu handlers
├── graph.hpp          # MetroGraph engine (Dijkstra, BFS, annotation, map rendering)
├── heap.hpp           # Custom binary min-heap (Dijkstra's priority queue)
├── fare.hpp           # Fare slab calculator
├── metro_data.hpp     # Hardcoded Red/Blue/Green line dataset
├── test_all.cpp       # Verification test suite
└── README.md
```

---

## Testing

The project ships with `test_all.cpp`, a standalone executable that prints diagnostic output for:

- Graph construction correctness (station count, interchange merging)
- Algorithmic correctness (Dijkstra and BFS on real cross-line and same-line routes)
- Fare-table correctness across all distance bands
- Defensive behavior on unknown station names

> **Note:** The current suite is print-and-inspect rather than assert-based. A planned improvement is migrating to a proper assertion framework (see Limitations below) for automated pass/fail reporting and CI integration.

---
<!--
## Limitations & Future Improvements

**Current limitations:**
- Uses `#include <bits/stdc++.h>`, which is GCC-specific and not portable across all compilers.
- Test suite reports results via console output rather than asserting pass/fail automatically.
- Station and fare data are hardcoded in C++ rather than loaded from an external config (JSON/CSV).
- No build system (CMake/Makefile) — relies on direct `g++` invocation.
- Dijkstra's min-heap uses lazy deletion rather than decrease-key (a deliberate, documented trade-off — see Algorithms section).

**Planned improvements:**
- [ ] Replace `bits/stdc++.h` with targeted standard headers
- [ ] Migrate `test_all.cpp` to GoogleTest/Catch2 with CI (GitHub Actions)
- [ ] Externalize station/fare data into JSON, loaded at runtime
- [ ] Add a CMake build system
- [ ] Add an A* search mode using real station coordinates as a heuristic
- [ ] Add fare-optimal routing (minimizing cost, not just distance/stops)
- [ ] Export the metro map to Graphviz/SVG for visual documentation
- [ ] Benchmark the custom MinHeap against `std::priority_queue` on synthetic larger graphs
- [ ] Wrap `MetroGraph` in a minimal REST API for programmatic route queries

---
-->
**Author:** *zen-code11*
