#include "switches.h"
#include "util.h"

// Door lock mechanics, lifts, and cabinet verification without structures

#include "physics.h"

bool canUseLift1(int fromFloor, int toFloor)
{
    if (fromFloor < 1 || fromFloor > NUM_FLOORS || toFloor < 1 || toFloor > NUM_FLOORS)
    {
        return false;
    }

    if ((fromFloor == 1 && toFloor == 2) || (fromFloor == 2 && toFloor == 1))
    {
        return true;
    }

    return false;
}

bool canUseLift2(int fromFloor, int toFloor, bool lift2Operational, int emergencyTripsLeft)
{
    if (fromFloor < 1 || fromFloor > NUM_FLOORS || toFloor < 1 || toFloor > NUM_FLOORS)
    {
        return false;
    }

    if (fromFloor == toFloor)
    {
        return false;
    }

    bool validConnection = false;

    if ((fromFloor == 2 && toFloor == 4) ||
        (fromFloor == 4 && toFloor == 2) ||
        (fromFloor == 2 && toFloor == 5) ||
        (fromFloor == 5 && toFloor == 2) ||
        (fromFloor == 4 && toFloor == 5) ||
        (fromFloor == 5 && toFloor == 4))
    {
        validConnection = true;
    }

    if (!validConnection)
    {
        return false;
    }

    return (lift2Operational || emergencyTripsLeft > 0);
}

bool unlockFloor3Door(const char inventory[MAX_INVENTORY_CAPACITY], int inventoryCount)
{
    return hasItem(inventory, inventoryCount, ENT_MASTER_KEY);
}

bool installFuseControlPanel(char inventory[MAX_INVENTORY_CAPACITY], int& inventoryCount,
                             bool objectives[NUM_OBJECTIVES], int& score)
{
    if (!objectives[OBJ_FUSE_COLLECTED])
    {
        return false;
    }

    if (!removeItem(inventory, inventoryCount, ENT_FUSE))
    {
        return false;
    }

    objectives[OBJ_FUSE_INSTALLED] = true;

    if (objectives[OBJ_LIFT_PASS])
    {
        objectives[OBJ_LIFT2_ACTIVE] = true;
    }

    applyScoreDelta(score, SCORE_INSTALL_FUSE);

    return true;
}

bool verifyCabinetCode(const char* enteredCode, const bool objectives[NUM_OBJECTIVES])
{
    if (enteredCode == nullptr)
    {
        return false;
    }

    if (!objectives[OBJ_CLUE_4] || !objectives[OBJ_CLUE_2] || !objectives[OBJ_CLUE_7])
    {
        return false;
    }

    return stringEquals(enteredCode, "427");
}