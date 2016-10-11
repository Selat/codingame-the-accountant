#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <climits>

using namespace std;

/**
 * Shoot enemies before they collect all the incriminating data!
 * The closer you are to an enemy, the more damage you do but don't get too close or you'll get killed.
 **/
int main()
{

    // game loop
    while (1) {
        int x;
        int y;
        cin >> x >> y; cin.ignore();
        cerr << x << " " << y << endl;
        int dataCount;
        cin >> dataCount; cin.ignore();
        cerr << dataCount << endl;
        for (int i = 0; i < dataCount; i++) {
            int dataId;
            int dataX;
            int dataY;
            cin >> dataId >> dataX >> dataY; cin.ignore();
            cerr << dataId << " " << dataX << " " << dataY << endl;
        }
        int enemyCount;
        cin >> enemyCount; cin.ignore();
        cerr << enemyCount << endl;
        int min_dist = INT_MAX;
        int min_id = -1;
        int min_life = -1;
        for (int i = 0; i < enemyCount; i++) {
            int enemyId;
            int enemyX;
            int enemyY;
            int enemyLife;
            cin >> enemyId >> enemyX >> enemyY >> enemyLife; cin.ignore();
            cerr << enemyId << " " << enemyX << " " << enemyY << " " << enemyLife << endl;
            int d = (x - enemyX) * (x - enemyX) + (y - enemyY) * (y - enemyY);
            if (d < min_dist) {
                min_dist = d;
                min_id = enemyId;
                min_life = enemyLife;
            }
        }
        cout << "ERR" << endl;
    }
}
