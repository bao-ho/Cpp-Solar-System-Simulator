#include <QApplication>
#include <cmath>
#include "solarviswindow.h"

int main(int argc, char *argv[]){
    QApplication app(argc, argv);
    SolarVisWindow window;
    window.resize(640,480);
    window.setWindowTitle("Solar System Simulator");
    //window.setWindowState(Qt::WindowMaximized);
    window.show();
    return app.exec();
}
