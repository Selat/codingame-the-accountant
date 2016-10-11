#!/usr/bin/env python

import time
import math
import subprocess
import sys
import os

class Unit:
    def __init__(self, speed):
        self.x = 0
        self.y = 0
        self.speed = speed
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
    def __init__(self):
        Unit.__init__(self, Enemy.SPEED)
        self.life_points = 0

    def move(self, world):
        min_dist = 10 ** 9
        target = None
        for p in world.data_points.values():
            cur_dist = (p.x - self.x) * (p.x - self.x) + (p.y - self.y) * (p.y - self.y)
            if cur_dist < min_dist:
                min_dist = cur_dist
                target = p
        Unit.move(self, target.x, target.y)
        #time.sleep(1)

    def damage(self, damage):
        self.life_points = max(self.life_points - damage, 0)

    def serialize(self):
        return '{} {} {} {}'.format(self.id, self.x, self.y, self.life_points)

    def deserialize(self, s):
        data = s.split(' ')
        assert(len(data) == 4)
        self.id = int(data[0])
        self.x = int(data[1])
        self.y = int(data[2])
        self.life_points = int(data[3])
        assert 0 <= self.x < World.WIDTH and 0 <= self.y < World.HEIGHT
        assert 1 <= self.life_points < World.MAX_LIFE_POINTS

class Wolff(Unit):
    SPEED = 1000
    def __init__(self):
        Unit.__init__(self, Wolff.SPEED)
    def move(self, x, y):
        Unit.move(self, x, y)

    def shoot(self, world, target_id):
        enemy = world.enemies[target_id]
        dist = math.sqrt((self.x - enemy.x) * (self.x - enemy.x) + (self.y - enemy.y) * (self.y - enemy.y))
        damage = round(125000 / (dist ** 1.2))
        enemy.damage(damage)

    def serialize(self):
        return '{} {}'.format(self.x, self.y)

    def deserialize(self, s):
        data = s.split(' ')
        assert(len(data) == 2)
        self.x = int(data[0])
        self.y = int(data[1])
        assert 0 <= self.x < World.WIDTH and 0 <= self.y < World.HEIGHT

class DataPoint:
    def __init__(self):
        self.x = 0
        self.y = 0

    def serialize(self):
        return '{} {} {}'.format(self.id, self.x, self.y)

    def deserialize(self, s):
        data = s.split(' ')
        assert len(data) == 3
        self.id = int(data[0])
        self.x = int(data[1])
        self.y = int(data[2])
        assert 0 <= self.x < World.WIDTH and 0 <= self.y < World.HEIGHT

class World:
    WIDTH = 16000
    HEIGHT = 9000
    DATA_POINT_SCORE = 100
    MAX_LIFE_POINTS = 150

    def __init__(self, bot):
        self.bot = bot
        self.wolff = Wolff()
        self.data_points = {}
        self.enemies = {}
        self.score = 0
        self.is_wolff_killed = False
        self.initial_life_points_sum = 0
        self.shots_num = 0

    def move(self):
        self.cur_turn = self.bot.make_turn(self)
        # 1. Enemies move towards their targets.
        for e in self.enemies.values():
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
        killed_ids = [id for id, e in self.enemies.items() if e.life_points == 0]
        for id in killed_ids:
            del self.enemies[id]
            self.score += 10
        if not self.enemies:
            self.calculate_bonus()

        # 6. Enemies collect data points they share coordinates with.
        self.collect_data_points()

    def check_wolff_killed(self):
        wx, wy = self.wolff.x, self.wolff.y
        for e in self.enemies.values():
            d = (e.x - wx) * (e.x - wx) + (e.y - wy) * (e.y - wy)
            if d <= Enemy.RANGE * Enemy.RANGE:
                self.is_wolff_killed = True
                return

    def game_over(self):
        return not self.data_points or not self.enemies or self.is_wolff_killed

    def collect_data_points(self):
        new_data_points = {}
        for id, p in self.data_points.items():
            is_taken = False
            for e in self.enemies.values():
                if e.x == p.x and e.y == p.y:
                    is_taken = True
            if not is_taken:
                new_data_points[id] = p
            else:
                self.score -= World.DATA_POINT_SCORE
        self.data_points = new_data_points

    def calculate_bonus(self):
        self.score += (len(self.data_points) *
            max(0, self.initial_life_points_sum - 3 * self.shots_num) * 3)

    def serialize(self):
        # World state should end with a newline, otherwise bot hangs
        return '\n'.join([self.wolff.serialize(),
                          self.serialize_entities(self.data_points),
                          self.serialize_entities(self.enemies), ''])

    def serialize_entities(self, entities):
        return '\n'.join([str(len(entities))] +
                         [e.serialize() for e in entities.values()])

    def deserialize(self, input):
        self.__init__(self.bot)
        self.wolff.deserialize(input.readline())
        data_points_num = int(input.readline())
        for i in range(0, data_points_num):
            dp = DataPoint()
            dp.deserialize(input.readline())
            self.data_points[dp.id] = dp
        enemies_num = int(input.readline())
        for i in range(0, enemies_num):
            enemy = Enemy()
            enemy.deserialize(input.readline())
            self.enemies[enemy.id] = enemy
        self.score = len(self.data_points) * World.DATA_POINT_SCORE
        self.initial_life_points_sum = sum(
            [e.life_points for e in self.enemies.values()])

# Provides an interface for an actual bot written in C++.
class Bot:
    def __init__(self, prog_name):
        self.proc = subprocess.Popen(prog_name, shell=True,
                                     stdin=subprocess.PIPE,
                                     stdout=subprocess.PIPE)
    def make_turn(self, world):
        if self.proc.poll():
            raise Exception('Bot process terminated!')

        self.proc.stdin.write(bytes(world.serialize(), encoding='utf8'))
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
            assert target_id in world.enemies
            turn[1] = target_id
            return turn
        else:
            raise Exception('Unknown command: {}'.format(cmd))

def list_tests(test_set):
    files = os.listdir(os.path.join(os.getcwd(), test_set))
    files.sort()
    res = [os.path.join(os.getcwd(), test_set, f) for f in files]
    return res

def run_test(world, test_path):
    with open(test_path) as f:
        world.deserialize(f)
    while not world.game_over():
        world.move()

def get_test_name(test_path):
    return os.path.split(test_path)[-1]

def run_regression_tests():
    expected_scores = (110, 220, 120, 120, 148, 311, 450, 60, 510, 90,
                       406, 0, 220, 346, 770, 342, 0, 120, 120, 136,
                       152, 0, 110, 110, 260, 60, 240, 320, 339, 160,
                       0, 2220)
    tests = list_tests('public_tests')
    world = World(Bot('./paralyzed_wolff'))
    all_tests_pass = True
    for expected_score, test in zip(expected_scores, tests):
        run_test(world, test)
        if world.score != expected_score:
            all_tests_pass = False
            print('Test {} failed. Expected score {}, got {}'.format(
                get_test_name(test), expected_score, world.score))
            break
    if all_tests_pass:
        print('All tests passed successfully :)')

def main():
    #run_regression_tests()
    if len(sys.argv) != 3:
        print('Plese use the following format:')
        print('./simulator.py TEST_SET BOT_PROGRAM')
    else:
        test_set = sys.argv[1]
        bot_program = sys.argv[2]
        tests = list_tests(test_set)
        world = World(Bot(bot_program))
        scores_sum = 0
        for test in tests:
            run_test(world, test)
            scores_sum += world.score
            print('{:31} score: {} {}'.format(get_test_name(test), world.score,
                  '(killed)' if world.is_wolff_killed else ''))
        print('Sum: {}'.format(scores_sum))

if __name__ == '__main__':
    main()
