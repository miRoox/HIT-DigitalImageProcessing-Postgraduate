#ifndef LOCALENHSETTINGFORM_H
#define LOCALENHSETTINGFORM_H

#include <QWidget>

namespace Ui {
class LocalEnhSettingForm;
}

class LocalEnhSettingForm : public QWidget
{
    Q_OBJECT

public:
    explicit LocalEnhSettingForm(QWidget *parent = nullptr);
    ~LocalEnhSettingForm();

private:
    Ui::LocalEnhSettingForm *ui;
};

#endif // LOCALENHSETTINGFORM_H
