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
    QSplitter, QTextEdit, QWidget)

from QDoubleSlider import QDoubleSlider

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
        sizePolicy = QSizePolicy(QSizePolicy.Policy.Preferred, QSizePolicy.Policy.Preferred)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.splitter.sizePolicy().hasHeightForWidth())
        self.splitter.setSizePolicy(sizePolicy)
        self.splitter.setFrameShape(QFrame.NoFrame)
        self.splitter.setOrientation(Qt.Horizontal)
        self.splitter.setOpaqueResize(True)
        self.splitter.setHandleWidth(10)
        self.splitter.setChildrenCollapsible(False)
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
        self.VdStepSpinbox.setMaximum(5.000000000000000)
        self.VdStepSpinbox.setSingleStep(0.001000000000000)

        self.VdStepHLayout.addWidget(self.VdStepSpinbox)

        self.VdStepSlider = QSlider(self.formLayoutWidget)
        self.VdStepSlider.setObjectName(u"VdStepSlider")
        self.VdStepSlider.setOrientation(Qt.Horizontal)

        self.VdStepHLayout.addWidget(self.VdStepSlider)


        self.configFormLayout.setLayout(2, QFormLayout.FieldRole, self.VdStepHLayout)

        self.tempReadsLabel = QLabel(self.formLayoutWidget)
        self.tempReadsLabel.setObjectName(u"tempReadsLabel")

        self.configFormLayout.setWidget(3, QFormLayout.LabelRole, self.tempReadsLabel)

        self.horizontalLayout = QHBoxLayout()
        self.horizontalLayout.setObjectName(u"horizontalLayout")
        self.tempReadsSpinbox = QSpinBox(self.formLayoutWidget)
        self.tempReadsSpinbox.setObjectName(u"tempReadsSpinbox")
        self.tempReadsSpinbox.setMaximum(4096)

        self.horizontalLayout.addWidget(self.tempReadsSpinbox)

        self.tempReadsSlider = QSlider(self.formLayoutWidget)
        self.tempReadsSlider.setObjectName(u"tempReadsSlider")
        self.tempReadsSlider.setMaximum(4096)
        self.tempReadsSlider.setOrientation(Qt.Horizontal)

        self.horizontalLayout.addWidget(self.tempReadsSlider)


        self.configFormLayout.setLayout(3, QFormLayout.FieldRole, self.horizontalLayout)

        self.verticalSpacer = QSpacerItem(20, 40, QSizePolicy.Policy.Minimum, QSizePolicy.Policy.Expanding)

        self.configFormLayout.setItem(4, QFormLayout.SpanningRole, self.verticalSpacer)

        self.beginTestButton = QPushButton(self.formLayoutWidget)
        self.beginTestButton.setObjectName(u"beginTestButton")

        self.configFormLayout.setWidget(5, QFormLayout.SpanningRole, self.beginTestButton)

        self.splitter.addWidget(self.formLayoutWidget)
        self.textEdit = QTextEdit(self.splitter)
        self.textEdit.setObjectName(u"textEdit")
        sizePolicy1 = QSizePolicy(QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Expanding)
        sizePolicy1.setHorizontalStretch(1)
        sizePolicy1.setVerticalStretch(0)
        sizePolicy1.setHeightForWidth(self.textEdit.sizePolicy().hasHeightForWidth())
        self.textEdit.setSizePolicy(sizePolicy1)
        self.textEdit.setBaseSize(QSize(1000, 0))
        self.textEdit.setFrameShape(QFrame.StyledPanel)
        self.splitter.addWidget(self.textEdit)

        self.gridLayout.addWidget(self.splitter, 0, 0, 1, 1)


        self.retranslateUi(ganWidget)
        self.tempReadsSlider.valueChanged.connect(self.tempReadsSpinbox.setValue)
        self.tempReadsSpinbox.valueChanged.connect(self.tempReadsSlider.setValue)

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
        self.tempReadsLabel.setText(QCoreApplication.translate("ganWidget", u"Temp reads", None))
        self.beginTestButton.setText(QCoreApplication.translate("ganWidget", u"Begin test", None))
        self.textEdit.setHtml(QCoreApplication.translate("ganWidget", u"<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><meta charset=\"utf-8\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"hr { height: 1px; border-width: 0; }\n"
"li.unchecked::marker { content: \"\\2610\"; }\n"
"li.checked::marker { content: \"\\2612\"; }\n"
"</style></head><body style=\" font-family:'Noto Sans'; font-size:13pt; font-weight:400; font-style:normal;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:12pt;\">This is where the output will go</span></p></body></html>", None))
    # retranslateUi

