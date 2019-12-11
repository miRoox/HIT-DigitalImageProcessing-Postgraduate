#ifndef LABELEDSLIDER_H
#define LABELEDSLIDER_H

#include <QWidget>

class QSlider;
class QDoubleSpinBox;

class LabeledSlider : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(double value READ value WRITE setValue NOTIFY valueChanged)
    Q_PROPERTY(double maximum READ maximum WRITE setMaximum)
    Q_PROPERTY(double minimum READ minimum WRITE setMinimum)
    Q_PROPERTY(double singleStep READ singleStep WRITE setSingleStep)
    Q_PROPERTY(int decimals READ decimals WRITE setDecimals)

public:
    explicit LabeledSlider(QWidget *parent = nullptr);
    explicit LabeledSlider(double max, QWidget *parent = nullptr);
    explicit LabeledSlider(double min, double max, QWidget *parent = nullptr);
    explicit LabeledSlider(double min, double max, double step, QWidget *parent = nullptr);
    double maximum() const;
    double minimum() const;
    int decimals() const;
    double singleStep() const;
    double value() const;

signals:
    void valueChanged(double val);

public slots:
    void setMaximum(double max);
    void setMinimum(double min);
    void setDecimals(int prec);
    void setSingleStep(double val);
    void setValue(double val);

private:
    QSlider* slider;
    QDoubleSpinBox* spinbox;
};

#endif // LABELEDSLIDER_H
