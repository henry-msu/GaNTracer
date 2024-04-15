# This Python file uses the following encoding: utf-8
# DoubleSlider to connect with doublespinbox

from PySide6.QtWidgets import QSlider
from PySide6.QtCore import Signal, Slot

class QDoubleSlider(QSlider):
    valueChanged = Signal((float,))
    doubleValue = 0;

    def __init__(self, parent=None):
        super(QDoubleSlider, self).__init__(parent)
