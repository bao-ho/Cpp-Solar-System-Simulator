#ifndef BASICMODE_H
#define BASICMODE_H

#include <QDialog>

#define USER_SCALE_SMALL    10
#define USER_SCALE_MEDIUM   20
#define USER_SCALE_LARGE    30

namespace Ui {
class BasicMode;
}

class BasicMode : public QDialog
{
    Q_OBJECT

public:
    explicit BasicMode(QWidget *parent = 0);
    ~BasicMode();
    unsigned int getNumberOfPlanetFromUser();
    unsigned int getUpdateIntervalFromUser();
    unsigned int getDiameterScaleFromUser();
    unsigned int getbasicModeAction();

private slots:

    void on_createNewButton_clicked();

private:
    Ui::BasicMode *ui;
    unsigned int numberOfPlanetFromUser;
    unsigned int updateIntervalFromUser;
    unsigned int diameterScaleFromUser;

};

#endif // BASICMODE_H
