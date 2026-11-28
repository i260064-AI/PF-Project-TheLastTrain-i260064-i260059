#include "viewer_iface.h"
#include "util.h"
#include "level.h"
#include <iostream>
using namespace std;

// Terminal UI and dashboard rendering using primitive parameters

void renderDashboard(int playerFloor, int playerRoom, int energy, int time, int score,
                     const char inventory[MAX_INVENTORY_CAPACITY], int inventoryCount,
                     int emergencyTripsLeft)
                     {
                        cout << "Location: F" << playerFloor << "-R" << playerRoom << " | Energy: " << energy << "/" << MAX_ENERGY << " | Time: " << time << "/" << MAX_TIME << " | Score: " << score << endl;
                        cout << "Emergency Lift Trips Left: " << emergencyTripsLeft << endl;

                        printHeader("THE LAST TRAIN");

                        int safeCount = clampInt(inventoryCount, 0, MAX_INVENTORY_CAPACITY);

                        cout << "Inventory: ";
                        if (safeCount == 0)
                        {
                            cout << "[Empty]";
                        }
                        else
                        {
                            for (int i = 0 ; i < safeCount ; i++)
                            {
                                char item = inventory[i];
                                cout << item << " (" << getEntityName(item) << ")";

                                if (i < safeCount - 1)
                                {
                                    cout << ", ";
                                }
                            }
                        }
                        cout << endl;
                     }

void renderRoomDetails(int floor, int room,
                       const char roomEntities[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM],
                       int entityCount)
                       {
                            int fIdx = floor - 1;
                            int rIdx = room - 1;

                            if (fIdx < 0 || fIdx >= NUM_FLOORS || rIdx < 0 || rIdx >= ROOMS_PER_FLOOR)
                            {
                                cout << "Invalid floor or room number." << endl;
                                return;
                            }

                            int safeCount = clampInt(entityCount, 0, MAX_ENTITIES_PER_ROOM);

                            cout << "Room F" << floor << "-R" << room << " Entities: ";

                            if (safeCount == 0)
                            {
                                cout << "[Empty]";
                            }
                            else
                            {
                                for (int i = 0; i < safeCount; i++)
                                {
                                    char entity = roomEntities[fIdx][rIdx][i];
                                    cout << entity << " (" << getEntityName(entity) << ")";

                                    if (i < safeCount - 1)
                                    {
                                        cout << ", ";
                                    }
                                }
                            }
                            cout << endl;
                       }

void renderAsciiBuilding(int playerFloor, int playerRoom)
{
    
}
