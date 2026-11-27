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

bool handleMoveCommand(int targetFloor, int targetRoom,
                       int& currentFloor, int& currentRoom,
                       int& energy, int& time,
                       const char horizontalConn[NUM_FLOORS][ROOMS_PER_FLOOR - 1],
                       const char inventory[MAX_INVENTORY_CAPACITY], int inventoryCount,
                       int& emergencyTripsLeft, bool lift2Operational)
{
    char connType;

    if (!isDirectlyConnected(currentFloor, currentRoom, targetFloor, targetRoom,
                             horizontalConn, connType))
    {
        cout << "There is no direct connection to that room." << endl;
        return false;
    }

    if (!canTraverse(targetFloor, targetRoom, connType, inventory, inventoryCount,
                     emergencyTripsLeft, lift2Operational))
    {
        cout << "You cannot go that way right now." << endl;
        return false;
    }

    int costEnergy = COST_MOVE_ORDINARY_ENERGY;
    int costTime = COST_MOVE_ORDINARY_TIME;

    if (connType == CONN_WINDOW)
    {
        costEnergy = COST_MOVE_WINDOW_ENERGY;
        costTime = COST_MOVE_WINDOW_TIME;
    }
    else if (connType == CONN_LIFT)
    {
        costEnergy = COST_MOVE_LIFT_ENERGY;
        costTime = COST_MOVE_LIFT_TIME;

        if (targetRoom == 4 && !lift2Operational)
        {
            emergencyTripsLeft--;
            cout << "Emergency trip used. Trips left: "
                 << emergencyTripsLeft << endl;
        }
    }

    applyResourceDelta(energy, time, costEnergy, costTime);

    currentFloor = targetFloor;
    currentRoom = targetRoom;

    cout << "Moved to F" << currentFloor << "-R" << currentRoom << endl;
    return true;
}

bool handleInspectCommand(int currentFloor, int currentRoom,
                          int& energy, int& time,
                          const char roomEntities[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM],
                          int entityCount,
                          bool objectives[NUM_OBJECTIVES], int& score)
{
    if (currentFloor < 1 || currentFloor > NUM_FLOORS ||
        currentRoom < 1 || currentRoom > ROOMS_PER_FLOOR)
    {
        cout << "Invalid player position." << endl;
        return false;
    }

    applyResourceDelta(energy, time, COST_INSPECT_ENERGY, COST_INSPECT_TIME);

    int f = currentFloor - 1;
    int r = currentRoom - 1;

    if (entityCount <= 0)
    {
        cout << "There is nothing here." << endl;
        return true;
    }

    cout << "You see:" << endl;

    for (int e = 0; e < entityCount; e++)
    {
        char sym = roomEntities[f][r][e];

        cout << sym << " (" << getEntityName(sym) << ")" << endl;

        if (sym == ENT_CLUE_4 && !objectives[OBJ_CLUE_4])
        {
            objectives[OBJ_CLUE_4] = true;
            applyScoreDelta(score, SCORE_CLUE_FOUND);
            cout << "Clue found: digit 4" << endl;
        }
        else if (sym == ENT_CLUE_2 && !objectives[OBJ_CLUE_2])
        {
            objectives[OBJ_CLUE_2] = true;
            applyScoreDelta(score, SCORE_CLUE_FOUND);
            cout << "Clue found: digit 2" << endl;
        }
        else if (sym == ENT_CLUE_7 && !objectives[OBJ_CLUE_7])
        {
            objectives[OBJ_CLUE_7] = true;
            applyScoreDelta(score, SCORE_CLUE_FOUND);
            cout << "Clue found: digit 7" << endl;
        }
    }

    return true;
}

bool handleCollectCommand(char itemSymbol, int currentFloor, int currentRoom,
                          int& energy, int& time, int& score,
                          char roomEntities[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM],
                          int& entityCount,
                          char inventory[MAX_INVENTORY_CAPACITY], int& inventoryCount,
                          bool objectives[NUM_OBJECTIVES])
{
    if (currentFloor < 1 || currentFloor > NUM_FLOORS ||
        currentRoom < 1 || currentRoom > ROOMS_PER_FLOOR)
    {
        cout << "Invalid player position." << endl;
        return false;
    }

    int f = currentFloor - 1;
    int r = currentRoom - 1;

    int index = -1;

    for (int e = 0; e < entityCount; e++)
    {
        if (roomEntities[f][r][e] == itemSymbol)
        {
            index = e;
            break;
        }
    }

    if (index == -1)
    {
        cout << "That item is not here." << endl;
        return false;
    }

    if (itemSymbol == ENT_CLUE_4 || itemSymbol == ENT_CLUE_2 ||
        itemSymbol == ENT_CLUE_7)
    {
        cout << "Clues are found by inspecting the room." << endl;
        return false;
    }

    if (itemSymbol == ENT_CONTROL_PANEL)
    {
        cout << "The control panel cannot be collected." << endl;
        return false;
    }

    if (itemSymbol == ENT_CABINET_TRAIN_KEY &&
        !objectives[OBJ_CABINET_UNLOCKED])
    {
        cout << "The cabinet is locked." << endl;
        return false;
    }

    bool addToInventory = true;

    if (itemSymbol == ENT_PASSENGER)
    {
        if (!hasItem(inventory, inventoryCount, ENT_MEDICAL_KIT))
        {
            cout << "You need a Medical Kit to rescue the passenger." << endl;
            return false;
        }

        addToInventory = false;
    }

    if (itemSymbol == ENT_CAT)
    {
        if (!hasItem(inventory, inventoryCount, ENT_CAT_CARRIER))
        {
            cout << "You need the Cat Carrier to secure the cat." << endl;
            return false;
        }

        addToInventory = false;
    }

    if (itemSymbol == ENT_ENERGY_PACK ||
        itemSymbol == ENT_EMERGENCY_TOOLKIT ||
        itemSymbol == ENT_OBSERVATION_TOKEN)
    {
        addToInventory = false;
    }

    if (addToInventory &&
        !addItem(inventory, inventoryCount, itemSymbol))
    {
        cout << "Your inventory is full." << endl;
        return false;
    }

    for (int e = index; e < entityCount - 1; e++)
    {
        roomEntities[f][r][e] = roomEntities[f][r][e + 1];
    }

    entityCount--;
    roomEntities[f][r][entityCount] = ENT_NONE;

    if (addToInventory)
    {
        applyResourceDelta(energy, time,
                           COST_ITEM_PICKUP_ENERGY,
                           COST_ITEM_PICKUP_TIME);
    }

    if (itemSymbol == ENT_ENERGY_PACK)
    {
        applyResourceDelta(energy, time, BONUS_ENERGY_PACK, 0);
    }
    else if (itemSymbol == ENT_EMERGENCY_TOOLKIT)
    {
        applyResourceDelta(energy, time, BONUS_EMERGENCY_TOOLKIT, 0);
    }
    else if (itemSymbol == ENT_OBSERVATION_TOKEN)
    {
        applyResourceDelta(energy, time, 0, BONUS_OBSERVATION_TOKEN);
    }

    if (itemSymbol == ENT_MASTER_KEY)
    {
        objectives[OBJ_MASTER_KEY] = true;
        applyScoreDelta(score, SCORE_KEY_COLLECT);
    }
    else if (itemSymbol == ENT_FUSE)
    {
        objectives[OBJ_FUSE_COLLECTED] = true;
    }
    else if (itemSymbol == ENT_LIFT_PASS)
    {
        objectives[OBJ_LIFT_PASS] = true;
        applyScoreDelta(score, SCORE_LIFT_PASS);

        if (objectives[OBJ_FUSE_INSTALLED])
        {
            objectives[OBJ_LIFT2_ACTIVE] = true;
        }
    }
    else if (itemSymbol == ENT_CABINET_TRAIN_KEY)
    {
        objectives[OBJ_TRAIN_KEY] = true;
        applyScoreDelta(score, SCORE_TRAIN_KEY);
    }
    else if (itemSymbol == ENT_CAT_CARRIER)
    {
        objectives[OBJ_CAT_CARRIER] = true;
    }
    else if (itemSymbol == ENT_PASSENGER)
    {
        objectives[OBJ_PASSENGER_RESCUED] = true;
        applyScoreDelta(score, SCORE_RESCUE_PASSENGER);
        cout << "Passenger rescued." << endl;
    }
    else if (itemSymbol == ENT_CAT)
    {
        objectives[OBJ_CAT_SECURED] = true;
        applyScoreDelta(score, SCORE_SECURE_CAT);
        cout << "Cat secured." << endl;
    }

    cout << "Collected: " << getEntityName(itemSymbol) << endl;
    return true;
}

bool handleUseCommand(char itemSymbol, int currentFloor, int currentRoom,
                      int& energy, int& time, int& score,
                      char inventory[MAX_INVENTORY_CAPACITY], int& inventoryCount,
                      bool objectives[NUM_OBJECTIVES])
{
    if (!hasItem(inventory, inventoryCount, itemSymbol))
    {
        cout << "You do not have that item." << endl;
        return false;
    }

    if (itemSymbol == ENT_FUSE)
    {
        if (currentFloor != 3 || currentRoom != 4)
        {
            cout << "The fuse can only be installed at the control panel." << endl;
            return false;
        }

        if (installFuseControlPanel(inventory, inventoryCount,
                                    objectives, score))
        {
            applyResourceDelta(energy, time,
                               COST_ITEM_USE_ENERGY, COST_ITEM_USE_TIME);

            cout << "Fuse installed." << endl;
            return true;
        }

        cout << "The fuse cannot be installed." << endl;
        return false;
    }
    else if (itemSymbol == ENT_CABINET_TRAIN_KEY)
    {
        if (currentFloor != 5 || currentRoom != 2)
        {
            cout << "You must be at the train boarding platform." << endl;
            return false;
        }

        if (!objectives[OBJ_PASSENGER_RESCUED] ||
            !objectives[OBJ_CAT_SECURED])
        {
            cout << "The train is not ready. Rescue the passenger and secure the cat first." << endl;
            return false;
        }

        applyResourceDelta(energy, time,
                           COST_ITEM_USE_ENERGY, COST_ITEM_USE_TIME);

        objectives[OBJ_TRAIN_PREPARED] = true;
        applyScoreDelta(score, SCORE_PREPARE_TRAIN);

        objectives[OBJ_MISSION_WON] = true;
        applyScoreDelta(score, SCORE_MISSION_VICTORY);

        cout << "The train is prepared and boarded." << endl;
        return true;
    }

    cout << "Nothing happens." << endl;
    return false;
}

bool handleUnlockCabinetCommand(const char* code, int currentFloor, int currentRoom,
                                bool objectives[NUM_OBJECTIVES], int& score,
                                char roomEntities[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM],
                                int& entityCount)
{
    if (currentFloor < 1 || currentFloor > NUM_FLOORS ||
        currentRoom < 1 || currentRoom > ROOMS_PER_FLOOR)
    {
        cout << "Invalid player position." << endl;
        return false;
    }

    if (objectives[OBJ_CABINET_UNLOCKED])
    {
        cout << "The cabinet is already unlocked." << endl;
        return false;
    }

    int f = currentFloor - 1;
    int r = currentRoom - 1;

    bool cabinetHere = false;

    for (int e = 0; e < entityCount; e++)
    {
        if (roomEntities[f][r][e] == ENT_CABINET_TRAIN_KEY)
        {
            cabinetHere = true;
            break;
        }
    }

    if (!cabinetHere)
    {
        cout << "There is no cabinet here." << endl;
        return false;
    }

    if (!verifyCabinetCode(code, objectives))
    {
        cout << "The code is wrong, or you have not found all the clues." << endl;
        return false;
    }

    objectives[OBJ_CABINET_UNLOCKED] = true;
    applyScoreDelta(score, SCORE_CABINET_UNLOCK);

    cout << "Cabinet unlocked." << endl;
    return true;
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

        bool positionValid =
            (playerFloor >= 1 && playerFloor <= NUM_FLOORS &&
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
                    bool hadCard = hasItem(inventory, inventoryCount,
                                           ENT_EMERGENCY_CARD);

                    handleCollectCommand(sym, playerFloor, playerRoom,
                                         energy, time, score, roomEntities,
                                         entityCounts[playerFloor - 1][playerRoom - 1],
                                         inventory, inventoryCount, objectives);

                    if (!hadCard &&
                        hasItem(inventory, inventoryCount, ENT_EMERGENCY_CARD))
                    {
                        emergencyTripsLeft = 3;
                    }
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
                handleUseCommand(sym, playerFloor, playerRoom, energy, time,
                                 score, inventory, inventoryCount, objectives);
            }
        }
        else if (stringEquals(cmd, "unlock"))
        {
            cin.width(20);

            if (cin >> code)
            {
                if (positionValid)
                {
                    if (handleUnlockCabinetCommand(code, playerFloor, playerRoom,
                                                   objectives, score, roomEntities,
                                                   entityCounts[playerFloor - 1][playerRoom - 1]))
                    {
                        applyResourceDelta(energy, time,
                                           COST_ITEM_USE_ENERGY,
                                           COST_ITEM_USE_TIME);
                    }
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
            {
                cout << "Game saved." << endl;
            }
            else
            {
                cout << "Save failed." << endl;
            }
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
        {
            objectives[OBJ_MISSION_FAILED] = true;
        }

        if (checkEndConditions(energy, time, objectives))
        {
            gameOver = true;
            victory = objectives[OBJ_MISSION_WON];

            if (objectives[OBJ_MISSION_FAILED])
            {
                victory = false;
            }
        }
    }

    if (gameOver)
    {
        cout << endl;

        if (victory)
        {
            cout << "Mission won" << endl;
        }
        else
        {
            cout << "Mission failed" << endl;
        }

        writeGameSummaryReport("./out/game_summary.txt",
                               score, energy, time, objectives, victory);
    }
}
