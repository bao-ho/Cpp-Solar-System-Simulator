#ifndef REALMODE_H
#define REALMODE_H

#include <QDialog>
#include <qstring.h>
#define REAL_MODE_CHANGE_SPEED 1
#define REAL_MODE_CREATE_NEW 2

namespace Ui {
class RealMode;
}

class RealMode : public QDialog
{
    Q_OBJECT

public:
    explicit RealMode(QWidget *parent = 0);
    ~RealMode();
    QString getFileNameFromUI();

    unsigned int getSpeedValueFromUI();

    unsigned int getRealModeAction();

    void setSlider (unsigned int);

private slots:
    void on_loadFile_clicked();

    void on_runButton_clicked();

    void on_speedSlider_valueChanged(int value);

private:
    Ui::RealMode *ui;
    unsigned int realModeAction;
    QString fileName;
    unsigned int speedValue;
};

#endif // REALMODE_H
