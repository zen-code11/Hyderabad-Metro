#pragma once
#include <bits/stdc++.h>
using namespace std;

// ============================================================
//  FareCalculator — Official Hyderabad Metro fare slabs
//
//  Distance Band          Fare (Token / Digital / Smart Card)
//  ──────────────────────────────────────────────────────────
//  Up to 2 km             ₹ 11
//  > 2 km  up to  4 km   ₹ 17
//  > 4 km  up to  6 km   ₹ 28
//  > 6 km  up to  9 km   ₹ 37
//  > 9 km  up to 12 km   ₹ 47
//  > 12 km up to 15 km   ₹ 51
//  > 15 km up to 18 km   ₹ 56
//  > 18 km up to 21 km   ₹ 61
//  > 21 km up to 24 km   ₹ 65
//  Above 24 km            ₹ 69
//
//  Source: Hyderabad Metro Rail (L&T Metro Rail Hyderabad Ltd.)
// ============================================================
namespace FareCalculator {

    inline double calculateFare(double distanceKm) {
        if (distanceKm <=  0.0) return  0.0;
        if (distanceKm <=  2.0) return 11.0;
        if (distanceKm <=  4.0) return 17.0;
        if (distanceKm <=  6.0) return 28.0;
        if (distanceKm <=  9.0) return 37.0;
        if (distanceKm <= 12.0) return 47.0;
        if (distanceKm <= 15.0) return 51.0;
        if (distanceKm <= 18.0) return 56.0;
        if (distanceKm <= 21.0) return 61.0;
        if (distanceKm <= 24.0) return 65.0;
        return 69.0;
    }

    // Prints a formatted fare breakdown box to the console
    inline void printFareBreakdown(double distanceKm) {
        double fare = calculateFare(distanceKm);
        cout << fixed << setprecision(2);
        cout << "\n  ┌──────────────────────────────────────┐\n";
        cout << "  │          FARE BREAKDOWN              │\n";
        cout << "  ├──────────────────────────────────────┤\n";
        cout << "  │  Distance : " << setw(8) << distanceKm << " km             │\n";
        cout << "  │  Fare     :   \u20b9 " << setw(5) << fare << "               │\n";
        cout << "  └──────────────────────────────────────┘\n";
    }

} // namespace FareCalculator
