/* This file is part of the KDE libraries
 * Initial implementation:
 *     Copyright (c) 1997 Patrick Dowler <dowler@morgul.fsh.uvic.ca>
 * Rewritten and maintained by:
 *     Copyright (c) 2000 Dirk Mueller <mueller@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#include "kpNumInput.h"

#include <cmath>

#include <QApplication>
#include <QDebug>
#include <QLabel>
#include <QLineEdit>
#include <QResizeEvent>
#include <QSlider>
#include <QStyle>

#include <KLocalizedString>

static inline int calcDiffByTen(int x, int y)
{
    // calculate ( x - y ) / 10 without overflowing ints:
    return (x / 10) - (y / 10)  + (x % 10 - y % 10) / 10;
}

// ----------------------------------------------------------------------------

class kpNumInputPrivate
{
public:
    kpNumInputPrivate(kpNumInput *q) :
        q(q),
        column1Width(0),
        column2Width(0),
        label(nullptr),
        slider(nullptr),
        labelAlignment()
    {
    }

    static kpNumInputPrivate *get(const kpNumInput *i)
    {
        return i->d;
    }

    kpNumInput *q;
    int column1Width, column2Width;

    QLabel  *label;
    QSlider *slider;
    QSize    sliderSize, labelSize;

    Qt::Alignment labelAlignment;
};

#define K_USING_kpNumInput_P(_d) kpNumInputPrivate *_d = kpNumInputPrivate::get(this)

kpNumInput::kpNumInput(QWidget *parent)
    : QWidget(parent), d(new kpNumInputPrivate(this))
{
    setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed));
    setFocusPolicy(Qt::StrongFocus);
}

kpNumInput::~kpNumInput()
{
    delete d;
}

QSlider *kpNumInput::slider() const
{
    return d->slider;
}

bool kpNumInput::showSlider() const
{
    return d->slider;
}

void kpNumInput::setLabel(const QString &label, Qt::Alignment a)
{
    if (label.isEmpty()) {
        delete d->label;
        d->label = nullptr;
        d->labelAlignment = {};
    } else {
        if (!d->label) {
            d->label = new QLabel(this);
        }
        d->label->setText(label);
        d->label->setObjectName(QStringLiteral("kpNumInput::QLabel"));
        d->label->setAlignment(a);
        // if no vertical alignment set, use Top alignment
        if (!(a & (Qt::AlignTop | Qt::AlignBottom | Qt::AlignVCenter))) {
            a |= Qt::AlignTop;
        }
        d->labelAlignment = a;
    }

    layout();
}

QString kpNumInput::label() const
{
    return d->label ? d->label->text() : QString();
}

void kpNumInput::layout()
{
    // label sizeHint
    d->labelSize = (d->label ? d->label->sizeHint() : QSize(0, 0));

    if (d->label && (d->labelAlignment & Qt::AlignVCenter)) {
        d->column1Width = d->labelSize.width() + 4;
    } else {
        d->column1Width = 0;
    }

    // slider sizeHint
    d->sliderSize = (d->slider ? d->slider->sizeHint() : QSize(0, 0));

    doLayout();

}

QSize kpNumInput::sizeHint() const
{
    return minimumSizeHint();
}

void kpNumInput::setSteps(int minor, int major)
{
    if (d->slider) {
        d->slider->setSingleStep(minor);
        d->slider->setPageStep(major);
    }
}

// ----------------------------------------------------------------------------

class kpIntNumInput::kpIntNumInputPrivate
{
public:
    kpIntNumInput *q;
    QSpinBox     *intSpinBox;
    QSize        intSpinBoxSize;

    kpIntNumInputPrivate(kpIntNumInput *q)
        : q(q) {}
};

kpIntNumInput::kpIntNumInput(QWidget *parent)
    : kpNumInput(parent)
    , d(new kpIntNumInputPrivate(this))
{
    initWidget(0);
}

kpIntNumInput::kpIntNumInput(int val, QWidget *parent)
    : kpNumInput(parent)
    , d(new kpIntNumInputPrivate(this))
{
    initWidget(val);
}

QSpinBox *kpIntNumInput::spinBox() const
{
    return d->intSpinBox;
}

void kpIntNumInput::initWidget(int val)
{
    d->intSpinBox = new QSpinBox(this);
    d->intSpinBox->setRange(INT_MIN, INT_MAX);
    d->intSpinBox->setSingleStep(1);
    d->intSpinBox->setValue(val);
    d->intSpinBox->setObjectName(QStringLiteral("kpIntNumInput::QSpinBox"));

    connect(d->intSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &kpIntNumInput::spinValueChanged);

    setFocusProxy(d->intSpinBox);
    layout();
}

void kpIntNumInput::spinValueChanged(int val)
{
    K_USING_kpNumInput_P(priv);

    if (priv->slider) {
        priv->slider->setValue(val);
    }

    emit valueChanged(val);
}

void kpIntNumInput::setRange(int lower, int upper, int singleStep)
{
    if (upper < lower || singleStep <= 0) {
        qDebug() << "WARNING: kpIntNumInput::setRange() called with bad arguments. Ignoring call...";
        return;
    }

    d->intSpinBox->setMinimum(lower);
    d->intSpinBox->setMaximum(upper);
    d->intSpinBox->setSingleStep(singleStep);

    singleStep = d->intSpinBox->singleStep(); // maybe QRangeControl didn't like our lineStep?

    layout();

    // update slider information
    K_USING_kpNumInput_P(priv);
    if (!priv->slider) {
        priv->slider = new QSlider(Qt::Horizontal, this);
        connect(priv->slider, &QSlider::valueChanged,
                d->intSpinBox, &QSpinBox::setValue);
        priv->slider->setTickPosition(QSlider::TicksBelow);
        layout();
    }

    const int value = d->intSpinBox->value();
    priv->slider->setRange(d->intSpinBox->minimum(), d->intSpinBox->maximum());
    priv->slider->setPageStep(d->intSpinBox->singleStep());
    priv->slider->setValue(value);
    // calculate (upper-lower)/10 without overflowing int's:
    const int major = calcDiffByTen(d->intSpinBox->maximum(), d->intSpinBox->minimum());

    priv->slider->setSingleStep(d->intSpinBox->singleStep());
    priv->slider->setPageStep(qMax(1, major));
    priv->slider->setTickInterval(major);
}

void kpIntNumInput::setMinimum(int min)
{
    setRange(min, d->intSpinBox->maximum(), d->intSpinBox->singleStep());
}

int kpIntNumInput::minimum() const
{
    return d->intSpinBox->minimum();
}

void kpIntNumInput::setMaximum(int max)
{
    setRange(d->intSpinBox->minimum(), max, d->intSpinBox->singleStep());
}

int kpIntNumInput::maximum() const
{
    return d->intSpinBox->maximum();
}

int kpIntNumInput::singleStep() const
{
    return d->intSpinBox->singleStep();
}

void kpIntNumInput::setSingleStep(int singleStep)
{
    d->intSpinBox->setSingleStep(singleStep);
}

void kpIntNumInput::setSuffix(const QString &suffix)
{
    d->intSpinBox->setSuffix(suffix);

    layout();
}

QString kpIntNumInput::suffix() const
{
    return d->intSpinBox->suffix();
}

void kpIntNumInput::setEditFocus(bool mark)
{
    if (mark)
    {
        d->intSpinBox->setFocus();
    }
}

QSize kpIntNumInput::minimumSizeHint() const
{
    K_USING_kpNumInput_P(priv);
    ensurePolished();

    int w;
    int h;

    h = qMax(d->intSpinBoxSize.height(), priv->sliderSize.height());

    // if in extra row, then count it here
    if (priv->label && (priv->labelAlignment & (Qt::AlignBottom | Qt::AlignTop))) {
        h += 4 + priv->labelSize.height();
    } else {
        // label is in the same row as the other widgets
        h = qMax(h, priv->labelSize.height() + 2);
    }

    w = priv->slider ? priv->slider->sizeHint().width() : 0;
    w += priv->column1Width + priv->column2Width;

    if (priv->labelAlignment & (Qt::AlignTop | Qt::AlignBottom)) {
        w = qMax(w, priv->labelSize.width() + 4);
    }

    return {w, h};
}

void kpIntNumInput::doLayout()
{
    K_USING_kpNumInput_P(priv);

    d->intSpinBoxSize = d->intSpinBox->sizeHint();
    priv->column2Width = d->intSpinBoxSize.width();

    if (priv->label) {
        priv->label->setBuddy(d->intSpinBox);
    }
}

void kpIntNumInput::resizeEvent(QResizeEvent *e)
{
    K_USING_kpNumInput_P(priv);

    int w = priv->column1Width;
    int h = 0;
    const int spacingHint = 0;//style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing);

    if (priv->label && (priv->labelAlignment & Qt::AlignTop)) {
        priv->label->setGeometry(0, 0, e->size().width(), priv->labelSize.height());
        h += priv->labelSize.height() + spacingHint;
    }

    if (priv->label && (priv->labelAlignment & Qt::AlignVCenter)) {
        priv->label->setGeometry(0, 0, w, d->intSpinBoxSize.height());
    }

    if (qApp->layoutDirection() == Qt::RightToLeft) {
        d->intSpinBox->setGeometry(w, h, priv->slider ? priv->column2Width : qMax(priv->column2Width, e->size().width() - w), d->intSpinBoxSize.height());
        w += priv->column2Width + spacingHint;

        if (priv->slider) {
            priv->slider->setGeometry(w, h, e->size().width() - w, d->intSpinBoxSize.height() + spacingHint);
        }
    } else if (priv->slider) {
        priv->slider->setGeometry(w, h, e->size().width() - (w + priv->column2Width + spacingHint), d->intSpinBoxSize.height() + spacingHint);
        d->intSpinBox->setGeometry(w + priv->slider->size().width() + spacingHint, h, priv->column2Width, d->intSpinBoxSize.height());
    } else {
        d->intSpinBox->setGeometry(w, h, qMax(priv->column2Width, e->size().width() - w), d->intSpinBoxSize.height());
    }

    h += d->intSpinBoxSize.height() + 2;

    if (priv->label && (priv->labelAlignment & Qt::AlignBottom)) {
        priv->label->setGeometry(0, h, priv->labelSize.width(), priv->labelSize.height());
    }
}

kpIntNumInput::~kpIntNumInput()
{
    delete d;
}

void kpIntNumInput::setValue(int val)
{
    d->intSpinBox->setValue(val);
    // slider value is changed by spinValueChanged
}

int kpIntNumInput::value() const
{
    return d->intSpinBox->value();
}

void kpIntNumInput::setSpecialValueText(const QString &text)
{
    d->intSpinBox->setSpecialValueText(text);
    layout();
}

QString kpIntNumInput::specialValueText() const
{
    return d->intSpinBox->specialValueText();
}

void kpIntNumInput::setLabel(const QString &label, Qt::Alignment a)
{
    K_USING_kpNumInput_P(priv);

    kpNumInput::setLabel(label, a);

    if (priv->label) {
        priv->label->setBuddy(d->intSpinBox);
    }
}

// ----------------------------------------------------------------------------

class kpDoubleNumInput::kpDoubleNumInputPrivate
{
public:
    kpDoubleNumInputPrivate()
        : spin(nullptr) {}
    QDoubleSpinBox *spin;
    QSize editSize;
    QString specialValue;
};

kpDoubleNumInput::kpDoubleNumInput(QWidget *parent)
    : kpNumInput(parent)
    , d(new kpDoubleNumInputPrivate())

{
    initWidget(0.0, 0.0, 9999.0, 0.01, 2);
}

kpDoubleNumInput::kpDoubleNumInput(double lower, double upper, double value, QWidget *parent,
                                 double singleStep, int precision)
    : kpNumInput(parent)
    , d(new kpDoubleNumInputPrivate())
{
    initWidget(value, lower, upper, singleStep, precision);
}

kpDoubleNumInput::~kpDoubleNumInput()
{
    delete d;
}

QString kpDoubleNumInput::specialValueText() const
{
    return d->specialValue;
}

void kpDoubleNumInput::initWidget(double value, double lower, double upper,
                                 double singleStep, int precision)
{
    d->spin = new QDoubleSpinBox(this);
    d->spin->setRange(lower, upper);
    d->spin->setSingleStep(singleStep);
    d->spin->setValue(value);
    d->spin->setDecimals(precision);

    d->spin->setObjectName(QStringLiteral("kpDoubleNumInput::QDoubleSpinBox"));
    setFocusProxy(d->spin);
    connect(d->spin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &kpDoubleNumInput::valueChanged);

    layout();
}

double kpDoubleNumInput::mapSliderToSpin(int val) const
{
    K_USING_kpNumInput_P(priv);

    // map [slidemin,slidemax] to [spinmin,spinmax]
    const double spinmin = d->spin->minimum();
    const double spinmax = d->spin->maximum();
    const double slidemin = priv->slider->minimum(); // cast int to double to avoid
    const double slidemax = priv->slider->maximum(); // overflow in rel denominator
    const double rel = (double(val) - slidemin) / (slidemax - slidemin);
    return spinmin + rel * (spinmax - spinmin);
}

void kpDoubleNumInput::sliderMoved(int val)
{
    d->spin->setValue(mapSliderToSpin(val));
}

void kpDoubleNumInput::spinBoxChanged(double val)
{
    K_USING_kpNumInput_P(priv);

    const double spinmin = d->spin->minimum();
    const double spinmax = d->spin->maximum();
    const double slidemin = priv->slider->minimum(); // cast int to double to avoid
    const double slidemax = priv->slider->maximum(); // overflow in rel denominator

    const double rel = (val - spinmin) / (spinmax - spinmin);

    if (priv->slider) {
        priv->slider->blockSignals(true);
        priv->slider->setValue(qRound(slidemin + rel * (slidemax - slidemin)));
        priv->slider->blockSignals(false);
    }
}

QSize kpDoubleNumInput::minimumSizeHint() const
{
    K_USING_kpNumInput_P(priv);

    ensurePolished();

    int w;
    int h;

    h = qMax(d->editSize.height(), priv->sliderSize.height());

    // if in extra row, then count it here
    if (priv->label && (priv->labelAlignment & (Qt::AlignBottom | Qt::AlignTop))) {
        h += 4 + priv->labelSize.height();
    } else {
        // label is in the same row as the other widgets
        h = qMax(h, priv->labelSize.height() + 2);
    }

    const int spacingHint = 0;//style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing);
    w = priv->slider ? priv->slider->sizeHint().width() + spacingHint : 0;
    w += priv->column1Width + priv->column2Width;

    if (priv->labelAlignment & (Qt::AlignTop | Qt::AlignBottom)) {
        w = qMax(w, priv->labelSize.width() + 4);
    }

    return {w, h};
}

void kpDoubleNumInput::resizeEvent(QResizeEvent *e)
{
    K_USING_kpNumInput_P(priv);

    int w = priv->column1Width;
    int h = 0;
    const int spacingHint = 0;//style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing);

    if (priv->label && (priv->labelAlignment & Qt::AlignTop)) {
        priv->label->setGeometry(0, 0, e->size().width(), priv->labelSize.height());
        h += priv->labelSize.height() + 4;
    }

    if (priv->label && (priv->labelAlignment & Qt::AlignVCenter)) {
        priv->label->setGeometry(0, 0, w, d->editSize.height());
    }

    if (qApp->layoutDirection() == Qt::RightToLeft) {
        d->spin->setGeometry(w, h, priv->slider ? priv->column2Width
                             : e->size().width() - w, d->editSize.height());
        w += priv->column2Width + spacingHint;

        if (priv->slider) {
            priv->slider->setGeometry(w, h, e->size().width() - w, d->editSize.height() + spacingHint);
        }
    } else if (priv->slider) {
        priv->slider->setGeometry(w, h, e->size().width() -
                                  (priv->column1Width + priv->column2Width + spacingHint),
                                  d->editSize.height() + spacingHint);
        d->spin->setGeometry(w + priv->slider->width() + spacingHint, h,
                             priv->column2Width, d->editSize.height());
    } else {
        d->spin->setGeometry(w, h, e->size().width() - w, d->editSize.height());
    }

    h += d->editSize.height() + 2;

    if (priv->label && (priv->labelAlignment & Qt::AlignBottom)) {
        priv->label->setGeometry(0, h, priv->labelSize.width(), priv->labelSize.height());
    }
}

void kpDoubleNumInput::doLayout()
{
    K_USING_kpNumInput_P(priv);

    d->editSize = d->spin->sizeHint();
    priv->column2Width = d->editSize.width();
}

void kpDoubleNumInput::setValue(double val)
{
    d->spin->setValue(val);
}

void kpDoubleNumInput::setRange(double lower, double upper, double singleStep)
{
    K_USING_kpNumInput_P(priv);

    QDoubleSpinBox *spin = d->spin;

    d->spin->setRange(lower, upper);
    d->spin->setSingleStep(singleStep);

    const double range = spin->maximum() - spin->minimum();
    const double steps = range * std::pow(10.0, spin->decimals());
    if (!priv->slider) {
        priv->slider = new QSlider(Qt::Horizontal, this);
        priv->slider->setTickPosition(QSlider::TicksBelow);
        // feedback line: when one moves, the other moves, too:
        connect(priv->slider, &QSlider::valueChanged,
                this, &kpDoubleNumInput::sliderMoved);
        layout();
    }
    if (steps > 1000 ) {
        priv->slider->setRange(0, 1000);
        priv->slider->setSingleStep(1);
        priv->slider->setPageStep(50);
    } else {
        const int singleSteps = qRound(steps);
        priv->slider->setRange(0, singleSteps);
        priv->slider->setSingleStep(1);
        const int pageSteps = qBound(1, singleSteps / 20, 10);
        priv->slider->setPageStep(pageSteps);
    }
    spinBoxChanged(spin->value());
    connect(spin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &kpDoubleNumInput::spinBoxChanged);

    layout();
}

void kpDoubleNumInput::setMinimum(double min)
{
    setRange(min, maximum(), d->spin->singleStep());
}

double kpDoubleNumInput::minimum() const
{
    return d->spin->minimum();
}

void kpDoubleNumInput::setMaximum(double max)
{
    setRange(minimum(), max, d->spin->singleStep());
}

double kpDoubleNumInput::maximum() const
{
    return d->spin->maximum();
}

double kpDoubleNumInput::singleStep() const
{
    return d->spin->singleStep();
}

void kpDoubleNumInput::setSingleStep(double singleStep)
{
    d->spin->setSingleStep(singleStep);
}

double kpDoubleNumInput::value() const
{
    return d->spin->value();
}

QString kpDoubleNumInput::suffix() const
{
    return d->spin->suffix();
}

void kpDoubleNumInput::setSuffix(const QString &suffix)
{
    d->spin->setSuffix(suffix);

    layout();
}

void kpDoubleNumInput::setDecimals(int decimals)
{
    d->spin->setDecimals(decimals);

    layout();
}

int kpDoubleNumInput::decimals() const
{
    return d->spin->decimals();
}

void kpDoubleNumInput::setSpecialValueText(const QString &text)
{
    d->spin->setSpecialValueText(text);

    layout();
}

void kpDoubleNumInput::setLabel(const QString &label, Qt::Alignment a)
{
    K_USING_kpNumInput_P(priv);

    kpNumInput::setLabel(label, a);

    if (priv->label) {
        priv->label->setBuddy(d->spin);
    }
}

#include "moc_kpNumInput.cpp"
