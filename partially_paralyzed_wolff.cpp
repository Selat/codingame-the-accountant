#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <climits>

int main() {
  int it_id = 0;
  while (true) {
    ++it_id;
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

    if (it_id < 5) {
      std::cout << "MOVE 8000 4000" << std::endl;
    } else {
      std::cout << "SHOOT " << min_id << std::endl;
    }
  }
}
