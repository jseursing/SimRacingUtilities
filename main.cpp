#include "FramelessInterface.h"
#include "SRU_Main.h"

#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    FramelessInterface frameless(FramelessInterface::TBF_MIN | FramelessInterface::TBF_CLOSE, true);
    SRU_Main w(&frameless);
    frameless.InitializeMainInterface(&w, ":/SimRU/resources/icon.png", std::vector<QAction*>());
    frameless.show();

    return a.exec();
}
