from qt import *
from janela import *
import sys
if __name__ == "__main__":
    app = QApplication(sys.argv)
    f = janela()
    f.show()
    app.setMainWidget(f)
    app.exec_loop()
