
/*
   Copyright (c) 2003-2004 Clarence Dang <dang@kde.org>
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

#define DEBUG_KP_TOOL_RESIZE_SCALE_COMMAND 0
#define DEBUG_KP_TOOL_RESIZE_SCALE_DIALOG 0

#include <math.h>

#include <qapplication.h>
#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qgroupbox.h>
#include <kiconloader.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpoint.h>
#include <qradiobutton.h>
#include <qrect.h>

#include <kdebug.h>
#include <klocale.h>
#include <knuminput.h>

#include <kpdefs.h>
#include <kpdocument.h>
#include <kpmainwindow.h>
#include <kppixmapfx.h>
#include <kpselection.h>
#include <kptool.h>
#include <kptoolresizescale.h>


/*
 * kpToolResizeScaleCommand
 */

kpToolResizeScaleCommand::kpToolResizeScaleCommand (bool actOnSelection,
                                                    int newWidth, int newHeight,
                                                    Type type,
                                                    kpMainWindow *mainWindow)
    : m_actOnSelection (actOnSelection),
      m_newWidth (newWidth), m_newHeight (newHeight),
      m_type (type),
      m_mainWindow (mainWindow),
      m_backgroundColor (mainWindow ? mainWindow->backgroundColor () : kpColor::invalid)
{
    kpDocument *doc = document ();

    m_oldWidth = doc->width (m_actOnSelection);
    m_oldHeight = doc->height (m_actOnSelection);

    m_actOnTextSelection = (m_actOnSelection &&
                            doc && doc->selection () &&
                            doc->selection ()->isText ());

    m_isLosslessScale = ((m_type == Scale) &&
                         (m_newWidth / m_oldWidth * m_oldWidth == m_newWidth) &&
                         (m_newHeight / m_oldHeight * m_oldHeight == m_newHeight));
}

// virtual
QString kpToolResizeScaleCommand::name () const
{
    QString opName;

    switch (m_type)
    {
    case Resize:
        opName = i18n ("Resize");
        break;
    case Scale:
        opName = i18n ("Scale");
        break;
    case SmoothScale:
        opName = i18n ("Smooth Scale");
        break;
    }

    if (m_actOnSelection)
    {
        if (m_actOnTextSelection)
            return i18n ("Text: %1 Box").arg (opName);
        else
            return i18n ("Selection: %1").arg (opName);
    }
    else
        return opName;
}

kpToolResizeScaleCommand::~kpToolResizeScaleCommand ()
{
}


// private
kpDocument *kpToolResizeScaleCommand::document () const
{
    return m_mainWindow ? m_mainWindow->document () : 0;
}


// public virtual [base KCommand]
void kpToolResizeScaleCommand::execute ()
{
#if DEBUG_KP_TOOL_RESIZE_SCALE_COMMAND
    kdDebug () << "kpToolResizeScaleCommand::execute() type="
               << (int) m_type
               << " oldWidth=" << m_oldWidth
               << " oldHeight=" << m_oldHeight
               << " newWidth=" << m_newWidth
               << " newHeight=" << m_newHeight
               << endl;
#endif

    if (m_oldWidth == m_newWidth && m_oldHeight == m_newHeight)
        return;

    if (m_type == Resize)
    {
        if (m_actOnSelection)
        {
            if (!m_actOnTextSelection)
            {
                kdError () << "kpToolResizeScaleCommand::execute() resizing sel doesn't make sense" << endl;
                return;
            }
            else
            {
                QApplication::setOverrideCursor (Qt::waitCursor);
                document ()->selection ()->textResize (m_newWidth, m_newHeight);

                if (m_mainWindow->tool ())
                    m_mainWindow->tool ()->somethingBelowTheCursorChanged ();

                QApplication::restoreOverrideCursor ();
            }
        }
        else
        {
            QApplication::setOverrideCursor (Qt::waitCursor);


            if (m_newWidth < m_oldWidth)
            {
                m_oldRightPixmap = kpPixmapFX::getPixmapAt (
                    *document ()->pixmap (),
                    QRect (m_newWidth, 0,
                        m_oldWidth - m_newWidth, m_oldHeight));
            }

            if (m_newHeight < m_oldHeight)
            {
                m_oldBottomPixmap = kpPixmapFX::getPixmapAt (
                    *document ()->pixmap (),
                    QRect (0, m_newHeight,
                        m_newWidth, m_oldHeight - m_newHeight));
            }

            document ()->resize (m_newWidth, m_newHeight, m_backgroundColor);


            QApplication::restoreOverrideCursor ();
        }
    }
    else
    {
        QApplication::setOverrideCursor (Qt::waitCursor);


        QPixmap oldPixmap = *document ()->pixmap (m_actOnSelection);

        if (!m_isLosslessScale)
            m_oldPixmap = oldPixmap;

        QPixmap newPixmap = kpPixmapFX::scale (oldPixmap, m_newWidth, m_newHeight,
                                               m_type == SmoothScale);


        if (m_actOnSelection)
        {
            // Save sel border
            m_oldSelection = *document ()->selection ();
            m_oldSelection.setPixmap (QPixmap ());

            QRect newRect = QRect (m_oldSelection.x (), m_oldSelection.y (),
                                   newPixmap.width (), newPixmap.height ());

            // Not possible to retain non-rectangular selection borders on scale
            // (think about e.g. a 45 deg line as part of the border & 2x scale)
            document ()->setSelection (
                kpSelection (kpSelection::Rectangle, newRect, newPixmap,
                             m_oldSelection.transparency ()));

            if (m_mainWindow->tool ())
                m_mainWindow->tool ()->somethingBelowTheCursorChanged ();
        }
        else
        {
            document ()->setPixmap (newPixmap);
        }


        QApplication::restoreOverrideCursor ();
    }
}

// public virtual [base KCommand]
void kpToolResizeScaleCommand::unexecute ()
{
#if DEBUG_KP_TOOL_RESIZE_SCALE_COMMAND
    kdDebug () << "kpToolResizeScaleCommand::unexecute() type="
               << m_type << endl;
#endif

    if (m_oldWidth == m_newWidth && m_oldHeight == m_newHeight)
        return;

    kpDocument *doc = document ();
    if (!doc)
        return;

    if (m_type == Resize)
    {
        if (m_actOnSelection)
        {
            if (!m_actOnTextSelection)
            {
                kdError () << "kpToolResizeScaleCommand::unexecute() resizing sel doesn't make sense" << endl;
                return;
            }
            else
            {
                QApplication::setOverrideCursor (Qt::waitCursor);
                document ()->selection ()->textResize (m_oldWidth, m_oldHeight);

                if (m_mainWindow->tool ())
                    m_mainWindow->tool ()->somethingBelowTheCursorChanged ();

                QApplication::restoreOverrideCursor ();
            }
        }
        else
        {
            QApplication::setOverrideCursor (Qt::waitCursor);


            QPixmap newPixmap (m_oldWidth, m_oldHeight);

            kpPixmapFX::setPixmapAt (&newPixmap, QPoint (0, 0),
                                    *doc->pixmap ());

            if (m_newWidth < m_oldWidth)
            {
                kpPixmapFX::setPixmapAt (&newPixmap,
                                        QPoint (m_newWidth, 0),
                                        m_oldRightPixmap);
            }

            if (m_newHeight < m_oldHeight)
            {
                kpPixmapFX::setPixmapAt (&newPixmap,
                                        QPoint (0, m_newHeight),
                                        m_oldBottomPixmap);
            }

            doc->setPixmap (newPixmap);


            QApplication::restoreOverrideCursor ();
        }
    }
    else
    {
        QApplication::setOverrideCursor (Qt::waitCursor);


        QPixmap oldPixmap;

        if (!m_isLosslessScale)
            oldPixmap = m_oldPixmap;
        else
            oldPixmap = kpPixmapFX::scale (*doc->pixmap (m_actOnSelection),
                                           m_oldWidth, m_oldHeight);


        if (m_actOnSelection)
        {
            kpSelection oldSelection = m_oldSelection;
            oldSelection.setPixmap (oldPixmap);
            doc->setSelection (oldSelection);

            if (m_mainWindow->tool ())
                m_mainWindow->tool ()->somethingBelowTheCursorChanged ();
        }
        else
            doc->setPixmap (oldPixmap);


        QApplication::restoreOverrideCursor ();
    }
}


/*
 * kpToolResizeScaleDialog
 */

#define SET_VALUE_WITHOUT_SIGNAL_EMISSION(knuminput_instance,value)    \
{                                                                      \
    knuminput_instance->blockSignals (true);                           \
    knuminput_instance->setValue (value);                              \
    knuminput_instance->blockSignals (false);                          \
}

#define IGNORE_KEEP_ASPECT_RATIO(cmd) \
{                                     \
    m_ignoreKeepAspectRatio++;        \
    cmd;                              \
    m_ignoreKeepAspectRatio--;        \
}


// private static
kpToolResizeScaleCommand::Type kpToolResizeScaleDialog::s_lastType =
    kpToolResizeScaleCommand::Resize;

// private static
double kpToolResizeScaleDialog::s_lastPercentWidth = 100,
       kpToolResizeScaleDialog::s_lastPercentHeight = 100;

// private static
bool kpToolResizeScaleDialog::s_lastKeepAspectRatio = false;


kpToolResizeScaleDialog::kpToolResizeScaleDialog (bool actOnSelection,
                                                  kpMainWindow *mainWindow)
    : KDialogBase ((QWidget *) mainWindow,
                   0/*name*/,
                   true/*modal*/,
                   actOnSelection ? i18n ("Scale Selection") : i18n ("Resize / Scale Image"),
                   KDialogBase::Ok | KDialogBase::Cancel),
      m_actOnSelection (actOnSelection),
      m_ignoreKeepAspectRatio (0)
{
    kpDocument *document = mainWindow->document ();
    m_oldWidth = document->width (actOnSelection);
    m_oldHeight = document->height (actOnSelection);

    m_actOnTextSelection = (m_actOnSelection &&
                            document && document->selection () &&
                            document->selection ()->isText ());


    // Using the percentage from last time become too confusing so disable for now
    s_lastPercentWidth = 100, s_lastPercentHeight = 100;


    QWidget *baseWidget = new QWidget (this);
    setMainWidget (baseWidget);


    createOperationGroupBox (baseWidget);
    createDimensionsGroupBox (baseWidget);


    QVBoxLayout *baseLayout = new QVBoxLayout (baseWidget, marginHint (), spacingHint ());
    baseLayout->addWidget (m_operationGroupBox);
    baseLayout->addWidget (m_dimensionsGroupBox);


    //enableButtonOK (!isNoOp ());
}

kpToolResizeScaleDialog::~kpToolResizeScaleDialog ()
{
}


// private
void kpToolResizeScaleDialog::createOperationGroupBox (QWidget *baseWidget)
{
    m_operationGroupBox = new QGroupBox (i18n ("Operation"), baseWidget);

    QLabel *resizePixmapLabel = new QLabel (m_operationGroupBox);
    resizePixmapLabel->setPixmap (UserIcon ("resize_scale_apple_resize"));

    QLabel *scalePixmapLabel = new QLabel (m_operationGroupBox);
    scalePixmapLabel->setPixmap (UserIcon ("resize_scale_apple_scale"));

    QLabel *smoothScalePixmapLabel = new QLabel (m_operationGroupBox);
    smoothScalePixmapLabel->setPixmap (UserIcon ("resize_scale_apple_smooth_scale"));


    m_resizeRadioButton = new QRadioButton (i18n ("&Resize"), m_operationGroupBox);
    m_scaleRadioButton = new QRadioButton (i18n ("&Scale"), m_operationGroupBox);
    m_smoothScaleRadioButton = new QRadioButton (i18n ("S&mooth Scale"), m_operationGroupBox);

    if (m_actOnSelection)
    {
        if (m_actOnTextSelection)
        {
            m_resizeRadioButton->setChecked (true);

            m_operationGroupBox->setEnabled (!m_actOnSelection);
        }
        else
        {
            if (s_lastType == kpToolResizeScaleCommand::Scale)
                m_scaleRadioButton->setChecked (true);
            else
                m_smoothScaleRadioButton->setChecked (true);

            resizePixmapLabel->setEnabled (false);
            m_resizeRadioButton->setEnabled (false);
        }
    }
    else
    {
        if (s_lastType == kpToolResizeScaleCommand::Resize)
            m_resizeRadioButton->setChecked (true);
        else if (s_lastType == kpToolResizeScaleCommand::Scale)
            m_scaleRadioButton->setChecked (true);
        else
            m_smoothScaleRadioButton->setChecked (true);
    }



    QButtonGroup *resizeScaleButtonGroup = new QButtonGroup (baseWidget);
    resizeScaleButtonGroup->hide ();

    resizeScaleButtonGroup->insert (m_resizeRadioButton);
    resizeScaleButtonGroup->insert (m_scaleRadioButton);
    resizeScaleButtonGroup->insert (m_smoothScaleRadioButton);


    QGridLayout *operationLayout = new QGridLayout (m_operationGroupBox,
                                                    2, 2,
                                                    marginHint () * 2/*don't overlap groupbox title*/,
                                                    spacingHint () * 4/*more spacing between pics*/);

    operationLayout->addWidget (resizePixmapLabel, 0, 0, Qt::AlignCenter);
    operationLayout->addWidget (m_resizeRadioButton, 1, 0, Qt::AlignCenter);

    operationLayout->addWidget (scalePixmapLabel, 0, 1, Qt::AlignCenter);
    operationLayout->addWidget (m_scaleRadioButton, 1, 1, Qt::AlignCenter);

    operationLayout->addWidget (smoothScalePixmapLabel, 0, 2, Qt::AlignCenter);
    operationLayout->addWidget (m_smoothScaleRadioButton, 1, 2, Qt::AlignCenter);


    connect (m_resizeRadioButton, SIGNAL (toggled (bool)),
             this, SLOT (slotTypeChanged ()));
    connect (m_scaleRadioButton, SIGNAL (toggled (bool)),
             this, SLOT (slotTypeChanged ()));
    connect (m_smoothScaleRadioButton, SIGNAL (toggled (bool)),
             this, SLOT (slotTypeChanged ()));
}

// private
void kpToolResizeScaleDialog::createDimensionsGroupBox (QWidget *baseWidget)
{
    m_dimensionsGroupBox = new QGroupBox (i18n ("Dimensions"), baseWidget);

    QLabel *widthLabel = new QLabel (i18n ("Width:"), m_dimensionsGroupBox);
    widthLabel->setAlignment (widthLabel->alignment () | Qt::AlignHCenter);
    QLabel *heightLabel = new QLabel (i18n ("Height:"), m_dimensionsGroupBox);
    heightLabel->setAlignment (heightLabel->alignment () | Qt::AlignHCenter);

    QLabel *originalLabel = new QLabel (i18n ("Original:"), m_dimensionsGroupBox);
    KIntNumInput *originalWidthInput = new KIntNumInput (m_oldWidth, m_dimensionsGroupBox);
    QLabel *xLabel0 = new QLabel (i18n ("x"), m_dimensionsGroupBox);
    KIntNumInput *originalHeightInput = new KIntNumInput (m_oldHeight, m_dimensionsGroupBox);

    QLabel *newLabel = new QLabel (i18n ("&New:"), m_dimensionsGroupBox);
    m_newWidthInput = new KIntNumInput (m_dimensionsGroupBox);
    m_newWidthInput->setMinValue (!m_actOnTextSelection ? 1 : kpSelection::minimumWidth ());
    QLabel *xLabel1 = new QLabel (i18n ("x"), m_dimensionsGroupBox);
    m_newHeightInput = new KIntNumInput (m_dimensionsGroupBox);
    m_newHeightInput->setMinValue (!m_actOnTextSelection ? 1 : kpSelection::minimumHeight ());

    QLabel *percentLabel = new QLabel (i18n ("&Percent:"), m_dimensionsGroupBox);
    m_percentWidthInput = new KDoubleNumInput (0.01/*lower*/, 1000000/*upper*/,
                                               100/*value*/, 1/*step*/,
                                               2/*precision*/,
                                               m_dimensionsGroupBox);
    m_percentWidthInput->setSuffix (i18n ("%"));
    QLabel *xLabel2 = new QLabel (i18n ("x"), m_dimensionsGroupBox);
    m_percentHeightInput = new KDoubleNumInput (0.01/*lower*/, 1000000/*upper*/,
                                                100/*value*/, 1/*step*/,
                                                2/*precision*/,
                                                m_dimensionsGroupBox);
    m_percentHeightInput->setSuffix (i18n ("%"));

    m_keepAspectRatioCheckBox = new QCheckBox (i18n ("Keep &aspect ratio"),
                                               m_dimensionsGroupBox);


    originalWidthInput->setEnabled (false);
    originalHeightInput->setEnabled (false);
    originalLabel->setBuddy (originalWidthInput);
    newLabel->setBuddy (m_newWidthInput);
    m_percentWidthInput->setValue (s_lastPercentWidth);
    m_percentHeightInput->setValue (s_lastPercentHeight);
    percentLabel->setBuddy (m_percentWidthInput);
    m_keepAspectRatioCheckBox->setChecked (s_lastKeepAspectRatio);


    QGridLayout *dimensionsLayout = new QGridLayout (m_dimensionsGroupBox,
                                                     5, 4, marginHint () * 2, spacingHint ());
    dimensionsLayout->setColStretch (1/*column*/, 1);
    dimensionsLayout->setColStretch (3/*column*/, 1);


    dimensionsLayout->addWidget (widthLabel, 0, 1);
    dimensionsLayout->addWidget (heightLabel, 0, 3);

    dimensionsLayout->addWidget (originalLabel, 1, 0);
    dimensionsLayout->addWidget (originalWidthInput, 1, 1);
    dimensionsLayout->addWidget (xLabel0, 1, 2);
    dimensionsLayout->addWidget (originalHeightInput, 1, 3);

    dimensionsLayout->addWidget (newLabel, 2, 0);
    dimensionsLayout->addWidget (m_newWidthInput, 2, 1);
    dimensionsLayout->addWidget (xLabel1, 2, 2);
    dimensionsLayout->addWidget (m_newHeightInput, 2, 3);

    dimensionsLayout->addWidget (percentLabel, 3, 0);
    dimensionsLayout->addWidget (m_percentWidthInput, 3, 1);
    dimensionsLayout->addWidget (xLabel2, 3, 2);
    dimensionsLayout->addWidget (m_percentHeightInput, 3, 3);

    dimensionsLayout->addMultiCellWidget (m_keepAspectRatioCheckBox, 4, 4, 0, 3);
    dimensionsLayout->setRowStretch (4/*row*/, 1);
    dimensionsLayout->setRowSpacing (4/*row*/, dimensionsLayout->rowSpacing (4) * 2);


    connect (m_newWidthInput, SIGNAL (valueChanged (int)),
             this, SLOT (slotWidthChanged (int)));
    connect (m_newHeightInput, SIGNAL (valueChanged (int)),
             this, SLOT (slotHeightChanged (int)));

    connect (m_percentWidthInput, SIGNAL (valueChanged (double)),
             this, SLOT (slotPercentWidthChanged (double)));
    connect (m_percentHeightInput, SIGNAL (valueChanged (double)),
             this, SLOT (slotPercentHeightChanged (double)));

    IGNORE_KEEP_ASPECT_RATIO (slotPercentWidthChanged (m_percentWidthInput->value ()));
    IGNORE_KEEP_ASPECT_RATIO (slotPercentHeightChanged (m_percentHeightInput->value ()));

    connect (m_keepAspectRatioCheckBox, SIGNAL (toggled (bool)),
             this, SLOT (slotKeepAspectRatioToggled (bool)));

    slotKeepAspectRatioToggled (m_keepAspectRatioCheckBox->isChecked ());
}


// private
void kpToolResizeScaleDialog::widthFitHeightToAspectRatio ()
{
    if (m_keepAspectRatioCheckBox->isChecked () && !m_ignoreKeepAspectRatio)
    {
        // width / height = m_oldWidth / m_oldHeight
        // height = width * m_oldHeight / m_oldWidth
        const int newHeight = qRound (double (imageWidth ()) * double (m_oldHeight)
                                      / double (m_oldWidth));
        IGNORE_KEEP_ASPECT_RATIO (m_newHeightInput->setValue (newHeight));
    }
}

// private
void kpToolResizeScaleDialog::heightFitWidthToAspectRatio ()
{
    if (m_keepAspectRatioCheckBox->isChecked () && !m_ignoreKeepAspectRatio)
    {
        // width / height = m_oldWidth / m_oldHeight
        // width = height * m_oldWidth / m_oldHeight
        const int newWidth = qRound (double (imageHeight ()) * double (m_oldWidth)
                                     / double (m_oldHeight));
        IGNORE_KEEP_ASPECT_RATIO (m_newWidthInput->setValue (newWidth));
    }
}


// public slot
void kpToolResizeScaleDialog::slotTypeChanged ()
{
    s_lastType = type ();
}

// public slot
void kpToolResizeScaleDialog::slotWidthChanged (int width)
{
#if DEBUG_KP_TOOL_RESIZE_SCALE_DIALOG && 1
    kdDebug () << "kpToolResizeScaleDialog::slotWidthChanged("
               << width << ")" << endl;
#endif
    const double newPercentWidth = double (width) * 100 / double (m_oldWidth);

    SET_VALUE_WITHOUT_SIGNAL_EMISSION (m_percentWidthInput, newPercentWidth);

    widthFitHeightToAspectRatio ();

    //enableButtonOK (!isNoOp ());
    s_lastPercentWidth = newPercentWidth;
}

// public slot
void kpToolResizeScaleDialog::slotHeightChanged (int height)
{
#if DEBUG_KP_TOOL_RESIZE_SCALE_DIALOG && 1
    kdDebug () << "kpToolResizeScaleDialog::slotHeightChanged("
               << height << ")" << endl;
#endif
    const double newPercentHeight = double (height) * 100 / double (m_oldHeight);

    SET_VALUE_WITHOUT_SIGNAL_EMISSION (m_percentHeightInput, newPercentHeight);

    heightFitWidthToAspectRatio ();

    //enableButtonOK (!isNoOp ());
    s_lastPercentHeight = newPercentHeight;
}

// public slot
void kpToolResizeScaleDialog::slotPercentWidthChanged (double percentWidth)
{
#if DEBUG_KP_TOOL_RESIZE_SCALE_DIALOG && 1
    kdDebug () << "kpToolResizeScaleDialog::slotPercentWidthChanged("
               << percentWidth << ")" << endl;
#endif

    SET_VALUE_WITHOUT_SIGNAL_EMISSION (m_newWidthInput,
                                       qRound (percentWidth * m_oldWidth / 100.0));

    widthFitHeightToAspectRatio ();

    //enableButtonOK (!isNoOp ());
    s_lastPercentWidth = percentWidth;
}

// public slot
void kpToolResizeScaleDialog::slotPercentHeightChanged (double percentHeight)
{
#if DEBUG_KP_TOOL_RESIZE_SCALE_DIALOG && 1
    kdDebug () << "kpToolResizeScaleDialog::slotPercentHeightChanged("
               << percentHeight << ")" << endl;
#endif

    SET_VALUE_WITHOUT_SIGNAL_EMISSION (m_newHeightInput,
                                       qRound (percentHeight * m_oldHeight / 100.0));

    heightFitWidthToAspectRatio ();

    //enableButtonOK (!isNoOp ());
    s_lastPercentHeight = percentHeight;
}

// public slot
void kpToolResizeScaleDialog::slotKeepAspectRatioToggled (bool on)
{
#if DEBUG_KP_TOOL_RESIZE_SCALE_DIALOG && 1
    kdDebug () << "kpToolResizeScaleDialog::slotKeepAspectRatioToggled("
               << on << ")" << endl;
#endif
    if (on)
        widthFitHeightToAspectRatio ();

    s_lastKeepAspectRatio = on;
}

#undef IGNORE_KEEP_ASPECT_RATIO
#undef SET_VALUE_WITHOUT_SIGNAL_EMISSION


// public
int kpToolResizeScaleDialog::imageWidth () const
{
    return m_newWidthInput->value ();
}

// public
int kpToolResizeScaleDialog::imageHeight () const
{
    return m_newHeightInput->value ();
}

// public
kpToolResizeScaleCommand::Type kpToolResizeScaleDialog::type () const
{
    if (m_resizeRadioButton->isChecked ())
        return kpToolResizeScaleCommand::Resize;
    else if (m_scaleRadioButton->isChecked ())
        return kpToolResizeScaleCommand::Scale;
    else
        return kpToolResizeScaleCommand::SmoothScale;
}

// public
bool kpToolResizeScaleDialog::isNoOp () const
{
    return (imageWidth () == m_oldWidth && imageHeight () == m_oldHeight);
}


#include <kptoolresizescale.moc>
