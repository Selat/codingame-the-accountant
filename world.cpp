#include "world.hpp"

#include <climits>
#include <cassert>

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
    if (wolff.target_pos.x < 0 || wolff.target_pos.x >= 16000
        || wolff.target_pos.y < 0 || wolff.target_pos.y >= 9000) {
      score = 0;
      is_wolff_killed = true;
      return;
    }
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
    bool is_enemy_found = false;
    for (auto& enemy : enemies) {
      if (enemy.id == wolff.target_id) {
        is_enemy_found = true;
        ++shots_num;
        int damage = round(125000.0 / pow(sqrt(wolff.pos.dist2(enemy.pos)), 1.2));
        enemy.life_points = std::max(0, enemy.life_points - damage);
      }
    }
    if (!is_enemy_found) {
      score = 0;
      is_wolff_killed = true;
      return;
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


std::ostream& operator<<(std::ostream& out, const Vector2D& v) {
  return out << "(" << v.x << " " << v.y << ")";
}
