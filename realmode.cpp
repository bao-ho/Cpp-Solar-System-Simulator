#include "realmode.h"
#include "ui_realmode.h"
#include <qstring.h>
#include <qfiledialog.h>

RealMode::RealMode(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RealMode)
{
    ui->setupUi(this);
}

RealMode::~RealMode()
{
    delete ui;
}

void RealMode::on_loadFile_clicked()
{
    fileName = QFileDialog::getOpenFileName(this, tr("Open File"), "../Solar_Final",
                                                    "CSV Files (*.csv)");
    ui->fileNameLabel->setText(fileName);
}

void RealMode::on_runButton_clicked()
{
    fileName = ui->fileNameLabel->text();
    speedValue = ui->speedSlider->value();
    if (fileName == "Please load a .csv file")
        realModeAction = REAL_MODE_CHANGE_SPEED;
    else
        realModeAction = REAL_MODE_CREATE_NEW;
    this->accept();
}

QString RealMode::getFileNameFromUI(){
    return fileName;
}

void RealMode::setSlider(unsigned int value){
    ui->speedSlider->setValue(value);
}

unsigned int RealMode::getSpeedValueFromUI(){
    return speedValue;
}

unsigned int RealMode::getRealModeAction(){
    return realModeAction;
}

void RealMode::on_speedSlider_valueChanged(int value)
{
    ui->speedValueLabel->setText(QString::number(ui->speedSlider->value()/86400.0) + " Days/Second");
}
