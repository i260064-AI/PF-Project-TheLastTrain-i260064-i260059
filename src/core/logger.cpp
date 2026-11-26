#include "logger.h"
#include "util.h"

// File persistence for savegame.txt and game_summary.txt using primitive variables
#include <fstream>
using namespace std;

// File persistence for savegame.txt and game_summary.txt using primitive variables

bool saveGameStateToFile(const char* filename,
                         int playerFloor, int playerRoom,
                         int energy, int time, int score,
                         const char inventory[MAX_INVENTORY_CAPACITY], int inventoryCount,
                         const bool objectives[NUM_OBJECTIVES], int emergencyTripsLeft,
                         const char roomEntities[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM],
                         const int entityCounts[NUM_FLOORS][ROOMS_PER_FLOOR])
{
    if (filename == nullptr)
    {
        return false;
    }

    ofstream file(filename);
    if (!file.is_open())
    {
        return false;
    }

    // Player and resources
    file << playerFloor << " " << playerRoom << " "
         << energy << " " << time << " " << score << "\n";

    // Inventory
    file << inventoryCount << "\n";
    for (int i = 0; i < inventoryCount; i++)
    {
        file << inventory[i] << " ";
    }
    file << "\n";

    // Objectives (1 = done, 0 = not done)
    for (int i = 0; i < NUM_OBJECTIVES; i++)
    {
        if (objectives[i])
            file << 1 << " ";
        else
            file << 0 << " ";
    }
    file << "\n";

    file << emergencyTripsLeft << "\n";

    // Room entities: count, then that many chars
    for (int f = 0; f < NUM_FLOORS; f++)
    {
        for (int r = 0; r < ROOMS_PER_FLOOR; r++)
        {
            file << entityCounts[f][r] << " ";
            for (int e = 0; e < entityCounts[f][r]; e++)
            {
                file << roomEntities[f][r][e] << " ";
            }
            file << "\n";
        }
    }

    bool ok = file.good();
    file.close();
    return ok;
}

bool loadGameStateFromFile(const char* filename,
                           int& playerFloor, int& playerRoom,
                           int& energy, int& time, int& score,
                           char inventory[MAX_INVENTORY_CAPACITY], int& inventoryCount,
                           bool objectives[NUM_OBJECTIVES], int& emergencyTripsLeft,
                           char roomEntities[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM],
                           int entityCounts[NUM_FLOORS][ROOMS_PER_FLOOR])
{
    if (filename == nullptr)
    {
        return false;
    }

    ifstream file(filename);
    if (!file.is_open())
    {
        return false;
    }

    file >> playerFloor >> playerRoom >> energy >> time >> score;
    if (file.fail())
    {
        file.close();
        return false;
    }

    // Inventory
    file >> inventoryCount;
    if (file.fail() || inventoryCount < 0 || inventoryCount > MAX_INVENTORY_CAPACITY)
    {
        file.close();
        return false;
    }
    for (int i = 0; i < inventoryCount; i++)
    {
        file >> inventory[i];
    }

    // Objectives
    for (int i = 0; i < NUM_OBJECTIVES; i++)
    {
        int flag = 0;
        file >> flag;
        objectives[i] = (flag != 0);
    }

    file >> emergencyTripsLeft;

    // Room entities
    for (int f = 0; f < NUM_FLOORS; f++)
    {
        for (int r = 0; r < ROOMS_PER_FLOOR; r++)
        {
            int count = 0;
            file >> count;
            if (file.fail() || count < 0 || count > MAX_ENTITIES_PER_ROOM)
            {
                file.close();
                return false;
            }
            entityCounts[f][r] = count;
            for (int e = 0; e < count; e++)
            {
                file >> roomEntities[f][r][e];
            }
        }
    }

    bool ok = !file.fail();
    file.close();
    return ok;
}

void writeGameSummaryReport(const char* filename,
                            int score, int remainingEnergy, int remainingTime,
                            const bool objectives[NUM_OBJECTIVES], bool victory)
{
    if (filename == nullptr)
    {
        return;
    }

    ofstream file(filename);
    if (!file.is_open())
    {
        return;
    }

    int completed = 0;
    for (int i = 0; i < NUM_OBJECTIVES; i++)
    {
        if (objectives[i])
            completed++;
    }

    file << "===== THE LAST TRAIN: GAME SUMMARY =====\n";
    if (victory)
        file << "Result: VICTORY\n";
    else
        file << "Result: DEFEAT\n";

    file << "Final Score: " << score << "\n";
    file << "Remaining Energy: " << remainingEnergy << "\n";
    file << "Remaining Time: " << remainingTime << "\n";
    file << "Objectives Completed: " << completed << " / " << NUM_OBJECTIVES << "\n";

    for (int i = 0; i < NUM_OBJECTIVES; i++)
    {
        file << "  Objective " << (i + 1) << ": ";
        if (objectives[i])
            file << "Completed\n";
        else
            file << "Not completed\n";
    }

    file.close();
}