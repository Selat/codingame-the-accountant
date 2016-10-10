#!/usr/bin/env python

import time
import math
import subprocess
import sys

class Unit:
    def __init__(self, x, y, speed):
        self.x = x
        self.y = y
    def move(self, x, y):
        d = (x - self.x) * (x - self.x) + (y - self.y) * (y - self.y)
        if d <= self.SPEED * self.SPEED:
            self.x = x
            self.y = y
        else:
            d = math.sqrt(d)
            dx = x - self.x
            dy = y - self.y
            c = self.SPEED / d
            self.x += math.floor(dx * c)
            self.y += math.floor(dy * c)

class Enemy(Unit):
    SPEED = 500
    RANGE = 2000
    def __init__(self, x, y, life_points):
        Unit.__init__(self, x, y, Enemy.SPEED)
        self.life_points = life_points

    def move(self, world):
        min_dist = 10 ** 9
        target = None
        for p in world.get_data_points():
            cur_dist = (p.x - self.x) * (p.x - self.x) + (p.y - self.y) * (p.y - self.y)
            if cur_dist < min_dist:
                min_dist = cur_dist
                target = p
        Unit.move(self, target.x, target.y)
        #time.sleep(1)

    def damage(self, damage):
        self.life_points = max(self.life_points - damage, 0)

class Wolff(Unit):
    SPEED = 1000
    def __init__(self, x, y):
        Unit.__init__(self, x, y, Wolff.SPEED)
    def move(self, x, y):
        Unit.move(self, x, y)
    def shoot(self, world, target_id):
        enemy = world.enemies[target_id]
        dist = math.sqrt((self.x - enemy.x) * (self.x - enemy.x) + (self.y - enemy.y) * (self.y - enemy.y))
        print(dist, dist**1.2)
        damage = round(125000 / (dist ** 1.2))
        enemy.damage(damage)

class DataPoint:
    def __init__(self, x, y):
        self.x = x
        self.y = y

class World:
    WIDTH = 16000
    HEIGHT = 9000

    def __init__(self, bot):
        self.bot = bot
        self.wolff = Wolff(1100, 1200)
        self.data_points = [DataPoint(5000, 5000), DataPoint(10000, 5000), DataPoint(5000, 1900), DataPoint(1000, 4000), DataPoint(1000, 8999)]
        self.enemies = [Enemy(10500, 8000, 10), Enemy(15000, 0, 42), Enemy(14000, 0, 42)]
        self.score = len(self.data_points) * 100
        self.is_wolff_killed = False
        self.initial_live_points_sum = sum([e.life_points for e in self.enemies])
        self.shots_num = 0

    def move(self):
        self.cur_turn = self.bot.make_turn(self)
        # 1. Enemies move towards their targets.
        for e in self.enemies:
            e.move(self)

        # 2. If a MOVE command was given, Wolff moves towards his target.
        if self.cur_turn[0] == 'MOVE':
            self.wolff.move(self.cur_turn[1], self.cur_turn[2])

        # 3. Game over if an enemy is close enough to Wolff.
        self.check_wolff_killed()
        if self.is_wolff_killed:
            self.is_wolff_killed = True
            self.score = 0
            return

        # 4. If a SHOOT command was given, Wolff shoots an enemy.
        if self.cur_turn[0] == 'SHOOT':
            self.shots_num += 1
            self.wolff.shoot(self, self.cur_turn[1])

        # 5. Enemies with zero life points are removed from play.
        new_enemies = [e for e in self.enemies if e.life_points > 0]
        self.score += (len(self.enemies) - len(new_enemies)) * 10
        self.enemies = new_enemies
        if not self.enemies:
            self.calculate_bonus()

        print([(e.x, e.y, e.life_points) for e in self.enemies])

        # 6. Enemies collect data points they share coordinates with.
        self.collect_data_points()

    def check_wolff_killed(self):
        wx, wy = self.wolff.x, self.wolff.y
        for e in self.enemies:
            if (e.x - wx) * (e.x - wx) + (e.y - wy) * (e.y - wy) <= Enemy.RANGE * Enemy.RANGE:
                print(e.x, e.y, wx, wy)
                self.is_wolff_killed = True
                return

    def get_data_points(self):
        return self.data_points

    def game_over(self):
        return not self.data_points or not self.enemies or self.is_wolff_killed

    def collect_data_points(self):
        new_data_points = []
        for p in self.data_points:
            is_taken = False
            for e in self.enemies:
                if e.x == p.x and e.y == p.y:
                    is_taken = True
            if not is_taken:
                new_data_points.append(p)
            else:
                self.score -= 100
        self.data_points = new_data_points

    def calculate_bonus(self):
        self.score += (len(self.data_points) *
            max(0, self.initial_live_points_sum - 3 * self.shots_num) * 3)

# Provides an interface for an actual bot written in C++.
class Bot:
    def __init__(self):
        self.proc = subprocess.Popen('./bot', shell=True, stdin=subprocess.PIPE, stdout=subprocess.PIPE)
    def make_turn(self, world):
        if self.proc.poll():
            raise Exception('Bot process terminated!')

        self.proc.stdin.write(bytes(Bot.serialize_world_state(world), encoding='utf8'))
        self.proc.stdin.flush()
        turn = self.proc.stdout.readline().decode('utf8').split()
        cmd = turn[0]
        if cmd == 'MOVE':
            assert len(turn) == 3
            x = int(turn[1])
            y = int(turn[2])
            assert 0 <= x < world.WIDTH and 0 <= y < world.HEIGHT
            turn[1] = x
            turn[2] = x
            return turn
        elif cmd == 'SHOOT':
            assert len(turn) == 2
            target_id = int(turn[1])
            assert 0 <= target_id < len(world.enemies)
            turn[1] = target_id
            return turn
        else:
            raise Exception('Unknown command: {}'.format(cmd))

    def serialize_world_state(world):
        return ('{} {}\n'.format(world.wolff.x, world.wolff.y)
             + Bot.serialize_data_points(world) + Bot.serialize_enemies(world))
    def serialize_data_points(world):
        return '\n'.join([str(len(world.data_points))] + [' '.join((str(i), str(p.x), str(p.y))) for i, p in enumerate(world.data_points)]) + '\n'

    def serialize_enemies(world):
        return '\n'.join([str(len(world.enemies))] + [' '.join((str(i), str(e.x), str(e.y), str(e.life_points))) for i, e in enumerate(world.enemies)]) + '\n'

world = World(Bot())
while not world.game_over():
    world.move()
print('Game finished. Final score: {}'.format(world.score))
