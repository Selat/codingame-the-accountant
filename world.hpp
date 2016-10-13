#ifndef WORLD_H
#define WORLD_H

#include <cmath>
#include <list>
#include <iostream>

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

#endif
