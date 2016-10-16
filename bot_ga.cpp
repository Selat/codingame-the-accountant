#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <climits>
#include <cmath>
#include <cassert>
#include <chrono>
#include <array>

#include "world.hpp"

constexpr int kGenomeSize = 1;
constexpr int kMovesNum = 4;
constexpr int kPopulationSize = 100;
constexpr double kMutationPercentage = 1.0;
constexpr double kRecombinationsPercentage = 1.0;

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
  int fine = 0;
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
    //fine += 10;
  }
  return world.score - fine;
}

Vector2D ConvertMove(const Vector2D& pos, int move_id) {
  double angle = move_id * 2.0 * M_PI / kMovesNum;
  double dx = 1000.0 * cos(angle);
  double dy = 1000.0 * sin(angle);
  Vector2D next_pos = pos;
  next_pos.x += dx;
  next_pos.y += dy;
  return next_pos;
}

struct GameMove {
  GameMove() : type(MOVE), target_id(0), move_id(0) {
  }
  void GenerateRandom(const World& world) {
    int id = (rand() % (kMovesNum + 1));//+ world.enemies.size()));
    //std::cerr << id << std::endl;
    if (id < kMovesNum) {
      type = MOVE;
      move_id = id;
    } else {
      type = SHOOT;
      target_id = id - kMovesNum;
    }
  }
  void Apply(World& world) const {
    if (world.IsGameOver()) return;
    if (type == MOVE) {
      Vector2D next_pos = ConvertMove(world.wolff.pos, move_id);
      world.wolff.move(next_pos);
      world.step();
    } else {
      GetFinalScore(world);
      //world.wolff.shoot(target_id);
    }
    //world.step();
  }
  enum Type {MOVE, SHOOT} type;
  int target_id;
  int move_id;
};

struct Genome {
  Genome() : score(0) {
  }
  void GenerateRandom(const World& world) {
    for (auto& move : moves) {
      move.GenerateRandom(world);
    }
  }
  void Recombine(const Genome& g) {
    int mid = (rand() % (kGenomeSize - 2)) + 1;
    for (int i = mid; i < kGenomeSize; ++i) {
      moves[i] = g.moves[i];
    }
  }
  void Rescore(World world) {
    for (const auto& move : moves) {
      if (world.IsGameOver()) break;
      move.Apply(world);
    }
    score = GetFinalScore(world);
  }
  void Mutate(const World& world) {
    moves[rand() % moves.size()].GenerateRandom(world);
    //moves[rand() % kGenomeSize].GenerateRandom(world, 1);
  }
  int Score() const {
    return score;
  }
  bool operator<(const Genome& g) const {
    return score > g.score;
  }
  int score;
  std::array<GameMove, kGenomeSize> moves;
};

struct Population {
  Population(const World& world) {
    for (int i = 0; i < kPopulationSize; ++i) {
      auto genome = Genome();
      genome.GenerateRandom(world);
      genomes.push_back(std::move(genome));
    }
    for (int i = 0; i < kPopulationSize; ++i) {
      genomes[i].Rescore(world);
    }
  }
  void GenerateNext(const World& world) {
    int n = genomes.size();
    int mutants_num = genomes.size() * kMutationPercentage;
    int recombinations_num = n * kRecombinationsPercentage;
    for (int i = 0; i < mutants_num; ++i) {
      Genome new_genome = genomes[rand() % n];
      new_genome.Mutate(world);
      new_genome.Rescore(world);
      genomes.push_back(std::move(new_genome));
    }
    for (int i = 0; i < recombinations_num; ++i) {
      Genome new_genome = genomes[rand() % (n / 2)];
      new_genome.Recombine(genomes[(rand() % (n / 2))+ n / 2]);
      new_genome.Rescore(world);
      genomes.push_back(std::move(new_genome));
    }
    std::sort(genomes.begin(), genomes.end());
    genomes.resize(kPopulationSize);
  }
  GameMove GetBestMove(int& score) {
    int max_score = INT_MIN;
    GameMove best_move;
    for (const auto& genome : genomes) {
      if (genome.Score() > max_score) {
        max_score = genome.Score();
        best_move = genome.moves[0];
      }
    }
    score = max_score;
    //std::cerr << max_score << std::endl;
    return best_move;
  }
  std::vector<Genome> genomes;
};

int main() {
  srand(42);
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
    //std::cerr << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << std::endl;

    //std::cerr << move_score << " " << shoot_score << std::endl;
    auto start = std::chrono::high_resolution_clock::now();
    Population population(world);
    auto end = std::chrono::high_resolution_clock::now();
    int pid = 0;
    while (std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() < 80 && pid < 1000) {
      population.GenerateNext(world);
      end = std::chrono::high_resolution_clock::now();
      ++pid;
    }
    int ga_score;
    GameMove best_move = population.GetBestMove(ga_score);
    World test_world;
    int stand_score = GetFinalScore(test_world);
    if (ga_score > stand_score) {
      //std::cerr << "ok! " << best_move.move_id << std::endl;
      if (best_move.type == GameMove::SHOOT) {
        int id = 0;
        best_move.target_id %= world.enemies.size();
        for (const auto& enemy : world.enemies) {
          if (id == best_move.target_id) {
            std::cout << "SHOOT " << enemy.id << std::endl;
          }
          ++id;
        }
      } else {
        auto move = ConvertMove(world.wolff.pos, best_move.move_id);
        if (move.x < 0 || move.x >= 16000 || move.y < 0 || move.y >= 9000) {
          std::cout << "SHOOT " << world.FindNearestEnemy(world.wolff.pos) << std::endl;
        } else {
          std::cout << "MOVE " << move.x << " " << move.y << std::endl;
        }
      }
    } else {
      Vector2D pos = GetDangerousEnemyPos(world);
      if (world.wolff.pos.dist2(pos) <= kEnemyRange * kEnemyRange) {
        const auto& wolff = world.wolff;
        Vector2D target(wolff.pos.x + (wolff.pos.x - pos.x), wolff.pos.y + (wolff.pos.y - pos.y));
        if (target.x < 0 || target.x >= 16000 || target.y < 0 || target.y >= 9000) {
          std::cout << "SHOOT " << world.FindNearestEnemy(world.wolff.pos) << std::endl;
        } else {
          std::cout << "MOVE " << target.x << " " << target.y << std::endl;
        }
      } else {
        std::cout << "SHOOT " << world.FindNearestEnemy(world.wolff.pos) << std::endl;
      }
    }
  }
}
