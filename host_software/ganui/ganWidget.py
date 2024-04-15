# This Python file uses the following encoding: utf-8
import time, traceback, sys

from PySide6.QtWidgets import QApplication, QWidget
from PySide6.QtCore import Signal, Slot, QTimer, QRunnable, QThreadPool, QObject
from ganTester import ganTester

# Important:
# You need to run the following command to generate the ui_form.py file
#     pyside6-uic form.ui -o ui_form.py, or
#     pyside2-uic form.ui -o ui_form.py
from ui_form import Ui_ganWidget

class WorkerSignals(QObject):
    '''
    Defines the signals available from a running worker thread.

    Supported signals are:

    finished
        No data

    error
        tuple (exctype, value, traceback.format_exc() )

    result
        object data returned from processing, anything

    progress
        int indicating % progress

    '''
    finished = Signal()
    error = Signal(tuple)
    result = Signal(object)
    progress = Signal(int)

class Worker(QRunnable):
    '''
    Worker thread

    Inherits from QRunnable to handler worker thread setup, signals and wrap-up.

    :param callback: The function callback to run on this worker thread. Supplied args and
                     kwargs will be passed through to the runner.
    :type callback: function
    :param args: Arguments to pass to the callback function
    :param kwargs: Keywords to pass to the callback function
    '''

    def __init__(self, fn, *args, **kwargs):
        super(Worker, self).__init__()

        # Store constructor arguments (re-used for processing)
        self.fn = fn
        self.args = args
        self.kwargs = kwargs
        self.signals = WorkerSignals()

        # Add the callback to our kwargs
        #self.kwargs['progress_callback'] = self.signals.progress

    @Slot()
    def run(self):
        '''
        Initialise the runner function with passed args, kwargs.
        '''

        # Retrieve args/kwargs here; and fire processing using them
        try:
            result = self.fn(*self.args, **self.kwargs)
        except:
            traceback.print_exc()
            exctype, value = sys.exc_info()[:2]
            self.signals.error.emit((exctype, value, traceback.format_exc()))
        else:
            self.signals.result.emit(result)  # Return the result of the processing
        finally:
            self.signals.finished.emit()  # Done

class ganWidget(QWidget):
    # Vd can range from 0 to 5V, in 0.05V increments. This gives 101 different
    # possible values for the slider. This slot converts each slider position to
    # the proper decimal value, then (TODO) emits a signal for another component
    # to receive
    @Slot(int)
    def VdSliderToDoubleSpinbox(self, spinbox):
        spinbox.setValue(round(1*0.05, 2))

    # Only want to be able to specify 0.05 V increments
    @Slot(str)
    def VdSpinBoxValidate(self):
        sender = self.sender()
        s = sender.value()
        sender.blockSignals(True) # don't send a signal for this next update
        new_val = round(s/0.05)*0.05
        if s != new_val: # if value is changed, put it in the spinbox
            sender.setValue(new_val)
        sender.blockSignals(False)

    # Update the UI with the results of the test
    @Slot(str)
    def testComplete(self):
        self.ui.beginTestButton.setEnabled(True)
        self.ui.beginTestButton.setText("Begin test")

    @Slot(str)
    def beginTestClicked(self):
        button = self.sender() # the button that was pressed
        print(button)
        print(self)

        button.setEnabled(False)
        button.setText("Testing...")

        # gather all test parameters from the UI
        self.tester.VdMin = round(self.ui.VdMinSpinbox.value(), 2)
        self.tester.VdMax = round(self.ui.VdMaxSpinbox.value(), 2)
        self.tester.VdStep = round(self.ui.VdStepSpinbox.value(), 3)
        self.tester.tempReads = self.ui.tempReadsSpinbox.value()

        testingThread = Worker(self.tester.startTest) # set up testing thread
        testingThread.signals.finished.connect(self.testComplete) # reset the button when thread is finished
        self.threadpool.start(testingThread) # begin the test

    def __init__(self, parent=None):
        super().__init__(parent)
        self.ui = Ui_ganWidget()
        self.ui.setupUi(self)

        self.threadpool = QThreadPool()
        print("Multithreading with maximum %d threads" % self.threadpool.maxThreadCount())


        # create tester instance
        self.tester = ganTester("ftdi://ftdi:232h/1", 0, 1E6, 0)

        # connect start test slot
        self.ui.beginTestButton.clicked.connect(self.beginTestClicked)

        # connect slider slots
        self.ui.VdMinSlider.valueChanged.connect(self.VdSliderToDoubleSpinbox)

        # connect spinbox validation signals/slots
        self.ui.VdMinSpinbox.editingFinished.connect(self.VdSpinBoxValidate)
        self.ui.VdMaxSpinbox.editingFinished.connect(self.VdSpinBoxValidate)

if __name__ == "__main__":
    app = QApplication(sys.argv)
    widget = ganWidget()
    widget.show()
    sys.exit(app.exec())
