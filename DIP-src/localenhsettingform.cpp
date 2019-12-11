#include "localenhsettingform.h"
#include "ui_localenhsettingform.h"

LocalEnhSettingForm::LocalEnhSettingForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LocalEnhSettingForm)
{
    ui->setupUi(this);
}

LocalEnhSettingForm::~LocalEnhSettingForm()
{
    delete ui;
}
