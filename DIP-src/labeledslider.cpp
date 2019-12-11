#include "labeledslider.h"
#include <cmath>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QHBoxLayout>

LabeledSlider::LabeledSlider(QWidget *parent)
    : QWidget(parent), slider(new QSlider), spinbox(new QDoubleSpinBox)
{
    auto layout = new QHBoxLayout;
    layout->addWidget(slider);
    layout->addWidget(spinbox);
    slider->setOrientation(Qt::Horizontal);
    setLayout(layout);
    setFocusProxy(spinbox);

    connect(slider,&QSlider::valueChanged,[this](int i){
        double max = spinbox->maximum();
        double min = spinbox->minimum();
        int imax = slider->maximum();
        int imin = slider->minimum();
        spinbox->setValue((max*(i-imin)+min*(imax-i))/(imax-imin));
    });
    connect(spinbox,qOverload<double>(&QDoubleSpinBox::valueChanged),[this](double d){
        double max = spinbox->maximum();
        double min = spinbox->minimum();
        int imax = slider->maximum();
        int imin = slider->minimum();
        slider->setValue(static_cast<int>((imax*(d-min)+imin*(max-d))/(max-min)));
    });
    connect(spinbox,qOverload<double>(&QDoubleSpinBox::valueChanged),
            this,&LabeledSlider::valueChanged);
}

LabeledSlider::LabeledSlider(double max, QWidget *parent)
    :LabeledSlider(0,max,parent)
{
}

LabeledSlider::LabeledSlider(double min, double max, QWidget *parent)
    : LabeledSlider(parent)
{
    setMinimum(min);
    setMaximum(max);
}

LabeledSlider::LabeledSlider(double min, double max, double step, QWidget *parent)
    : LabeledSlider(parent)
{
    setMinimum(min);
    setMaximum(max);
    setSingleStep(step);
}

double LabeledSlider::maximum() const
{
    return spinbox->maximum();
}

double LabeledSlider::minimum() const
{
    return spinbox->minimum();
}

int LabeledSlider::decimals() const
{
    return spinbox->decimals();
}

double LabeledSlider::singleStep() const
{
    return spinbox->singleStep();
}

double LabeledSlider::value() const
{
    return spinbox->value();
}

void LabeledSlider::setMaximum(double max)
{
    spinbox->setMaximum(max);
    int dec = spinbox->decimals();
    double min = spinbox->minimum();
    int imin = slider->minimum();
    double d = max - min;
    int id = static_cast<int>(d*std::pow(10,dec));
    slider->setMaximum(imin+id);
}

void LabeledSlider::setMinimum(double min)
{
    spinbox->setMinimum(min);
    int dec = spinbox->decimals();
    double max = spinbox->maximum();
    int imax = slider->maximum();
    double d = max - min;
    int id = static_cast<int>(d*std::pow(10,dec));
    slider->setMinimum(imax-id);
}

void LabeledSlider::setDecimals(int prec)
{
    spinbox->setDecimals(prec);
    setMaximum(spinbox->maximum());
}

void LabeledSlider::setSingleStep(double val)
{
    spinbox->setSingleStep(val);
    int dec = spinbox->decimals();
    slider->setSingleStep(static_cast<int>(val*std::pow(10,dec)));
}

void LabeledSlider::setValue(double val)
{
    spinbox->setValue(val);
}
