#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <climits>
#include <list>
#include <cmath>
#include <cassert>
#include <chrono>

constexpr int kEnemyRange = 2000;

struct Enemy;
struct DataPoint;
struct Wolff;
struct World;
struct Vector2D;
std::ostream& operator<<(std::ostream& out, const Vector2D& v);

struct Vector2D {
  Vector2D() {}
  Vector2D(int _x, int _y) : x(_x), y(_y) {}
  int dist2(const Vector2D& v) const {
    return (x - v.x) * (x - v.x) + (y - v.y) * (y - v.y);
  }
  bool operator==(const Vector2D& v) const {
    return x == v.x && y == v.y;
  }
  void move(const Vector2D& v) {
    int d2 = dist2(v);
    if (d2 <= speed * speed) {
      x = v.x;
      y = v.y;
    } else {
      double d = sqrt(d2);
      double dx = v.x - x;
      double dy = v.y - y;
      x += floor(dx * speed / d);
      y += floor(dy * speed / d);
    }
  }
  int speed;
  int x, y;
};

std::ostream& operator<<(std::ostream& out, const Vector2D& v) {
  return out << "(" << v.x << " " << v.y << ")";
}

struct DataPoint {
  DataPoint() {}
  DataPoint(int _id, int x, int y) : id(_id), pos(x, y) {}
  int id;
  Vector2D pos;
};

struct Wolff {
  Wolff() {
    pos.speed = 1000;
    action = 0;
  }
  void shoot(int dp_id) {
    target_id = dp_id;
    action = 0;
  }
  void move(const Vector2D& pos) {
    target_pos = pos;
    action = 1;
  }
  bool isShooting() {
    return action == 0;
  }
  bool isMoving() {
    return action == 1;
  }
  Vector2D pos;
  int target_id;
  int action;
  Vector2D target_pos;
};

struct Enemy {
  Enemy();
  Enemy(int _id, int x, int y, int _life_points);
  void move(const World& world);
  int id;
  Vector2D pos;
  int life_points;
};


struct World {
  World();
  void Init();
  void step();
  int FindNearestDataPoint(const Vector2D& pos);
  int FindNearestEnemy(const Vector2D& pos) const;
  bool IsGameOver() const;
  void CalculateBonus();
  bool IsEnemyAlive(int id) const {
    for (const auto& enemy : enemies) {
      if (enemy.id == id) {
        return true;
      }
    }
    return false;
  }
  Wolff wolff;
  std::list<Enemy> enemies;
  std::list<DataPoint> data_points;
  int score;
  bool is_wolff_killed;
  int initial_life_points_sum;
  int shots_num;
};

Enemy::Enemy() {
  pos.speed = 500;
}

Enemy::Enemy(int _id, int x, int y, int _life_points)
    : id(_id), pos(x, y), life_points(_life_points) {
  pos.speed = 500;
}

void Enemy::move(const World& world) {
  int min_dist = INT_MAX;
  auto min_it = world.data_points.begin();
  for (auto it = world.data_points.begin(); it != world.data_points.end(); ++it) {
    int cur_dist = pos.dist2(it->pos);
    if (cur_dist < min_dist) {
      min_dist = cur_dist;
      min_it = it;
    }
  }
  assert(min_it != world.data_points.end());
  pos.move(min_it->pos);
}

World::World() : score(0), is_wolff_killed(false), initial_life_points_sum(0), shots_num(0) {}

void World::step() {
  // 1. Enemies move towards their targets.
  for (auto& enemy : enemies) {
    enemy.move(*this);
  }

  // 2. If a MOVE command was given, Wolff moves towards his target.
  if (wolff.isMoving()) {
    wolff.pos.move(wolff.target_pos);
  }

  // 3. Game over if an enemy is close enough to Wolff.
  for (const auto& enemy : enemies) {
    if (wolff.pos.dist2(enemy.pos) <= kEnemyRange * kEnemyRange) {
      is_wolff_killed = true;
      break;
    }
  }
  if (is_wolff_killed) {
    score = 0;
    return;
  }

  // 4. If a SHOOT command was given, Wolff shoots an enemy.
  if (wolff.isShooting()) {
    ++shots_num;
    for (auto& enemy : enemies) {
      if (enemy.id == wolff.target_id) {
        int damage = round(125000.0 / pow(sqrt(wolff.pos.dist2(enemy.pos)), 1.2));
        enemy.life_points = std::max(0, enemy.life_points - damage);
      }
    }
  }

  // 5. Enemies with zero life points are removed from play.
  for (auto it = enemies.begin(); it != enemies.end();) {
    if (it->life_points == 0) {
      it = enemies.erase(it);
      score += 10;
    } else {
      ++it;
    }
  }
  if (enemies.empty()) {
    CalculateBonus();
  }

  // 6. Enemies collect data points they share coordinates with.
  for (auto it = data_points.begin(); it != data_points.end();) {
    bool is_collected = false;
    for (const auto& enemy : enemies) {
      if (enemy.pos == it->pos) {
        is_collected = true;
        score -= 100;
        break;
      }
    }
    if (is_collected) {
      it = data_points.erase(it);
    } else {
      ++it;
    }
  }

  if (data_points.empty()) {
    CalculateBonus();
  }
}

int World::FindNearestDataPoint(const Vector2D& pos) {
  auto min_it = data_points.begin();
  int min_dist = INT_MAX;
  for (auto it = data_points.begin(); it != data_points.end(); ++it) {
    int cur_dist = pos.dist2(it->pos);
    if (cur_dist < min_dist) {
      min_dist = cur_dist;
      min_it = it;
    }
  }
  return min_it->id;
}

int World::FindNearestEnemy(const Vector2D& pos) const {
  auto min_it = enemies.begin();
  int min_dist = INT_MAX;
  for (auto it = enemies.begin(); it != enemies.end(); ++it) {
    int cur_dist = pos.dist2(it->pos);
    if (cur_dist < min_dist) {
      min_dist = cur_dist;
      min_it = it;
    }
  }
  assert(min_it != enemies.end());
  return min_it->id;
}

bool World::IsGameOver() const {
  return enemies.empty() || data_points.empty() || is_wolff_killed;
}

void World::CalculateBonus() {
  score += data_points.size() * std::max(0, initial_life_points_sum - 3 * shots_num) * 3;
}

void World::Init() {
  initial_life_points_sum = 0;
  for (const auto& e : enemies) {
    initial_life_points_sum += e.life_points;
  }
  shots_num = 0;
  is_wolff_killed = false;
  score = data_points.size() * 100;
}

Vector2D GetDangerousEnemyPos(const World& world) {
  Vector2D pos = world.enemies.front().pos;
  for (auto enemy : world.enemies) {
    enemy.move(world);
    if (world.wolff.pos.dist2(enemy.pos) < world.wolff.pos.dist2(pos)) {
      pos = enemy.pos;
    }
  }
  return pos;
}

int GetFinalScore(World& world) {
  while (!world.IsGameOver()) {
    Vector2D pos = GetDangerousEnemyPos(world);
    if (world.wolff.pos.dist2(pos) <= kEnemyRange * kEnemyRange) {
      const auto& wolff = world.wolff;
      Vector2D target(wolff.pos.x + (wolff.pos.x - pos.x), wolff.pos.y + (wolff.pos.y - pos.y));
      if (target.x < 0 || target.x >= 16000 || target.y < 0 || target.y >= 9000) {
        world.wolff.shoot(world.FindNearestEnemy(world.wolff.pos));
      } else {
        world.wolff.move(target);
      }
    } else {
      world.wolff.shoot(world.FindNearestEnemy(world.wolff.pos));
    }
    world.step();
  }
  return world.score;
}

int GetBestMove(const World& world, Vector2D& pos, int depth = 0) {
  if (world.IsGameOver()) {
    return world.score;
  }
  int kAngleStepsNum = 8;
  if (world.enemies.size() > 20) {
    kAngleStepsNum = 4;
  }
  int max_score = INT_MIN;
  for (int i = 0; i < kAngleStepsNum; ++i) {
    double angle = i * 2.0 * M_PI / kAngleStepsNum;
    double dx = 1000.0 * cos(angle);
    double dy = 1000.0 * sin(angle);
    Vector2D next_pos = world.wolff.pos;
    next_pos.x += dx;
    next_pos.y += dy;
    if (next_pos.x < 0 || next_pos.x >= 16000 || next_pos.y < 0 || next_pos.y >= 9000) {
      continue;
    }
    World test_world = world;
    test_world.wolff.move(next_pos);
    test_world.step();
    World test_world2 = test_world;
    int cur_score = GetFinalScore(test_world);
    if (depth == 0) {
      Vector2D tmp;
      cur_score = std::max(GetBestMove(test_world2, tmp, 1), cur_score);
    }
    if (cur_score > max_score) {
      max_score = cur_score;
      pos = next_pos;
    }
  }
  return max_score;
}

int GetBestShoot(const World& world, int& id) {
  World test_world = world;
  id = world.FindNearestEnemy(world.wolff.pos);
  int max_score = GetFinalScore(test_world);
  if (world.enemies.size() > 20) return max_score;
  for (const auto& enemy : world.enemies) {
    World test_world = world;
    while (!test_world.IsGameOver() && test_world.IsEnemyAlive(enemy.id)) {
      test_world.wolff.shoot(enemy.id);
      test_world.step();
    }
    int cur_score = GetFinalScore(test_world);
    if (cur_score > max_score) {
      max_score = cur_score;
      id = enemy.id;
    }
  }
  return max_score;
}

int main() {
  while (true) {
    int x;
    int y;
    std::cin >> x >> y; std::cin.ignore();
    World world;
    world.wolff.pos.x = x;
    world.wolff.pos.y = y;
    int dataCount;
    std::cin >> dataCount; std::cin.ignore();
    for (int i = 0; i < dataCount; i++) {
      int dataId;
      int dataX;
      int dataY;
      std::cin >> dataId >> dataX >> dataY; std::cin.ignore();
      world.data_points.push_back(DataPoint(dataId, dataX, dataY));
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
      world.enemies.push_back(Enemy(enemyId, enemyX, enemyY, enemyLife));
      int d = (x - enemyX) * (x - enemyX) + (y - enemyY) * (y - enemyY);
      if (d < min_dist) {
        min_dist = d;
        min_id = enemyId;
      }
    }

    world.Init();

    auto start = std::chrono::high_resolution_clock::now();
    Vector2D best_move;
    int move_score = GetBestMove(world, best_move);
    World test_world = world;
    int best_target = -1;
    int shoot_score = GetBestShoot(test_world, best_target);
    auto end = std::chrono::high_resolution_clock::now();
    //std::cerr << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << std::endl;

    //std::cerr << move_score << " " << shoot_score << std::endl;
    if (move_score > shoot_score) {
      std::cout << "MOVE " << best_move.x << " " << best_move.y << std::endl;
    } else {
      Vector2D pos = GetDangerousEnemyPos(world);
      if (world.wolff.pos.dist2(pos) <= kEnemyRange * kEnemyRange) {
        const auto& wolff = world.wolff;
        Vector2D target(wolff.pos.x + (wolff.pos.x - pos.x), wolff.pos.y + (wolff.pos.y - pos.y));
        std::cout << "MOVE " << target.x << " " << target.y << std::endl;
      } else {
        std::cout << "SHOOT " << best_target << std::endl;
      }
    }
  }
}
