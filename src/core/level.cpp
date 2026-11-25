#include "level.h"
#include "util.h"
#include <fstream>
#include <iostream>
#include <string>

using namespace std;

bool loadBuildingMap(const char* filename,
                     char horizontalConn[NUM_FLOORS][ROOMS_PER_FLOOR - 1],
                     char roomEntities[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM],
                     int entityCounts[NUM_FLOORS][ROOMS_PER_FLOOR])
{
    for (int f = 0; f < NUM_FLOORS; f++)
    {
        for (int r = 0; r < ROOMS_PER_FLOOR - 1; r++)
        {
            horizontalConn[f][r] = ' ';
        }
        for (int r = 0; r < ROOMS_PER_FLOOR; r++)
        {
            entityCounts[f][r] = 0;
            for (int e = 0; e < MAX_ENTITIES_PER_ROOM; e++)
            {
                roomEntities[f][r][e] = ' ';
            }
        }
    }

    ifstream mapFile(filename);
    if (!mapFile.is_open())
    {
        cout << "Error: Could not open " << filename << "\n";
        return false;
    }

    string lines[64];
    int lineCount = 0;
    while (lineCount < 64 && getline(mapFile, lines[lineCount]))
    {
        lineCount++;
    }
    mapFile.close();

    for (int i = 0; i + 2 < lineCount; i++)
    {
        int plusCols[5];
        int plusFound = 0;
        for (int c = 0; c < (int)lines[i].size() && plusFound < 5; c++)
        {
            if (lines[i][c] == '+')
            {
                plusCols[plusFound] = c;
                plusFound++;
            }
        }

        if (plusFound == 5)
        {
            const string& headerLine = lines[i + 1];
            const string& entityLine = lines[i + 2];

            int floorIdx = -1;
            for (int f = 1; f <= NUM_FLOORS; f++)
            {
                string tag = "F" + to_string(f) + "-R1";
                if (headerLine.find(tag) != string::npos)
                {
                    floorIdx = f - 1;
                    break;
                }
            }

            if (floorIdx == -1) continue;

            // Extract horizontal connections (D, W, #, L) between rooms
            for (int r = 0; r < ROOMS_PER_FLOOR - 1; r++)
            {
                int divCol = plusCols[r + 1];
                for (int offset = -1; offset <= 1; offset++)
                {
                    int idx = divCol + offset;
                    if (idx >= 0 && idx < (int)headerLine.size())
                    {
                        char ch = headerLine[idx];
                        if (ch == 'D' || ch == 'W' || ch == '#')
                        {
                            horizontalConn[floorIdx][r] = ch;
                            break;
                        }
                        if (ch == 'L' && (idx + 1 >= (int)headerLine.size() || headerLine[idx + 1] != '1'))
                        {
                            horizontalConn[floorIdx][r] = 'L';
                            break;
                        }
                    }
                }
            }

            // Extract entity symbols inside each room's column bounds
            for (int r = 0; r < ROOMS_PER_FLOOR; r++)
            {
                int startCol = plusCols[r] + 1;
                int endCol = plusCols[r + 1];
                for (int c = startCol; c < endCol && c < (int)entityLine.size(); c++)
                {
                    char ch = entityLine[c];
                    if (ch != ' ' && ch != '|' && ch != '\r' && ch != '\n')
                    {
                        int count = entityCounts[floorIdx][r];
                        if (count < MAX_ENTITIES_PER_ROOM)
                        {
                            roomEntities[floorIdx][r][count] = ch;
                            entityCounts[floorIdx][r]++;
                        }
                    }
                }
            }
        }
    }

    return true;
}
bool loadInitialState(const char* filename,
                      int& playerFloor, int& playerRoom,
                      int& energy, int& time, int& score,
                      char inventory[MAX_INVENTORY_CAPACITY], int& inventoryCount,
                      bool objectives[NUM_OBJECTIVES], int& emergencyTripsLeft)
{
    ifstream stateFile(filename);
    if (!stateFile.is_open())
    {
        cout << "Error: Could not open " << filename << "\n";
        return false;
    }

    playerFloor = 1;
    playerRoom = 1;
    energy = INITIAL_ENERGY;
    time = INITIAL_TIME;
    score = INITIAL_SCORE;
    inventoryCount = 0;
    emergencyTripsLeft = 0;

    for (int i = 0; i < MAX_INVENTORY_CAPACITY; i++)
    {
        inventory[i] = ' ';
    }
    for (int i = 0; i < NUM_OBJECTIVES; i++)
    {
        objectives[i] = false;
    }

    string key, val;
    while (stateFile >> key >> val)
    {
        if (key == "INITIAL_ENERGY:") energy = stoi(val);
        else if (key == "INITIAL_TIME:") time = stoi(val);
        else if (key == "INITIAL_SCORE:") score = stoi(val);
    }

    stateFile.close();
    return true;
}

bool validateStartingRoom(int startFloor, int startRoom,
                          const char horizontalConn[NUM_FLOORS][ROOMS_PER_FLOOR - 1])
{
    (void)horizontalConn;
    return (startFloor >= 1 && startFloor <= NUM_FLOORS &&
            startRoom >= 1 && startRoom <= ROOMS_PER_FLOOR);
}

const char* getEntityName(char entitySymbol)
{
    switch (entitySymbol)
    {
        case '<': return "Master Key";
        case ')': return "Energy Pack";
        case '\\': return "Emergency Toolkit";
        case '4': return "Clue Digit 4";
        case '*': return "Fuse";
        case '~': return "Stray Cat";
        case ';': return "Control Panel";
        case '(': return "Lift Pass";
        case '!': return "Injured Passenger";
        case '%': return "Medical Kit";
        case '$': return "Emergency Access Card";
        case '2': return "Clue Digit 2";
        case '7': return "Clue Digit 7";
        case '^': return "Security Cabinet";
        case '@': return "Cat Carrier";
        case '&': return "Observation Token";
        default:  return "Unknown";
    }
}