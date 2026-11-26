#include "util.h"
#include <iostream>
#include <iomanip>
using namespace std;

// Primitive helper utilities, clamping, and screen control

int clampInt(int val, int minVal, int maxVal)
{
    if (val < minVal)
        return minVal;
    else if (val > maxVal)
        return maxVal;
    else
        return val;
}

bool stringEquals(const char* a, const char* b)
{
    if (a == nullptr || b == nullptr)
    {
        return false;
    }

    int i = 0;

    for ( ; a[i] != '\0' && b[i] != '\0'; i++)
    {
        if (a[i] != b[i])
        {
            return false;
        }
    }
    return a[i] == b[i];
}

void trimWhitespace(char* str)
{
    if (str == nullptr)
    {
        return;
    }

    int start = 0;
    while (str[start] == ' ' || str[start] == '\t' || str[start] == '\n' || str[start] == '\r')
    {
        start++;
    }

    int i = 0;
    while (str[start + i] != '\0')
    {
        str[i] = str[start + i];
        i++;
    }
    str[i] = '\0';

    while (i > 0 && (str[i - 1] == ' ' || str[i - 1] == '\t' || str[i - 1] == '\n' || str[i - 1] == '\r'))
    {
        str[i - 1] = '\0';
        i--;
    }
}

void printHeader(const char* title)
{
    if (title == nullptr)
    {
        return;
    }

    cout << setfill('=') << setw(40) << "" << "\n";
    cout << setfill(' ');
    cout << "  " << title << "\n";
    cout << setfill('=') << setw(40) << "" << "\n";
    cout << setfill(' ');
}

void clearScreen()
{
    cout << "\033[2J";
    cout << "\033[1;1H";
}

