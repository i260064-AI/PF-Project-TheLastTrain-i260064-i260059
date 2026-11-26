#include "viewer_iface.h"
#include "util.h"

// Terminal UI and dashboard rendering using primitive parameters

void renderDashboard(int playerFloor, int playerRoom, int energy, int time, int score,
                     const char inventory[MAX_INVENTORY_CAPACITY], int inventoryCount,
                     int emergencyTripsLeft)
                     {
                        printHeader("THE LAST TRAIN");

                        int safeCount = clampInt(inventoryCount, 0, MAX_INVENTORY_CAPACITY);

                        cout << "Inventory: ";
                        if (safeCount == 0)
                        {
                            cout << "[Empty]";
                        }

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