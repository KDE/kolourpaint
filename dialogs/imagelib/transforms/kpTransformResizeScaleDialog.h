/*
   Copyright (c) 2003-2007 Clarence Dang <dang@kde.org>
   Copyright (c) 2011 Martin Koller <kollix@aon.at>
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

   1. Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
   2. Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
   IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
   OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
   IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
   INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
   NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
   THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef kpTransformResizeScaleDialog_H
#define kpTransformResizeScaleDialog_H

#include <QDialog>

#include "imagelib/kpColor.h"
#include "commands/imagelib/transforms/kpTransformResizeScaleCommand.h"


class QCheckBox;
class QComboBox;
class QGroupBox;
class QToolButton;
class QSpinBox;
class QDoubleSpinBox;

class kpAbstractSelection;
class kpDocument;
class kpTextSelection;
class kpTransformDialogEnvironment;


class kpTransformResizeScaleDialog : public QDialog
{
Q_OBJECT

  public:
    kpTransformResizeScaleDialog(kpTransformDialogEnvironment *_env, QWidget *parent);

    enum ActOn
    {
        Image, Selection
    };

    int imageWidth () const;
    int imageHeight () const;
    bool actOnSelection () const;
    kpTransformResizeScaleCommand::Type type () const;

    bool isNoOp () const;

  public slots:
    void slotActOnChanged ();
    void slotTypeChanged ();

    void slotWidthChanged (int width);
    void slotHeightChanged (int height);

    void slotPercentWidthChanged (double percentWidth);
    void slotPercentHeightChanged (double percentHeight);

  private:
    kpDocument *document () const;
    kpAbstractSelection *selection () const;
    kpTextSelection *textSelection () const;

    QWidget *createActOnBox(QWidget *baseWidget);
    QGroupBox *createOperationGroupBox(QWidget *baseWidget);
    QGroupBox *createDimensionsGroupBox(QWidget *baseWidget);

    void widthFitHeightToAspectRatio ();
    void heightFitWidthToAspectRatio ();

    bool resizeEnabled () const;
    bool scaleEnabled () const;
    bool smoothScaleEnabled () const;
    int originalWidth () const;
    int originalHeight () const;

  private slots:
    void accept() override;
    void setKeepAspectRatio(bool on);

  private:
    kpTransformDialogEnvironment *m_environ;

    QComboBox *m_actOnCombo;

    QToolButton *m_resizeButton,
                *m_scaleButton,
                *m_smoothScaleButton;

    QSpinBox *m_originalWidthInput, *m_originalHeightInput,
             *m_newWidthInput, *m_newHeightInput;
    QDoubleSpinBox *m_percentWidthInput, *m_percentHeightInput;
    QCheckBox *m_keepAspectRatioCheckBox;

    int m_ignoreKeepAspectRatio;

    kpTransformResizeScaleCommand::Type m_lastType;
};


#endif  // kpTransformResizeScaleDialog_H
