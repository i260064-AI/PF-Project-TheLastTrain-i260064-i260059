
#include "simulator.h"
#include "level.h"
#include "physics.h"
#include "switches.h"
#include "logger.h"
#include "viewer_iface.h"
#include "util.h"
#include <iostream>

using namespace std;

bool checkEndConditions(int energy, int time, const bool objectives[NUM_OBJECTIVES])
{
    if (objectives[OBJ_MISSION_WON] || objectives[OBJ_MISSION_FAILED])
        return true;

    if (energy <= 0 || time <= 0)
        return true;

    return false;
}

void runSimulation(int& playerFloor, int& playerRoom,
                   int& energy, int& time, int& score,
                   char inventory[MAX_INVENTORY_CAPACITY], int& inventoryCount,
                   bool objectives[NUM_OBJECTIVES], int& emergencyTripsLeft,
                   char horizontalConn[NUM_FLOORS][ROOMS_PER_FLOOR - 1],
                   char roomEntities[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM],
                   int entityCounts[NUM_FLOORS][ROOMS_PER_FLOOR])
{
    bool lift2Operational = objectives[OBJ_LIFT2_ACTIVE];
    bool gameOver = false;
    bool victory = false;

    char cmd[20];
    char code[20];
    char sym;
    int f, r;

    cout << "The Last Train" << endl;
    cout << "Commands: move F R, inspect, collect X, use X, unlock CODE, save, load, quit" << endl;

    while (!gameOver)
    {
        cout << endl;
        cout << "Location: F" << playerFloor << "-R" << playerRoom
             << " | Energy: " << energy
             << " | Time: " << time
             << " | Score: " << score << endl;
        cout << "Command: ";

        cin.width(20);
        if (!(cin >> cmd))
            break;

        bool positionValid = (playerFloor >= 1 && playerFloor <= NUM_FLOORS &&
                              playerRoom >= 1 && playerRoom <= ROOMS_PER_FLOOR);

        if (stringEquals(cmd, "quit"))
        {
            break;
        }
        else if (stringEquals(cmd, "move"))
        {
            if (cin >> f >> r)
            {
                lift2Operational = objectives[OBJ_LIFT2_ACTIVE];

                handleMoveCommand(f, r, playerFloor, playerRoom, energy, time,
                                  horizontalConn, inventory, inventoryCount,
                                  emergencyTripsLeft, lift2Operational);
            }
        }
        else if (stringEquals(cmd, "inspect"))
        {
            if (positionValid)
            {
                handleInspectCommand(playerFloor, playerRoom, energy, time,
                                     roomEntities,
                                     entityCounts[playerFloor - 1][playerRoom - 1],
                                     objectives, score);
            }
            else
            {
                cout << "Invalid player position." << endl;
            }
        }
        else if (stringEquals(cmd, "collect"))
        {
            if (cin >> sym)
            {
                if (positionValid)
                {
                    handleCollectCommand(sym, playerFloor, playerRoom, energy, time, score,
                                         roomEntities,
                                         entityCounts[playerFloor - 1][playerRoom - 1],
                                         inventory, inventoryCount, objectives);
                }
                else
                {
                    cout << "Invalid player position." << endl;
                }
            }
        }
        else if (stringEquals(cmd, "use"))
        {
            if (cin >> sym)
            {
                handleUseCommand(sym, playerFloor, playerRoom, energy, time, score,
                                 inventory, inventoryCount, objectives);
            }
        }
        else if (stringEquals(cmd, "unlock"))
        {
            cin.width(20);
            if (cin >> code)
            {
                if (positionValid)
                {
                    handleUnlockCabinetCommand(code, playerFloor, playerRoom,
                                               objectives, score, roomEntities,
                                               entityCounts[playerFloor - 1][playerRoom - 1]);
                }
                else
                {
                    cout << "Invalid player position." << endl;
                }
            }
        }
        else if (stringEquals(cmd, "save"))
        {
            if (saveGameStateToFile("./out/savegame.txt", playerFloor, playerRoom,
                                    energy, time, score, inventory, inventoryCount,
                                    objectives, emergencyTripsLeft,
                                    roomEntities, entityCounts))
                cout << "Game saved." << endl;
            else
                cout << "Save failed." << endl;
        }
        else if (stringEquals(cmd, "load"))
        {
            if (loadGameStateFromFile("./out/savegame.txt", playerFloor, playerRoom,
                                      energy, time, score, inventory, inventoryCount,
                                      objectives, emergencyTripsLeft,
                                      roomEntities, entityCounts))
            {
                if (playerFloor >= 1 && playerFloor <= NUM_FLOORS &&
                    playerRoom >= 1 && playerRoom <= ROOMS_PER_FLOOR)
                {
                    lift2Operational = objectives[OBJ_LIFT2_ACTIVE];
                    cout << "Game loaded." << endl;
                }
                else
                {
                    cout << "Invalid saved player position." << endl;
                }
            }
            else
            {
                cout << "Load failed." << endl;
            }
        }
        else
        {
            cout << "Unknown command." << endl;
        }

        if (cin.fail() && !cin.eof())
        {
            cin.clear();
            cin.ignore(1000, '\n');
        }

        if (energy <= 0 || time <= 0)
            objectives[OBJ_MISSION_FAILED] = true;

        if (checkEndConditions(energy, time, objectives))
        {
            gameOver = true;
            victory = objectives[OBJ_MISSION_WON];

            if (objectives[OBJ_MISSION_FAILED])
                victory = false;
        }
    }

    if (gameOver)
    {
        cout << endl;

        if (victory)
            cout << "Mission won" << endl;
        else
            cout << "Mission failed" << endl;

        writeGameSummaryReport("./out/game_summary.txt",
                               score, energy, time, objectives, victory);
    }
}
 