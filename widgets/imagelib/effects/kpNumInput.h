/* This file is part of the KDE libraries
 *  Copyright (c) 1997 Patrick Dowler <dowler@morgul.fsh.uvic.ca>
 *  Copyright (c) 2000 Dirk Mueller <mueller@kde.org>
 *  Copyright (c) 2002 Marc Mutz <mutz@kde.org>
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

#ifndef kpNumInput_H
#define kpNumInput_H

#include <QWidget>
#include <QSpinBox>

class QSlider;
class QSpinBox;

class kpNumInputPrivate;

/**
 * You need to inherit from this class if you want to implement K*NumInput
 * for a different variable type
 *
 */
class kpNumInput : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString label READ label WRITE setLabel)
public:
    /**
     * Default constructor
     * @param parent If parent is 0, the new widget becomes a top-level
     * window. If parent is another widget, this widget becomes a child
     * window inside parent. The new widget is deleted when its parent is deleted.
     */
    explicit kpNumInput(QWidget *parent = nullptr);

    /**
     * Destructor
     */
    ~kpNumInput();

    /**
     * Sets the text and alignment of the main description label.
     *
     * @param label The text of the label.
     *              Use QString() to remove an existing one.
     *
     * @param a The alignment of the label (Qt::Alignment).
     *          Default is @p Qt:AlignLeft | @p Qt:AlignTop.
     *
     * The vertical alignment flags have special meaning with this
     * widget:
     *
     *     @li @p Qt:AlignTop     The label is placed above the edit/slider
     *     @li @p Qt:AlignVCenter The label is placed left beside the edit
     *     @li @p Qt:AlignBottom  The label is placed below the edit/slider
     *
     */
    virtual void setLabel(const QString &label, Qt::Alignment a = Qt::AlignLeft | Qt::AlignTop);

    /**
     * @return the text of the label.
     */
    QString label() const;

    /**
     * @return if the num input has a slider.
     */
    bool showSlider() const;

    /**
     * Sets the spacing of tickmarks for the slider.
     *
     * @param minor Minor tickmark separation.
     * @param major Major tickmark separation.
     */
    void setSteps(int minor, int major);

    /**
     * Returns a size which fits the contents of the control.
     *
     * @return the preferred size necessary to show the control
     */
    QSize sizeHint() const Q_DECL_OVERRIDE;

protected:
    /**
     * @return the slider widget.
     * @internal
     */
    QSlider *slider() const;

    /**
     * Call this function whenever you change something in the geometry
     * of your kpNumInput child.
     *
     */
    void layout();

    /**
     * You need to overwrite this method and implement your layout
     * calculations there.
     *
     * See kpIntNumInput::doLayout and kpDoubleNumInput::doLayout implementation
     * for details.
     *
     */
    virtual void doLayout() = 0;

private:
    friend class kpNumInputPrivate;
    kpNumInputPrivate *const d;

    Q_DISABLE_COPY(kpNumInput)
};

/* ------------------------------------------------------------------------ */

/**
 * @short An input widget for integer numbers, consisting of a spinbox and a slider.
 *
 * kpIntNumInput combines a QSpinBox and optionally a QSlider
 * with a label to make an easy to use control for setting some integer
 * parameter. This is especially nice for configuration dialogs,
 * which can have many such combinated controls.
 *
 * A special feature of kpIntNumInput, designed specifically for
 * the situation when there are several kpIntNumInputs in a column,
 * is that you can specify what portion of the control is taken by the
 * QSpinBox (the remaining portion is used by the slider). This makes
 * it very simple to have all the sliders in a column be the same size.
 *
 * kpIntNumInput enforces the value to be in the given range.
 */

class kpIntNumInput : public kpNumInput
{
    Q_OBJECT
    Q_PROPERTY(int value READ value WRITE setValue NOTIFY valueChanged USER true)
    Q_PROPERTY(int minimum READ minimum WRITE setMinimum)
    Q_PROPERTY(int maximum READ maximum WRITE setMaximum)
    Q_PROPERTY(int singleStep READ singleStep WRITE setSingleStep)
    Q_PROPERTY(QString suffix READ suffix WRITE setSuffix)
    Q_PROPERTY(QString specialValueText READ specialValueText WRITE setSpecialValueText)

public:
    /**
     * Constructs an input control for integer values
     * with initial value 0.
     */
    explicit kpIntNumInput(QWidget *parent = nullptr);
    /**
     * Constructor
     * It constructs a QSpinBox that allows the input of integer numbers
     * in the range of -INT_MAX to +INT_MAX. To set a descriptive label,
     * use setLabel(). To enforce the value being in a range and optionally to
     * attach a slider to it, use setRange().
     *
     * @param value  initial value for the control
     * @param parent parent QWidget
     */
    explicit kpIntNumInput(int value, QWidget *parent = nullptr);

    /**
     * Destructor
     *
     *
     */
    virtual ~kpIntNumInput();

    /**
     * @return the current value.
     */
    int value() const;

    /**
     * @return the suffix displayed behind the value.
     * @see setSuffix()
     */
    QString suffix() const;
    /**
     * @return the string displayed for a special value.
     * @see setSpecialValueText()
     */
    QString specialValueText() const;

    /**
     * Sets the allowed input range and the step size for the slider and the
     * spin box.
     *
     * @param min  minimum value
     * @param max  maximum value
     * @param step step size
     */
    void setRange(int min, int max, int singleStep = 1);

    /**
     * Sets the minimum value.
     */
    void setMinimum(int min);
    /**
     * @return the minimum value.
     */
    int minimum() const;
    /**
     * Sets the maximum value.
     */
    void setMaximum(int max);
    /**
     * @return the maximum value.
     */
    int maximum() const;

    /**
     * @return the step of the spin box
     */
    int singleStep() const;

    /**
     * @return the step of the spin box
     */
    void setSingleStep(int step);

    /**
     * Sets the special value text. If set, the SpinBox will display
     * this text instead of the numeric value whenever the current
     * value is equal to minVal(). Typically this is used for indicating
     * that the choice has a special (default) meaning.
     */
    void setSpecialValueText(const QString &text);

    void setLabel(const QString &label, Qt::Alignment a = Qt::AlignLeft | Qt::AlignTop) Q_DECL_OVERRIDE;

    /**
     * This method returns the minimum size necessary to display the
     * control. The minimum size is enough to show all the labels
     * in the current font (font change may invalidate the return value).
     *
     * @return the minimum size necessary to show the control
     */
    QSize minimumSizeHint() const Q_DECL_OVERRIDE;

public Q_SLOTS:
    /**
     * Sets the value of the control.
     */
    void setValue(int);

    /**
     * Sets the suffix to @p suffix.
     * Use QString() to disable this feature.
     * Formatting has to be provided (e.g. a space separator between the
     * prepended @p value and the suffix's text has to be provided
     * as the first character in the suffix).
     *
     * @see QSpinBox::setSuffix()
     */
    void setSuffix(const QString &suffix);

    /**
     * sets focus to the edit widget and marks all text in if mark == true
     *
     */
    void setEditFocus(bool mark = true);

Q_SIGNALS:
    /**
     * Emitted every time the value changes (by calling setValue() or
     * by user interaction).
     */
    void valueChanged(int);


private Q_SLOTS:
    void spinValueChanged(int);

protected:
    /**
     * @return the spin box widget.
     * @internal
     */
    QSpinBox *spinBox() const;

    void doLayout() Q_DECL_OVERRIDE;
    void resizeEvent(QResizeEvent *) Q_DECL_OVERRIDE;

private:
    void initWidget(int value);

private:
    class kpIntNumInputPrivate;
    friend class kpIntNumInputPrivate;
    kpIntNumInputPrivate *const d;

    Q_DISABLE_COPY(kpIntNumInput)
};

/* ------------------------------------------------------------------------ */

class kpDoubleLine;

/**
 * @short An input control for real numbers, consisting of a spinbox and a slider.
 *
 * kpDoubleNumInput combines a QSpinBox and optionally a QSlider
 * with a label to make an easy to use control for setting some float
 * parameter. This is especially nice for configuration dialogs,
 * which can have many such combinated controls.
 *
 * The slider is created only when the user specifies a range
 * for the control using the setRange function with the slider
 * parameter set to "true".
 *
 * A special feature of kpDoubleNumInput, designed specifically for
 * the situation when there are several instances in a column,
 * is that you can specify what portion of the control is taken by the
 * QSpinBox (the remaining portion is used by the slider). This makes
 * it very simple to have all the sliders in a column be the same size.
 *
 * @see kpIntNumInput
 */

class kpDoubleNumInput : public kpNumInput
{
    Q_OBJECT
    Q_PROPERTY(double value READ value WRITE setValue NOTIFY valueChanged USER true)
    Q_PROPERTY(double minimum READ minimum WRITE setMinimum)
    Q_PROPERTY(double maximum READ maximum WRITE setMaximum)
    Q_PROPERTY(double singleStep READ singleStep WRITE setSingleStep)
    Q_PROPERTY(QString suffix READ suffix WRITE setSuffix)
    Q_PROPERTY(QString specialValueText READ specialValueText WRITE setSpecialValueText)
    Q_PROPERTY(int decimals READ decimals WRITE setDecimals)

public:
    /**
     * Constructs an input control for double values
     * with initial value 0.00.
     */
    explicit kpDoubleNumInput(QWidget *parent = nullptr);

    /**
     * Constructor
     *
     * @param lower lower boundary value
     * @param upper upper boundary value
     * @param value  initial value for the control
     * @param singleStep   step size to use for up/down arrow clicks
     * @param precision number of digits after the decimal point
     * @param parent parent QWidget
     */
    kpDoubleNumInput(double lower, double upper, double value, QWidget *parent = nullptr, double singleStep = 0.01,
                    int precision = 2);

    /**
     * destructor
     */
    virtual ~kpDoubleNumInput();

    /**
     * @return the current value.
     */
    double value() const;

    /**
     * @return the suffix.
     * @see setSuffix()
     */
    QString suffix() const;

    /**
     * @return number of decimals.
     * @see setDecimals()
     */
    int decimals() const;

    /**
     * @return the string displayed for a special value.
     * @see setSpecialValueText()
     */
    QString specialValueText() const;

    /**
    * @param min  minimum value
    * @param max  maximum value
    * @param singleStep step size for the QSlider
    * @param slider whether the slider is created or not
    */
    void setRange(double min, double max, double singleStep = 1);

    /**
     * Sets the minimum value.
     */
    void setMinimum(double min);
    /**
     * @return the minimum value.
     */
    double minimum() const;
    /**
     * Sets the maximum value.
     */
    void setMaximum(double max);
    /**
     * @return the maximum value.
     */
    double maximum() const;

    /**
     * @return the step of the spin box
     */
    double singleStep() const;

    /**
     * @return the step of the spin box
     */
    void setSingleStep(double singleStep);

    /**
     * Specifies the number of digits to use.
     */
    void setDecimals(int decimals);

    /**
     * Sets the special value text. If set, the spin box will display
     * this text instead of the numeric value whenever the current
     * value is equal to minVal(). Typically this is used for indicating
     * that the choice has a special (default) meaning.
     */
    void setSpecialValueText(const QString &text);

    void setLabel(const QString &label, Qt::Alignment a = Qt::AlignLeft | Qt::AlignTop) Q_DECL_OVERRIDE;
    QSize minimumSizeHint() const Q_DECL_OVERRIDE;

public Q_SLOTS:
    /**
     * Sets the value of the control.
     */
    void setValue(double);

    /**
     * Sets the suffix to be displayed to @p suffix. Use QString() to disable
     * this feature. Note that the suffix is attached to the value without any
     * spacing. So if you prefer to display a space separator, set suffix
     * to something like " cm".
     * @see setSuffix()
     */
    void setSuffix(const QString &suffix);

Q_SIGNALS:
    /**
     * Emitted every time the value changes (by calling setValue() or
     * by user interaction).
     */
    void valueChanged(double);

private Q_SLOTS:
    void sliderMoved(int);
    void spinBoxChanged(double);

protected:
    void doLayout() Q_DECL_OVERRIDE;
    void resizeEvent(QResizeEvent *) Q_DECL_OVERRIDE;

    friend class kpDoubleLine;
private:
    void initWidget(double value, double lower, double upper,
                    double singleStep, int precision);
    double mapSliderToSpin(int) const;

private:
    class kpDoubleNumInputPrivate;
    friend class kpDoubleNumInputPrivate;
    kpDoubleNumInputPrivate *const d;

    Q_DISABLE_COPY(kpDoubleNumInput)
};


#endif // kpNumInput_H
