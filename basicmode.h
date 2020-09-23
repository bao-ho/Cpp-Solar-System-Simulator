#ifndef BASICMODE_H
#define BASICMODE_H

#include <QDialog>

#define SUN_SMALL 1
#define SUN_MEDIUM 2
#define SUN_BIG 3
#define BASIC_MODE_UPDATE_ALL 1
#define BASIC_MODE_UPDATE_SIZE 2
#define BASIC_MODE_UPDATE_INTERVAL 3

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
    unsigned int getSunSizeFromUser();
    unsigned int getbasicModeAction();

private slots:

    void on_createNewButton_clicked();

    void on_changeSizeButton_clicked();

    void on_updateButton_clicked();

private:
    Ui::BasicMode *ui;
    unsigned int numberOfPlanetFromUser;
    unsigned int updateIntervalFromUser;
    unsigned int sunSizeFromUser;
    unsigned int basicModeAction;

};

#endif // BASICMODE_H
