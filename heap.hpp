#pragma once
#include <bits/stdc++.h>
using namespace std;

// ============================================================
//  HeapNode — one entry in the min-heap
//  Stores a tentative distance + the station's integer index.
//  Lazy-deletion is handled by the Dijkstra caller (visited[]).
// ============================================================
struct HeapNode {
    double distance;
    int    stationIndex;

    HeapNode(double d, int idx) : distance(d), stationIndex(idx) {}

    // Min-heap ordering: smaller distance = higher priority
    bool operator>(const HeapNode& o) const { return distance > o.distance; }
};

// ============================================================
//  MinHeap — custom binary min-heap
//  Supports push / extractMin / isEmpty / size.
//  heapifyUp / heapifyDown are public so test code can call them,
//  but they are considered implementation detail.
// ============================================================
class MinHeap {
public:
    MinHeap() = default;

    // ----- Core public API ----------------------------------

    void push(double distance, int stationIndex) {
        data.emplace_back(distance, stationIndex);
        heapifyUp(data.size() - 1);
    }

    // Returns and removes the minimum element.
    // Throws std::out_of_range if the heap is empty.
    HeapNode extractMin() {
        if (data.empty())
            throw out_of_range("MinHeap::extractMin called on empty heap");

        HeapNode minNode = data[0];

        // Move the last element to the root and restore heap property
        data[0] = data.back();
        data.pop_back();

        if (!data.empty())
            heapifyDown(0);

        return minNode;
    }

    bool   isEmpty() const noexcept { return data.empty(); }
    size_t size()    const noexcept { return data.size();  }

    // Peek at min without removing (useful for debugging / assertions)
    const HeapNode& peek() const {
        if (data.empty())
            throw out_of_range("MinHeap::peek called on empty heap");
        return data[0];
    }

private:
    vector<HeapNode> data;

    // ----- Index helpers ------------------------------------
    size_t parent(size_t i) const noexcept { return (i - 1) / 2; }
    size_t left  (size_t i) const noexcept { return 2 * i + 1;   }
    size_t right (size_t i) const noexcept { return 2 * i + 2;   }

    // ----- Structural helpers --------------------------------
    void swapNodes(size_t i, size_t j) {
        swap(data[i], data[j]);
    }

    // Bubble the element at idx UP until heap property is satisfied
    void heapifyUp(size_t idx) {
        while (idx > 0) {
            size_t p = parent(idx);
            if (data[p].distance > data[idx].distance) {
                swapNodes(p, idx);
                idx = p;
            } else {
                break;
            }
        }
    }

    // Sink the element at idx DOWN until heap property is satisfied
    void heapifyDown(size_t idx) {
        size_t n = data.size();
        while (true) {
            size_t smallest = idx;
            size_t l = left(idx);
            size_t r = right(idx);

            if (l < n && data[l].distance < data[smallest].distance)
                smallest = l;
            if (r < n && data[r].distance < data[smallest].distance)
                smallest = r;

            if (smallest != idx) {
                swapNodes(idx, smallest);
                idx = smallest;
            } else {
                break;
            }
        }
    }
};
