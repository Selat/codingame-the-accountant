#!/usr/bin/env python

import sys
import simulator
import io

from PyQt5.QtWidgets import QWidget, QApplication
from PyQt5.QtGui import QPainter, QColor, QBrush, QFont
from PyQt5.QtCore import Qt, QPoint

class GameVisualizer(QWidget):

    def __init__(self, world):
        super().__init__()
        self.cur_world_state = 0
        self.world_states = []
        self.world_steps = []
        self.world = world
        while not self.world.game_over():
            self.world_states.append(self.world.serialize())
            self.world_steps.append(self.world.serialize_step())
            self.world.step()
        self.world_states.append(self.world.serialize())
        self.world_steps.append(self.world.serialize_step())
        self.world.deserialize(io.StringIO(self.world_states[self.cur_world_state]))
        self.initUI()

    def initUI(self):
        self.setGeometry(100, 100, 1600, 900)
        self.setWindowTitle('The Accountant')
        self.show()

    def convertCoordinates(self, x, y):
        size = self.size()
        return (x / simulator.World.WIDTH * size.width(),
                y / simulator.World.HEIGHT * size.height())

    def paintEvent(self, e):
        qp = QPainter()
        qp.begin(self)
        self.drawWorld(e, qp)
        qp.end()

    def keyPressEvent(self, e):
        if e.key() == Qt.Key_Left:
            if self.cur_world_state > 0:
                self.cur_world_state -= 1
                self.world.deserialize(io.StringIO(self.world_states[self.cur_world_state]))
                self.world.deserialize_step(self.world_steps[self.cur_world_state])
                self.repaint()
        elif e.key() == Qt.Key_Right:
            if self.cur_world_state + 1 < len(self.world_states):
                self.cur_world_state += 1
                self.world.deserialize(io.StringIO(self.world_states[self.cur_world_state]))
                self.world.deserialize_step(self.world_steps[self.cur_world_state])
                self.repaint()

    def drawWorld(self, event, qp):
        qp.setFont(QFont('Decorative', 20))
        self.drawDataPoints(qp)
        self.drawWolff(qp)
        self.drawEnemies(qp)

        center = QPoint(self.size().width() - 200, 50)
        qp.drawText(center, 'Score: {}'.format(self.world.total_score()))

    def drawDataPoints(self, qp):
        for dp in self.world.data_points.values():
            qp.setBrush(Qt.green)
            x, y = self.convertCoordinates(dp.x, dp.y)
            r = 50
            center = QPoint(x, y)
            qp.drawEllipse(center, r, r)

    def drawEnemies(self, qp):
        for e in self.world.enemies.values():
            qp.setBrush(Qt.NoBrush)
            x, y = self.convertCoordinates(e.x, e.y)
            rx, ry = self.convertCoordinates(e.RANGE, e.RANGE)
            center = QPoint(x, y)
            qp.drawEllipse(center, rx, ry)
            qp.setBrush(Qt.red)
            x, y = self.convertCoordinates(e.x, e.y)
            r = 10
            center = QPoint(x, y)
            qp.drawEllipse(center, r, r)
            center = QPoint(x, y - 10)
            qp.drawText(center, str(e.life_points))
            did = e.find_target_id(self.world)
            if did >= 0:
                center = QPoint(x, y)
                dp = self.world.data_points[did]
                x, y = self.convertCoordinates(dp.x, dp.y)
                dp_coord = QPoint(x, y)
                qp.drawLine(center, dp_coord)

    def drawWolff(self, qp):
        qp.setBrush(Qt.yellow)
        x, y = self.convertCoordinates(self.world.wolff.x, self.world.wolff.y)
        r = 50
        center = QPoint(x, y)
        qp.drawEllipse(center, r, r)


def main():
    if len(sys.argv) < 3:
        print('Please use the following format:')
        print('./visualizer TEST_FILE BOT_PROGRAM')
        return
    app = QApplication(sys.argv)
    test_path = sys.argv[1]
    bot_program = sys.argv[2]
    world = simulator.World(simulator.Bot(bot_program))
    with open(test_path) as f:
        world.deserialize(f)
    game_visualizer = GameVisualizer(world)
    sys.exit(app.exec_())

if __name__ == '__main__':
    main()
