#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <climits>

int main() {
  while (true) {
    int x;
    int y;
    std::cin >> x >> y; std::cin.ignore();
    int dataCount;
    std::cin >> dataCount; std::cin.ignore();
    for (int i = 0; i < dataCount; i++) {
      int dataId;
      int dataX;
      int dataY;
      std::cin >> dataId >> dataX >> dataY; std::cin.ignore();
    }
    int enemyCount;
    std::cin >> enemyCount; std::cin.ignore();
    int min_dist = INT_MAX;
    int min_id = -1;
    for (int i = 0; i < enemyCount; i++) {
      int enemyId;
      int enemyX;
      int enemyY;
      int enemyLife;
      std::cin >> enemyId >> enemyX >> enemyY >> enemyLife; std::cin.ignore();
      int d = (x - enemyX) * (x - enemyX) + (y - enemyY) * (y - enemyY);
      if (d < min_dist) {
        min_dist = d;
        min_id = enemyId;
      }
    }

    std::cout << "SHOOT " << min_id << std::endl; // MOVE x y or SHOOT id
    //std::cout << "MOVE 5000 5000" << std::endl;
  }
}
