# -*- coding: utf-8 -*-

################################################################################
## Form generated from reading UI file 'form.ui'
##
## Created by: Qt User Interface Compiler version 6.7.0
##
## WARNING! All changes made in this file will be lost when recompiling UI file!
################################################################################

from PySide6.QtCore import (QCoreApplication, QDate, QDateTime, QLocale,
    QMetaObject, QObject, QPoint, QRect,
    QSize, QTime, QUrl, Qt)
from PySide6.QtGui import (QBrush, QColor, QConicalGradient, QCursor,
    QFont, QFontDatabase, QGradient, QIcon,
    QImage, QKeySequence, QLinearGradient, QPainter,
    QPalette, QPixmap, QRadialGradient, QTransform)
from PySide6.QtWidgets import (QApplication, QDoubleSpinBox, QFormLayout, QFrame,
    QGridLayout, QHBoxLayout, QLabel, QPushButton,
    QSizePolicy, QSlider, QSpacerItem, QSpinBox,
    QSplitter, QTabWidget, QWidget)

from QDoubleSlider import QDoubleSlider
from pyqtgraph import PlotWidget

class Ui_ganWidget(object):
    def setupUi(self, ganWidget):
        if not ganWidget.objectName():
            ganWidget.setObjectName(u"ganWidget")
        ganWidget.resize(1280, 720)
        font = QFont()
        font.setPointSize(13)
        ganWidget.setFont(font)
        self.gridLayout = QGridLayout(ganWidget)
        self.gridLayout.setObjectName(u"gridLayout")
        self.splitter = QSplitter(ganWidget)
        self.splitter.setObjectName(u"splitter")
        self.splitter.setOrientation(Qt.Horizontal)
        self.formLayoutWidget = QWidget(self.splitter)
        self.formLayoutWidget.setObjectName(u"formLayoutWidget")
        self.configFormLayout = QFormLayout(self.formLayoutWidget)
        self.configFormLayout.setObjectName(u"configFormLayout")
        self.configFormLayout.setContentsMargins(0, 0, 0, 0)
        self.VdMinLabel = QLabel(self.formLayoutWidget)
        self.VdMinLabel.setObjectName(u"VdMinLabel")
        self.VdMinLabel.setFont(font)

        self.configFormLayout.setWidget(0, QFormLayout.LabelRole, self.VdMinLabel)

        self.VdMinHLayout = QHBoxLayout()
        self.VdMinHLayout.setObjectName(u"VdMinHLayout")
        self.VdMinSpinbox = QDoubleSpinBox(self.formLayoutWidget)
        self.VdMinSpinbox.setObjectName(u"VdMinSpinbox")
        self.VdMinSpinbox.setFont(font)
        self.VdMinSpinbox.setMaximum(5.000000000000000)
        self.VdMinSpinbox.setSingleStep(0.050000000000000)

        self.VdMinHLayout.addWidget(self.VdMinSpinbox)

        self.VdMinSlider = QDoubleSlider(self.formLayoutWidget)
        self.VdMinSlider.setObjectName(u"VdMinSlider")
        self.VdMinSlider.setMaximum(100)
        self.VdMinSlider.setOrientation(Qt.Horizontal)

        self.VdMinHLayout.addWidget(self.VdMinSlider)


        self.configFormLayout.setLayout(0, QFormLayout.FieldRole, self.VdMinHLayout)

        self.VdMaxLabel = QLabel(self.formLayoutWidget)
        self.VdMaxLabel.setObjectName(u"VdMaxLabel")
        self.VdMaxLabel.setFont(font)
        self.VdMaxLabel.setFrameShape(QFrame.NoFrame)
        self.VdMaxLabel.setLineWidth(1)
        self.VdMaxLabel.setScaledContents(False)
        self.VdMaxLabel.setAlignment(Qt.AlignLeading|Qt.AlignLeft|Qt.AlignVCenter)
        self.VdMaxLabel.setWordWrap(False)

        self.configFormLayout.setWidget(1, QFormLayout.LabelRole, self.VdMaxLabel)

        self.VdMaxHLayout = QHBoxLayout()
        self.VdMaxHLayout.setObjectName(u"VdMaxHLayout")
        self.VdMaxSpinbox = QDoubleSpinBox(self.formLayoutWidget)
        self.VdMaxSpinbox.setObjectName(u"VdMaxSpinbox")
        self.VdMaxSpinbox.setFont(font)
        self.VdMaxSpinbox.setMaximum(5.000000000000000)
        self.VdMaxSpinbox.setSingleStep(0.050000000000000)
        self.VdMaxSpinbox.setValue(5.000000000000000)

        self.VdMaxHLayout.addWidget(self.VdMaxSpinbox)

        self.VdMaxSlider = QSlider(self.formLayoutWidget)
        self.VdMaxSlider.setObjectName(u"VdMaxSlider")
        self.VdMaxSlider.setOrientation(Qt.Horizontal)

        self.VdMaxHLayout.addWidget(self.VdMaxSlider)


        self.configFormLayout.setLayout(1, QFormLayout.FieldRole, self.VdMaxHLayout)

        self.VdStepLabel = QLabel(self.formLayoutWidget)
        self.VdStepLabel.setObjectName(u"VdStepLabel")
        self.VdStepLabel.setFont(font)

        self.configFormLayout.setWidget(2, QFormLayout.LabelRole, self.VdStepLabel)

        self.VdStepHLayout = QHBoxLayout()
        self.VdStepHLayout.setObjectName(u"VdStepHLayout")
        self.VdStepSpinbox = QDoubleSpinBox(self.formLayoutWidget)
        self.VdStepSpinbox.setObjectName(u"VdStepSpinbox")
        self.VdStepSpinbox.setFont(font)
        self.VdStepSpinbox.setDecimals(3)
        self.VdStepSpinbox.setMinimum(0.001000000000000)
        self.VdStepSpinbox.setMaximum(5.000000000000000)
        self.VdStepSpinbox.setSingleStep(0.001000000000000)

        self.VdStepHLayout.addWidget(self.VdStepSpinbox)

        self.VdStepSlider = QSlider(self.formLayoutWidget)
        self.VdStepSlider.setObjectName(u"VdStepSlider")
        self.VdStepSlider.setOrientation(Qt.Horizontal)

        self.VdStepHLayout.addWidget(self.VdStepSlider)


        self.configFormLayout.setLayout(2, QFormLayout.FieldRole, self.VdStepHLayout)

        self.VgMinLabel = QLabel(self.formLayoutWidget)
        self.VgMinLabel.setObjectName(u"VgMinLabel")

        self.configFormLayout.setWidget(3, QFormLayout.LabelRole, self.VgMinLabel)

        self.VgMinHLayout = QHBoxLayout()
        self.VgMinHLayout.setObjectName(u"VgMinHLayout")
        self.VgMinSpinbox = QDoubleSpinBox(self.formLayoutWidget)
        self.VgMinSpinbox.setObjectName(u"VgMinSpinbox")
        self.VgMinSpinbox.setMaximum(3.000000000000000)
        self.VgMinSpinbox.setSingleStep(0.250000000000000)

        self.VgMinHLayout.addWidget(self.VgMinSpinbox)

        self.VgMinSlider = QSlider(self.formLayoutWidget)
        self.VgMinSlider.setObjectName(u"VgMinSlider")
        self.VgMinSlider.setOrientation(Qt.Horizontal)

        self.VgMinHLayout.addWidget(self.VgMinSlider)


        self.configFormLayout.setLayout(3, QFormLayout.FieldRole, self.VgMinHLayout)

        self.VgMaxLabel = QLabel(self.formLayoutWidget)
        self.VgMaxLabel.setObjectName(u"VgMaxLabel")

        self.configFormLayout.setWidget(4, QFormLayout.LabelRole, self.VgMaxLabel)

        self.VgMaxHLayout = QHBoxLayout()
        self.VgMaxHLayout.setObjectName(u"VgMaxHLayout")
        self.VgMaxSpinbox = QDoubleSpinBox(self.formLayoutWidget)
        self.VgMaxSpinbox.setObjectName(u"VgMaxSpinbox")
        self.VgMaxSpinbox.setMaximum(3.000000000000000)
        self.VgMaxSpinbox.setSingleStep(0.250000000000000)
        self.VgMaxSpinbox.setValue(3.000000000000000)

        self.VgMaxHLayout.addWidget(self.VgMaxSpinbox)

        self.VgMaxSlider = QSlider(self.formLayoutWidget)
        self.VgMaxSlider.setObjectName(u"VgMaxSlider")
        self.VgMaxSlider.setOrientation(Qt.Horizontal)

        self.VgMaxHLayout.addWidget(self.VgMaxSlider)


        self.configFormLayout.setLayout(4, QFormLayout.FieldRole, self.VgMaxHLayout)

        self.VgStepLabel = QLabel(self.formLayoutWidget)
        self.VgStepLabel.setObjectName(u"VgStepLabel")

        self.configFormLayout.setWidget(5, QFormLayout.LabelRole, self.VgStepLabel)

        self.VgStepHLayout = QHBoxLayout()
        self.VgStepHLayout.setObjectName(u"VgStepHLayout")
        self.VgStepSpinbox = QDoubleSpinBox(self.formLayoutWidget)
        self.VgStepSpinbox.setObjectName(u"VgStepSpinbox")
        self.VgStepSpinbox.setMinimum(0.250000000000000)
        self.VgStepSpinbox.setMaximum(3.000000000000000)
        self.VgStepSpinbox.setSingleStep(0.250000000000000)

        self.VgStepHLayout.addWidget(self.VgStepSpinbox)

        self.VgStepSlider = QSlider(self.formLayoutWidget)
        self.VgStepSlider.setObjectName(u"VgStepSlider")
        self.VgStepSlider.setOrientation(Qt.Horizontal)

        self.VgStepHLayout.addWidget(self.VgStepSlider)


        self.configFormLayout.setLayout(5, QFormLayout.FieldRole, self.VgStepHLayout)

        self.tempReadsLabel = QLabel(self.formLayoutWidget)
        self.tempReadsLabel.setObjectName(u"tempReadsLabel")

        self.configFormLayout.setWidget(6, QFormLayout.LabelRole, self.tempReadsLabel)

        self.tempReadsHLayout = QHBoxLayout()
        self.tempReadsHLayout.setObjectName(u"tempReadsHLayout")
        self.tempReadsSpinbox = QSpinBox(self.formLayoutWidget)
        self.tempReadsSpinbox.setObjectName(u"tempReadsSpinbox")
        self.tempReadsSpinbox.setMinimum(1)
        self.tempReadsSpinbox.setMaximum(4096)

        self.tempReadsHLayout.addWidget(self.tempReadsSpinbox)

        self.tempReadsSlider = QSlider(self.formLayoutWidget)
        self.tempReadsSlider.setObjectName(u"tempReadsSlider")
        self.tempReadsSlider.setMinimum(1)
        self.tempReadsSlider.setMaximum(4096)
        self.tempReadsSlider.setOrientation(Qt.Horizontal)

        self.tempReadsHLayout.addWidget(self.tempReadsSlider)


        self.configFormLayout.setLayout(6, QFormLayout.FieldRole, self.tempReadsHLayout)

        self.tempOffsetLabel = QLabel(self.formLayoutWidget)
        self.tempOffsetLabel.setObjectName(u"tempOffsetLabel")

        self.configFormLayout.setWidget(7, QFormLayout.LabelRole, self.tempOffsetLabel)

        self.tempOffsetHLayout = QHBoxLayout()
        self.tempOffsetHLayout.setObjectName(u"tempOffsetHLayout")
        self.tempOffsetDoubleSpinBox = QDoubleSpinBox(self.formLayoutWidget)
        self.tempOffsetDoubleSpinBox.setObjectName(u"tempOffsetDoubleSpinBox")
        self.tempOffsetDoubleSpinBox.setMinimum(-50.000000000000000)
        self.tempOffsetDoubleSpinBox.setMaximum(50.000000000000000)

        self.tempOffsetHLayout.addWidget(self.tempOffsetDoubleSpinBox)

        self.tempOffsetSlider = QSlider(self.formLayoutWidget)
        self.tempOffsetSlider.setObjectName(u"tempOffsetSlider")
        self.tempOffsetSlider.setMinimum(-50)
        self.tempOffsetSlider.setMaximum(50)
        self.tempOffsetSlider.setOrientation(Qt.Horizontal)

        self.tempOffsetHLayout.addWidget(self.tempOffsetSlider)


        self.configFormLayout.setLayout(7, QFormLayout.FieldRole, self.tempOffsetHLayout)

        self.verticalSpacer = QSpacerItem(20, 40, QSizePolicy.Policy.Minimum, QSizePolicy.Policy.Expanding)

        self.configFormLayout.setItem(8, QFormLayout.LabelRole, self.verticalSpacer)

        self.beginTestButton = QPushButton(self.formLayoutWidget)
        self.beginTestButton.setObjectName(u"beginTestButton")

        self.configFormLayout.setWidget(11, QFormLayout.SpanningRole, self.beginTestButton)

        self.exportButton = QPushButton(self.formLayoutWidget)
        self.exportButton.setObjectName(u"exportButton")

        self.configFormLayout.setWidget(12, QFormLayout.SpanningRole, self.exportButton)

        self.splitter.addWidget(self.formLayoutWidget)
        self.tabWidget = QTabWidget(self.splitter)
        self.tabWidget.setObjectName(u"tabWidget")
        sizePolicy = QSizePolicy(QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Expanding)
        sizePolicy.setHorizontalStretch(1)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.tabWidget.sizePolicy().hasHeightForWidth())
        self.tabWidget.setSizePolicy(sizePolicy)
        self.tempTab = QWidget()
        self.tempTab.setObjectName(u"tempTab")
        self.gridLayout_2 = QGridLayout(self.tempTab)
        self.gridLayout_2.setObjectName(u"gridLayout_2")
        self.tempPlot = PlotWidget(self.tempTab)
        self.tempPlot.setObjectName(u"tempPlot")

        self.gridLayout_2.addWidget(self.tempPlot, 0, 0, 1, 1)

        self.tabWidget.addTab(self.tempTab, "")
        self.ivTab = QWidget()
        self.ivTab.setObjectName(u"ivTab")
        self.gridLayout_3 = QGridLayout(self.ivTab)
        self.gridLayout_3.setObjectName(u"gridLayout_3")
        self.ivPlot = PlotWidget(self.ivTab)
        self.ivPlot.setObjectName(u"ivPlot")

        self.gridLayout_3.addWidget(self.ivPlot, 0, 0, 1, 1)

        self.tabWidget.addTab(self.ivTab, "")
        self.splitter.addWidget(self.tabWidget)

        self.gridLayout.addWidget(self.splitter, 0, 0, 1, 1)


        self.retranslateUi(ganWidget)
        self.tempReadsSpinbox.valueChanged.connect(self.tempReadsSlider.setValue)
        self.tempReadsSlider.valueChanged.connect(self.tempReadsSpinbox.setValue)

        self.tabWidget.setCurrentIndex(1)


        QMetaObject.connectSlotsByName(ganWidget)
    # setupUi

    def retranslateUi(self, ganWidget):
        ganWidget.setWindowTitle(QCoreApplication.translate("ganWidget", u"ganwidget", None))
        self.VdMinLabel.setText(QCoreApplication.translate("ganWidget", u"<html><head/><body><p>V<span style=\" vertical-align:sub;\">D</span> min</p></body></html>", None))
        self.VdMinSpinbox.setSuffix(QCoreApplication.translate("ganWidget", u" V", None))
        self.VdMaxLabel.setText(QCoreApplication.translate("ganWidget", u"<html><head/><body><p>V<span style=\" vertical-align:sub;\">D</span> max</p></body></html>", None))
        self.VdMaxSpinbox.setSuffix(QCoreApplication.translate("ganWidget", u" V", None))
        self.VdStepLabel.setText(QCoreApplication.translate("ganWidget", u"<html><head/><body><p>V<span style=\" vertical-align:sub;\">D</span> step</p></body></html>", None))
        self.VdStepSpinbox.setSuffix(QCoreApplication.translate("ganWidget", u" V", None))
        self.VgMinLabel.setText(QCoreApplication.translate("ganWidget", u"<html><head/><body><p>V<span style=\" vertical-align:sub;\">G</span> min</p></body></html>", None))
        self.VgMinSpinbox.setSuffix(QCoreApplication.translate("ganWidget", u" V", None))
        self.VgMaxLabel.setText(QCoreApplication.translate("ganWidget", u"<html><head/><body><p>V<span style=\" vertical-align:sub;\">G</span> max</p></body></html>", None))
        self.VgMaxSpinbox.setSuffix(QCoreApplication.translate("ganWidget", u" V", None))
        self.VgStepLabel.setText(QCoreApplication.translate("ganWidget", u"Vg step", None))
        self.VgStepSpinbox.setSuffix(QCoreApplication.translate("ganWidget", u" V", None))
        self.tempReadsLabel.setText(QCoreApplication.translate("ganWidget", u"Temp reads", None))
        self.tempOffsetLabel.setText(QCoreApplication.translate("ganWidget", u"Temp offset", None))
        self.tempOffsetDoubleSpinBox.setSuffix(QCoreApplication.translate("ganWidget", u" \u00b0C", None))
        self.beginTestButton.setText(QCoreApplication.translate("ganWidget", u"Begin test", None))
        self.exportButton.setText(QCoreApplication.translate("ganWidget", u"Export results", None))
        self.tabWidget.setTabText(self.tabWidget.indexOf(self.tempTab), QCoreApplication.translate("ganWidget", u"Temperature results", None))
        self.tabWidget.setTabText(self.tabWidget.indexOf(self.ivTab), QCoreApplication.translate("ganWidget", u"I-V curve", None))
    # retranslateUi

