#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <climits>
#include <list>
#include <cmath>

struct Vector2D {
  Vector2D() {}
  Vector2D(int _x, int _y) : x(_x), y(_y) {}
  int speed;
  int dist2(const Vector2D& v) const {
    return (x - v.x) * (x - v.x) + (y - v.y) * (y - v.y);
  }
  void move(const Vector2D& v) {
    double d = sqrt(dist2(v));
    double dx = v.x - x;
    double dy = v.y - y;
    double c = speed / d;
    x += int(dx * c);
    y += int(dy * c);
  }
  int x, y;
};

struct Enemy {
  Enemy() {}
  Enemy(int _id, int x, int y, int _life_points) : id(_id), pos(x, y), life_points(_life_points) {}
  int id;
  Vector2D pos;
  int life_points;
};

struct DataPoint {
  DataPoint() {}
  DataPoint(int _id, int x, int y) : id(_id), pos(x, y) {}
  int id;
  Vector2D pos;
};

struct WorldState {
  Vector2D wolff_pos;
  std::list<Enemy> enemies;
  std::list<DataPoint> data_points;
  long long euristics_score;
  bool operator<(const WorldState& state) const {
    return euristics_score < state.euristics_score;
  }
};

void moveEnemies(WorldState& state) {
  for (auto& enemy : state.enemies) {
    auto min_it = state.data_points.begin();
    int min_dist = INT_MAX;
    for (auto it = state.data_points.begin(); it != state.data_points.end(); ++it) {
      int dist = enemy.pos.dist2(it->pos);
      if (dist < min_dist) {
        min_it = it;
        min_dist = dist;
      }
    }
  }
}

std::vector<WorldState> moveToDataPoints(WorldState state) {
  std::vector<WorldState> res;
  moveEnemies(state);
  for (const auto& dp : state.data_points) {
    WorldState new_state = state;
    new_state.wolff_pos.move(dp.pos);
  }
  return res;
}

std::vector<WorldState> retreatFromEnemy(WorldState state) {
  std::vector<WorldState> res;
  return res;
}

int calculateDamage(const Vector2D& p1, const Vector2D& p2) {
  double dist = sqrt(p1.dist2(p2));
  double dmg = 125000.0 / (pow(dist, 1.2));
  return dmg;
}

std::vector<WorldState> shootEnemy(WorldState state) {
  std::vector<WorldState> res;
  moveEnemies(state);
  for (auto& enemy : state.enemies) {
    int dmg = calculateDamage(state.wolff_pos, enemy.pos);
    std::cout << dmg << std::endl;
    int tmp = enemy.life_points;
    enemy.life_points = std::max(0, enemy.life_points - dmg);
    res.push_back(state);
    enemy.life_points = tmp;
  }
  return res;
}

void Search(WorldState state) {
  int k = 1;
  constexpr int kMaxStates = 200;
  std::vector<WorldState> states;
  while (k--) {
    std::vector<WorldState> new_states;
    for (const auto& s : states) {
      auto dp_moves = moveToDataPoints(s);
      new_states.insert(new_states.end(), dp_moves.begin(), dp_moves.end());
      auto enemy_moves = retreatFromEnemy(s);
      new_states.insert(new_states.end(), enemy_moves.begin(), enemy_moves.end());
      auto shoots = shootEnemy(s);
      new_states.insert(new_states.end(), shoots.begin(), shoots.end());
    }
    std::sort(new_states.begin(), new_states.end());
    int new_num = std::min(kMaxStates, (int)new_states.size());
    states.assign(new_states.begin(), new_states.begin() + new_num);
  }
}

int main() {
  while (true) {
    int x;
    int y;
    WorldState state;
    std::cin >> x >> y; std::cin.ignore();
    state.wolff_pos.x = x;
    state.wolff_pos.y = y;
    state.wolff_pos.speed = 1000;
    int dataCount;
    std::cin >> dataCount; std::cin.ignore();
    for (int i = 0; i < dataCount; i++) {
      int dataId;
      int dataX;
      int dataY;
      std::cin >> dataId >> dataX >> dataY; std::cin.ignore();
      state.data_points.push_back(DataPoint(dataId, dataX, dataY));
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
      state.enemies.push_back(Enemy(enemyId, enemyX, enemyY, enemyLife));
      int d = (x - enemyX) * (x - enemyX) + (y - enemyY) * (y - enemyY);
      if (d < min_dist) {
        min_dist = d;
        min_id = enemyId;
      }
    }

    //Search(state);

    std::cout << "SHOOT " << min_id << std::endl; // MOVE x y or SHOOT id
    //std::cout << "MOVE 5000 5000" << std::endl;
  }
}
