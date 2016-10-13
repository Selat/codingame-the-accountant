#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <climits>
#include <list>
#include <cmath>
#include <cassert>
#include <chrono>

#include "world.hpp"

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
    if (depth == 0 && world.enemies.size() < 20) {
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
