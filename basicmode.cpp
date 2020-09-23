#include "basicmode.h"
#include "ui_basicmode.h"
#include <qmessagebox.h>

BasicMode::BasicMode(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BasicMode)
{
    ui->setupUi(this);
}

BasicMode::~BasicMode()
{
    delete ui;
}

unsigned int BasicMode::getNumberOfPlanetFromUser(){
    return numberOfPlanetFromUser;
}
unsigned int BasicMode::getUpdateIntervalFromUser(){
    return updateIntervalFromUser;
}

unsigned int BasicMode::getSunSizeFromUser(){
    return sunSizeFromUser;
}

unsigned int BasicMode::getbasicModeAction(){
    return basicModeAction;
}

void BasicMode::on_createNewButton_clicked()
{
    numberOfPlanetFromUser = ui->noOfPlanets->toPlainText().toUInt();
    updateIntervalFromUser = ui->updateInterval->toPlainText().toUInt();
    if (ui->smallRadioButton->isChecked())
        sunSizeFromUser = SUN_SMALL;
    else if (ui->mediumRadioButton->isChecked())
        sunSizeFromUser = SUN_MEDIUM;
    else
        sunSizeFromUser = SUN_BIG;
    //pop up a message if number of planet is less than 1 or larger than 100, or if update interval is less than 10 or larger than 1000
    if (numberOfPlanetFromUser < 1 || numberOfPlanetFromUser > 100 || updateIntervalFromUser < 10 || updateIntervalFromUser > 1000)
        QMessageBox::information(this, tr("Input data invalid"),"Sorry, update_interval = 10 to 1000 and #planet = 1 to 100");
    else{
        basicModeAction = BASIC_MODE_UPDATE_ALL;
        this->accept();
    }
}

void BasicMode::on_changeSizeButton_clicked()
{
    if (ui->smallRadioButton->isChecked())
        sunSizeFromUser = SUN_SMALL;
    else if (ui->mediumRadioButton->isChecked())
        sunSizeFromUser = SUN_MEDIUM;
    else
        sunSizeFromUser = SUN_BIG;
    basicModeAction     = BASIC_MODE_UPDATE_SIZE;
    this->accept();
}

void BasicMode::on_updateButton_clicked()
{
    updateIntervalFromUser = ui->updateInterval->toPlainText().toUInt();
    //pop up a message if update interval is less than 10 or larger than 1000
    if (updateIntervalFromUser < 10 || updateIntervalFromUser > 1000)
        QMessageBox::information(this, tr("Input data invalid"),"Sorry, update_interval = 10 to 1000");
    else{
        basicModeAction = BASIC_MODE_UPDATE_INTERVAL;
        this->accept();
    }
}
