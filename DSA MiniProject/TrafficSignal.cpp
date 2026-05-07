// ============================================================
//           PART 1: HEADERS, INPUT & DATA SETUP
// ============================================================

#include <iostream>    // For cin, cout
#include <vector>      // For dynamic array (vector)
#include <cfloat>      // For DBL_MAX (largest double value)
#include <iomanip>     // For fixed, setprecision (decimal formatting)
#include <algorithm>   // For min() function

using namespace std;

// Structure to store each time interval's traffic data
struct TrafficData {
    int intervalNumber;    // Which time slot (1, 2, 3...)
    int arrivalRate;       // How many vehicles arrived
};

// Function to take input dynamically
vector<TrafficData> takeInput(int &n) {
    cout << "\n========================================\n";
    cout << "   SMART TRAFFIC SIGNAL TIMING SYSTEM   \n";
    cout << "========================================\n";

    cout << "\nEnter number of time intervals: ";
    cin >> n;

    // Validate input
    while (n <= 0) {
        cout << "Invalid! Enter a positive number: ";
        cin >> n;
    }

    vector<TrafficData> data(n);

    cout << "\nEnter vehicle arrival rates for each interval:\n";
    cout << "------------------------------------------------\n";

    for (int i = 0; i < n; i++) {
        data[i].intervalNumber = i + 1;
        cout << "  Interval " << i + 1 << " (vehicles/sec): ";
        cin >> data[i].arrivalRate;

        // Validate each arrival rate
        while (data[i].arrivalRate < 0) {
            cout << "  Invalid! Enter a non-negative value: ";
            cin >> data[i].arrivalRate;
        }
    }

    // Display what was entered
    cout << "\n--- Input Summary ---\n";
    cout << left << setw(12) << "Interval"
         << setw(20) << "Arrival Rate" << "\n";
    cout << "-----------------------------\n";
    for (int i = 0; i < n; i++) {
        cout << left << setw(12) << data[i].intervalNumber
             << setw(20) << data[i].arrivalRate << "\n";
    }

    return data;
}

// ============================================================
//         PART 2: SIMULATE FUNCTION (CORE LOGIC)
// ============================================================

// Structure to hold simulation result for one green duration
struct SimulationResult {
    int greenTime;         // Green signal duration tested
    int redTime;           // Corresponding red duration
    double avgWaitTime;    // Average waiting time calculated
    int maxQueueSize;      // Peak queue size during simulation
    int totalVehicles;     // Total vehicles that arrived
    int totalServed;       // Total vehicles that passed
    int totalLeftover;     // Vehicles that couldn't pass
};

SimulationResult simulate(vector<TrafficData> &data, int greenTime) {

    SimulationResult result;
    result.greenTime    = greenTime;
    result.redTime      = 60 - greenTime;   // Fixed 60s cycle
    result.maxQueueSize = 0;
    result.totalServed  = 0;
    result.totalLeftover= 0;
    result.totalVehicles= 0;

    int totalWait = 0;
    int queue     = 0;
    int n         = data.size();

    // Simulate each time interval
    for (int i = 0; i < n; i++) {

        // Step 1: New vehicles join the queue
        queue += data[i].arrivalRate;
        result.totalVehicles += data[i].arrivalRate;

        // Step 2: Track peak queue
        if (queue > result.maxQueueSize)
            result.maxQueueSize = queue;

        // Step 3: Serve vehicles during green phase
        // Cannot serve more than queue size OR green time allows
        int served = min(queue, greenTime);
        queue -= served;
        result.totalServed += served;

        // Step 4: Remaining vehicles wait through red phase
        result.totalLeftover += queue;
        totalWait += queue * result.redTime;
    }

    // Calculate average waiting time
    result.avgWaitTime = (double)totalWait / n;

    return result;
}

// Display detailed simulation result for a given green time
void displaySimulation(SimulationResult &res) {
    cout << "\n  Green=" << res.greenTime << "s"
         << " | Red=" << res.redTime << "s"
         << " | AvgWait=" << fixed << setprecision(2) << res.avgWaitTime << "s"
         << " | MaxQueue=" << res.maxQueueSize
         << " | Served=" << res.totalServed
         << " | Leftover=" << res.totalLeftover;
}

// ============================================================
//        PART 3: BINARY SEARCH FOR OPTIMAL GREEN TIME
// ============================================================

// Structure to track binary search progress
struct SearchStep {
    int step;         // Iteration number
    int lo, hi, mid;  // Search boundaries and midpoint
    double waitMid;   // Wait time at mid
    double waitMid1;  // Wait time at mid+1
    string direction; // Which way search moved
};

// Binary Search to find optimal green signal duration
SimulationResult binarySearchOptimal(vector<TrafficData> &data) {

    int lo = 10, hi = 50;          // Search range: 10s to 50s
    double bestWait    = DBL_MAX;  // Start with infinity
    int optimalGreen   = lo;
    int stepCount      = 0;

    vector<SearchStep> searchLog;  // Log all steps for display

    cout << "\n--- Binary Search Progress ---\n";
    cout << left
         << setw(6)  << "Step"
         << setw(6)  << "Lo"
         << setw(6)  << "Hi"
         << setw(6)  << "Mid"
         << setw(12) << "Wait@Mid"
         << setw(12) << "Wait@Mid+1"
         << setw(12) << "Direction" << "\n";
    cout << string(60, '-') << "\n";

    while (lo <= hi) {
        stepCount++;
        int mid = (lo + hi) / 2;

        // Simulate for mid and mid+1
        SimulationResult resMid  = simulate(data, mid);
        SimulationResult resMid1 = simulate(data, mid + 1);

        // Save best result found so far
        if (resMid.avgWaitTime < bestWait) {
            bestWait     = resMid.avgWaitTime;
            optimalGreen = mid;
        }

        // Determine search direction
        string direction;
        if (resMid1.avgWaitTime < resMid.avgWaitTime) {
            lo        = mid + 1;   // More green time helps → go RIGHT
            direction = "RIGHT →";
        } else {
            hi        = mid - 1;   // Less green time better → go LEFT
            direction = "← LEFT";
        }

        // Log this step
        cout << left
             << setw(6)  << stepCount
             << setw(6)  << lo
             << setw(6)  << hi
             << setw(6)  << mid
             << setw(12) << fixed << setprecision(1) << resMid.avgWaitTime
             << setw(12) << fixed << setprecision(1) << resMid1.avgWaitTime
             << setw(12) << direction << "\n";
    }

    cout << "\nTotal Search Steps: " << stepCount
         << " (out of possible 40 linear steps)\n";

    // Return final best simulation result
    return simulate(data, optimalGreen);
}

// ============================================================
//          PART 4: OUTPUT, RESULTS & MAIN FUNCTION
// ============================================================

// Display a visual bar representing green vs red time
void displaySignalBar(int greenTime) {
    int redTime = 60 - greenTime;
    cout << "\nSignal Cycle (60s):\n  [";

    // Green portion
    for (int i = 0; i < greenTime / 2; i++) cout << "G";
    cout << "|";

    // Red portion
    for (int i = 0; i < redTime / 2; i++) cout << "R";
    cout << "]\n";
    cout << "  |←── " << greenTime << "s Green ──→|←── "
         << redTime << "s Red ──→|\n";
}

// Display efficiency comparison
void displayEfficiency(SimulationResult &optimal,
                       SimulationResult &worst) {
    double improvement = 0;
    if (worst.avgWaitTime > 0)
        improvement = ((worst.avgWaitTime - optimal.avgWaitTime)
                       / worst.avgWaitTime) * 100;

    cout << "\n--- Efficiency Metrics ---\n";
    cout << "  Worst case wait  : " << fixed << setprecision(2)
         << worst.avgWaitTime << "s  (green=10s)\n";
    cout << "  Optimal wait     : " << fixed << setprecision(2)
         << optimal.avgWaitTime << "s  (green=" << optimal.greenTime << "s)\n";
    cout << "  Improvement      : " << fixed << setprecision(1)
         << improvement << "% reduction in wait time\n";
    cout << "  Vehicles served  : " << optimal.totalServed << "\n";
    cout << "  Vehicles leftover: " << optimal.totalLeftover << "\n";
    cout << "  Peak queue size  : " << optimal.maxQueueSize << "\n";
}

// Display final results
void displayResults(SimulationResult &optimal,
                    SimulationResult &worst) {
    cout << "\n\n========================================\n";
    cout << "             FINAL RESULTS              \n";
    cout << "========================================\n";
    cout << "  Optimal Green Duration : " << optimal.greenTime << " seconds\n";
    cout << "  Red Signal Duration    : " << optimal.redTime   << " seconds\n";
    cout << fixed << setprecision(2);
    cout << "  Min Avg Waiting Time   : " << optimal.avgWaitTime << " seconds\n";

    displaySignalBar(optimal.greenTime);
    displayEfficiency(optimal, worst);

    cout << "\n========================================\n";
    cout << "  Status: ";
    if (optimal.avgWaitTime == 0)
        cout << "✅ No congestion — all vehicles pass!\n";
    else if (optimal.avgWaitTime < 50)
        cout << "⚡ Moderate wait — traffic manageable.\n";
    else
        cout << "⚠️  High wait — consider adding lanes.\n";
    cout << "========================================\n";
}

// ============================================================
//                        MAIN FUNCTION
// ============================================================
int main() {

    // --- PART 1: Take Input ---
    int n;
    vector<TrafficData> data = takeInput(n);

    // --- PART 2 & 3: Run Binary Search ---
    cout << "\n\n--- Simulating Traffic Flow ---\n";
    SimulationResult optimal = binarySearchOptimal(data);

    // Get worst case (minimum green = 10s) for comparison
    SimulationResult worst = simulate(data, 10);

    // --- PART 4: Display Results ---
    displayResults(optimal, worst);

    return 0;
}