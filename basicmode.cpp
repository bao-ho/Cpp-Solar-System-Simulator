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

unsigned int BasicMode::getDiameterScaleFromUser(){
    return diameterScaleFromUser;
}

void BasicMode::on_createNewButton_clicked()
{
    numberOfPlanetFromUser = ui->noOfPlanetsTextBox->toPlainText().toUInt();
    updateIntervalFromUser = ui->updateIntervalTextBox->toPlainText().toUInt();
    if (ui->smallRadioButton->isChecked())
        diameterScaleFromUser = USER_SCALE_SMALL;
    else if (ui->mediumRadioButton->isChecked())
        diameterScaleFromUser = USER_SCALE_MEDIUM;
    else
        diameterScaleFromUser = USER_SCALE_LARGE;
    //pop up a message if number of planet is less than 1 or larger than 100, or if update interval is less than 10 or larger than 1000
    if (numberOfPlanetFromUser < 1 || numberOfPlanetFromUser > 100 || updateIntervalFromUser < 10 || updateIntervalFromUser > 1000)
        QMessageBox::information(this, tr("Input data invalid"),"Sorry, update_interval = 10 to 1000 and #planet = 1 to 100");
    else{
        this->accept();
    }
}

