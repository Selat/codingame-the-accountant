#!/usr/bin/env python

import sys
import simulator

def main():
    if len(sys.argv) != 4:
        print('Please use the following format:')
        print('./sxs_test.py TEST_SET OLD_BOT NEW_BOT')
    else:
        test_set = sys.argv[1]
        bot_program1 = sys.argv[2]
        bot_program2 = sys.argv[3]
        tests = simulator.list_tests(test_set)
        world = simulator.World(None)
        scores_sum1 = 0
        positive_bonus_num1 = 0
        bonuses_sum1 = 0
        scores_sum2 = 0
        positive_bonus_num2 = 0
        bonuses_sum2 = 0
        for test in tests:
            world.bot = simulator.Bot(bot_program1)
            simulator.run_test(world, test)
            world.bot.proc.terminate()
            world.bot = None
            score1 = world.total_score()
            scores_sum1 += world.total_score()
            bonuses_sum1 += world.bonus
            if world.bonus > 0:
                positive_bonus_num1 += 1

            world.bot = simulator.Bot(bot_program2)
            simulator.run_test(world, test)
            world.bot.proc.terminate()
            world.bot = None
            score2 = world.total_score()
            scores_sum2 += world.total_score()
            bonuses_sum2 += world.bonus
            if world.bonus > 0:
                positive_bonus_num2 += 1

            if score1 < score2:
                print('{:31} improvement: {} -> {}'.format(
                    simulator.get_test_name(test), score1, score2))
            elif score1 > score2:
                print('{:31} regression: {} -> {}'.format(
                    simulator.get_test_name(test), score1, score2))
        print('Sum: {} -> {}'.format(scores_sum1, scores_sum2))
        print('Bonus: {} -> {} ({:0.4}% -> {:0.4}%)'.format(
            bonuses_sum1, bonuses_sum2, bonuses_sum1 / scores_sum1 * 100,
            bonuses_sum2 / scores_sum2 * 100))
        print('Positive bonus: {:0.4}% -> {:0.4}%'.format(
            positive_bonus_num1 / len(tests) * 100,
            positive_bonus_num2 / len(tests) * 100))



if __name__ == '__main__':
    main()
