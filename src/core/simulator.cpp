#include "simulator.h"
#include "level.h"
#include "physics.h"
#include "switches.h"
#include "logger.h"
#include "viewer_iface.h"
#include "util.h"
#include <iostream>
#include <string>

using namespace std;

void runSimulation(int& playerFloor, int& playerRoom,
                   int& energy, int& time, int& score,
                   char inventory[MAX_INVENTORY_CAPACITY], int& inventoryCount,
                   bool objectives[NUM_OBJECTIVES], int& emergencyTripsLeft,
                   char horizontalConn[NUM_FLOORS][ROOMS_PER_FLOOR - 1],
                   char roomEntities[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM],
                   int entityCounts[NUM_FLOORS][ROOMS_PER_FLOOR])
{
    (void)inventory;
    (void)inventoryCount;
    (void)objectives;
    (void)emergencyTripsLeft;
    (void)horizontalConn;

    cout << "\n=== THE LAST TRAIN: INITIALIZED ===\n";
    cout << "Location: F" << playerFloor << "-R" << playerRoom
         << " | Energy: " << energy
         << " | Time: " << time
         << " | Score: " << score << "\n\n";

    cout << "--- Parsed Map Entities ---\n";
    for (int f = NUM_FLOORS - 1; f >= 0; f--)
    {
        for (int r = 0; r < ROOMS_PER_FLOOR; r++)
        {
            if (entityCounts[f][r] > 0)
            {
                cout << "F" << (f + 1) << "-R" << (r + 1) << ": ";
                for (int e = 0; e < entityCounts[f][r]; e++)
                {
                    char sym = roomEntities[f][r][e];
                    cout << sym << " (" << getEntityName(sym) << ") ";
                }
                cout << "\n";
            }
        }
    }

    string cmd;
    while (true)
    {
        cout << "\nCommand (quit to exit): ";
        if (!(cin >> cmd) || cmd == "quit")
        {
            break;
        }
        cout << "Command received: " << cmd << "\n";
    }
}