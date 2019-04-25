
/*
   Copyright (c) 2003-2007 Clarence Dang <dang@kde.org>
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

#define DEBUG_KP_DOCUMENT_SAVE_OPTIONS_WIDGET 0


#include "widgets/kpDocumentSaveOptionsWidget.h"

#include "kpDefs.h"
#include "document/kpDocument.h"
#include "dialogs/kpDocumentSaveOptionsPreviewDialog.h"
#include "pixmapfx/kpPixmapFX.h"
#include "generic/widgets/kpResizeSignallingLabel.h"
#include "dialogs/imagelib/transforms/kpTransformPreviewDialog.h"
#include "generic/kpWidgetMapper.h"

#include "kpLogCategories.h"
#include <KSharedConfig>
#include <KConfigGroup>

#include <QApplication>
#include <QBoxLayout>
#include <QBuffer>
#include <QComboBox>
#include <QEvent>
#include <QGridLayout>
#include <QImage>
#include <QLabel>
#include <QLayout>
#include <QPushButton>
#include <QSpinBox>
#include <QTimer>


kpDocumentSaveOptionsWidget::kpDocumentSaveOptionsWidget (
        const QImage &docPixmap,
        const kpDocumentSaveOptions &saveOptions,
        const kpDocumentMetaInfo &metaInfo,
        QWidget *parent)
    : QWidget (parent),
      m_visualParent (parent)
{
    init ();
    setDocumentSaveOptions (saveOptions);
    setDocumentPixmap (docPixmap);
    setDocumentMetaInfo (metaInfo);
}

kpDocumentSaveOptionsWidget::kpDocumentSaveOptionsWidget (
        QWidget *parent)
    : QWidget (parent),
      m_visualParent (parent)
{
    init ();
}

// private
void kpDocumentSaveOptionsWidget::init ()
{
    m_documentPixmap = nullptr;
    m_previewDialog = nullptr;
    m_visualParent = nullptr;


    m_colorDepthLabel = new QLabel (i18n ("Convert &to:"), this);
    m_colorDepthCombo = new QComboBox (this);

    m_colorDepthSpaceWidget = new QWidget (this);

    m_qualityLabel = new QLabel(i18n ("Quali&ty:"), this);
    m_qualityInput = new QSpinBox(this);
    // Note that we set min to 1 not 0 since "0 Quality" is a bit misleading
    // and 101 quality settings would be weird.  So we lose 1 quality setting
    // according to QImage::save().
    // TODO: 100 quality is also misleading since that implies perfect quality.
    m_qualityInput->setRange (1, 100);

    m_previewButton = new QPushButton (i18n ("&Preview"), this);
    m_previewButton->setCheckable (true);


    m_colorDepthLabel->setBuddy (m_colorDepthCombo);

    m_qualityLabel->setBuddy (m_qualityInput);


    auto *lay = new QHBoxLayout (this);
    lay->setContentsMargins(0, 0, 0, 0);

    lay->addWidget (m_colorDepthLabel, 0/*stretch*/, Qt::AlignLeft);
    lay->addWidget (m_colorDepthCombo, 0/*stretch*/);

    lay->addWidget (m_colorDepthSpaceWidget, 1/*stretch*/);

    lay->addWidget (m_qualityLabel, 0/*stretch*/, Qt::AlignLeft);
    lay->addWidget (m_qualityInput, 2/*stretch*/);

    lay->addWidget (m_previewButton, 0/*stretch*/, Qt::AlignRight);


    connect (m_colorDepthCombo,
             static_cast<void (QComboBox::*)(int)>(&QComboBox::activated),
             this, &kpDocumentSaveOptionsWidget::slotColorDepthSelected);

    connect (m_colorDepthCombo,
             static_cast<void (QComboBox::*)(int)>(&QComboBox::activated),
             this, &kpDocumentSaveOptionsWidget::updatePreview);

    connect (m_qualityInput,
             static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
             this, &kpDocumentSaveOptionsWidget::updatePreviewDelayed);

    connect (m_previewButton, &QPushButton::toggled,
             this, &kpDocumentSaveOptionsWidget::showPreview);


    m_updatePreviewDelay = 200/*ms*/;

    m_updatePreviewTimer = new QTimer (this);
    m_updatePreviewTimer->setSingleShot (true);
    connect (m_updatePreviewTimer, &QTimer::timeout,
             this, &kpDocumentSaveOptionsWidget::updatePreview);

    m_updatePreviewDialogLastRelativeGeometryTimer = new QTimer (this);
    connect (m_updatePreviewDialogLastRelativeGeometryTimer,
             &QTimer::timeout,
             this, &kpDocumentSaveOptionsWidget::updatePreviewDialogLastRelativeGeometry);

    setMode (None);

    slotColorDepthSelected ();
}

kpDocumentSaveOptionsWidget::~kpDocumentSaveOptionsWidget ()
{
#if DEBUG_KP_DOCUMENT_SAVE_OPTIONS_WIDGET
    qCDebug(kpLogWidgets) << "kpDocumentSaveOptionsWidget::<dtor>()";
#endif
    hidePreview ();

    delete m_documentPixmap;
}


// public
void kpDocumentSaveOptionsWidget::setVisualParent (QWidget *visualParent)
{
#if DEBUG_KP_DOCUMENT_SAVE_OPTIONS_WIDGET
    qCDebug(kpLogWidgets) << "kpDocumentSaveOptionsWidget::setVisualParent("
               << visualParent << ")" << endl;
#endif

    m_visualParent = visualParent;
}


// protected
bool kpDocumentSaveOptionsWidget::mimeTypeHasConfigurableColorDepth () const
{
    return kpDocumentSaveOptions::mimeTypeHasConfigurableColorDepth (mimeType ());
}

// protected
bool kpDocumentSaveOptionsWidget::mimeTypeHasConfigurableQuality () const
{
    return kpDocumentSaveOptions::mimeTypeHasConfigurableQuality (mimeType ());
}


// public
QString kpDocumentSaveOptionsWidget::mimeType () const
{
    return m_baseDocumentSaveOptions.mimeType ();
}

// public slots
void kpDocumentSaveOptionsWidget::setMimeType (const QString &string)
{
#if DEBUG_KP_DOCUMENT_SAVE_OPTIONS_WIDGET
    qCDebug(kpLogWidgets) << "kpDocumentSaveOptionsWidget::setMimeType(" << string
               << ") maxColorDepth="
               << kpDocumentSaveOptions::mimeTypeMaximumColorDepth (string)
               << endl;
#endif

    const int newMimeTypeMaxDepth =
        kpDocumentSaveOptions::mimeTypeMaximumColorDepth (string);

#if DEBUG_KP_DOCUMENT_SAVE_OPTIONS_WIDGET
    qCDebug(kpLogWidgets) << "\toldMimeType=" << mimeType ()
               << " maxColorDepth="
               << kpDocumentSaveOptions::mimeTypeMaximumColorDepth (
                      mimeType ())
               << endl;
#endif

    if (mimeType ().isEmpty () ||
        kpDocumentSaveOptions::mimeTypeMaximumColorDepth (mimeType ()) !=
        newMimeTypeMaxDepth)
    {
        m_colorDepthCombo->clear ();

        m_colorDepthCombo->insertItem (0, i18n ("Monochrome"));
        m_colorDepthCombo->insertItem (1, i18n ("Monochrome (Dithered)"));

        if (newMimeTypeMaxDepth >= 8)
        {
            m_colorDepthCombo->insertItem (2, i18n ("256 Color"));
            m_colorDepthCombo->insertItem (3, i18n ("256 Color (Dithered)"));
        }

        if (newMimeTypeMaxDepth >= 24)
        {
            m_colorDepthCombo->insertItem (4, i18n ("24-bit Color"));
        }

        if (m_colorDepthComboLastSelectedItem >= 0 &&
            m_colorDepthComboLastSelectedItem < m_colorDepthCombo->count ())
        {
        #if DEBUG_KP_DOCUMENT_SAVE_OPTIONS_WIDGET
            qCDebug(kpLogWidgets) << "\tsetting colorDepthCombo to "
                       << m_colorDepthComboLastSelectedItem << endl;
        #endif

            m_colorDepthCombo->setCurrentIndex (m_colorDepthComboLastSelectedItem);
        }
        else
        {
        #if DEBUG_KP_DOCUMENT_SAVE_OPTIONS_WIDGET
            qCDebug(kpLogWidgets) << "\tsetting colorDepthCombo to max item since"
                       << " m_colorDepthComboLastSelectedItem="
                       << m_colorDepthComboLastSelectedItem
                       << " out of range" << endl;
        #endif

            m_colorDepthCombo->setCurrentIndex (m_colorDepthCombo->count () - 1);
        }
    }


    m_baseDocumentSaveOptions.setMimeType (string);

    if (mimeTypeHasConfigurableColorDepth ()) {
        setMode (ColorDepth);
    }
    else if (mimeTypeHasConfigurableQuality ()) {
        setMode (Quality);
    }
    else {
        setMode (None);
    }

    updatePreview ();
}


// public
int kpDocumentSaveOptionsWidget::colorDepth () const
{
    if (mode () & ColorDepth)
    {
        // The returned values match QImage's supported depths.
        switch (m_colorDepthCombo->currentIndex ())
        {
        case 0:
        case 1:
            return 1;

        case 2:
        case 3:
            return 8;

        case 4:
            // 24-bit is known as 32-bit with QImage.
            return 32;

        default:
            return kpDocumentSaveOptions::invalidColorDepth ();
        }
    }
    else
    {
        return m_baseDocumentSaveOptions.colorDepth ();
    }
}

// public
bool kpDocumentSaveOptionsWidget::dither () const
{
    if (mode () & ColorDepth)
    {
        return (m_colorDepthCombo->currentIndex () == 1 ||
                m_colorDepthCombo->currentIndex () == 3);
    }

    return m_baseDocumentSaveOptions.dither ();
}

// protected static
int kpDocumentSaveOptionsWidget::colorDepthComboItemFromColorDepthAndDither (
    int depth, bool dither)
{
    switch (depth) {
    case 1:
        if (!dither) {
            return 0;
        }
        return 1;

    case 8:
        if (!dither) {
            return 2;
        }
        return 3;

    case 32:
        return 4;

    default:
        return -1;
    }
}

// public slots
void kpDocumentSaveOptionsWidget::setColorDepthDither (int newDepth, bool newDither)
{
#if DEBUG_KP_DOCUMENT_SAVE_OPTIONS_WIDGET
    qCDebug(kpLogWidgets) << "kpDocumentSaveOptionsWidget::setColorDepthDither("
               << "depth=" << newDepth
               << ",dither=" << newDither
               << ")" << endl;
#endif

    m_baseDocumentSaveOptions.setColorDepth (newDepth);
    m_baseDocumentSaveOptions.setDither (newDither);


    const int comboItem = colorDepthComboItemFromColorDepthAndDither (
                              newDepth, newDither);
    // TODO: Ignoring when comboItem >= m_colorDepthCombo->count() is wrong.
    //       This happens if this mimeType has configurable colour depth
    //       and an incorrect maximum colour depth (less than a QImage of
    //       this mimeType, opened by kpDocument).
    if (comboItem >= 0 && comboItem < m_colorDepthCombo->count ()) {
        m_colorDepthCombo->setCurrentIndex (comboItem);
    }


    slotColorDepthSelected ();
}


// protected slot
void kpDocumentSaveOptionsWidget::slotColorDepthSelected ()
{
    if (mode () & ColorDepth)
    {
        m_colorDepthComboLastSelectedItem = m_colorDepthCombo->currentIndex ();
    }
    else
    {
        m_colorDepthComboLastSelectedItem =
            colorDepthComboItemFromColorDepthAndDither (
                m_baseDocumentSaveOptions.colorDepth (),
                m_baseDocumentSaveOptions.dither ());
    }

#if DEBUG_KP_DOCUMENT_SAVE_OPTIONS_WIDGET
    qCDebug(kpLogWidgets) << "kpDocumentSaveOptionsWidget::slotColorDepthSelected()"
               << " mode&ColorDepth=" << (mode () & ColorDepth)
               << " colorDepthComboLastSelectedItem="
               << m_colorDepthComboLastSelectedItem
               << endl;
#endif
}


// public
int kpDocumentSaveOptionsWidget::quality () const
{
    if (mode () & Quality)
    {
        return m_qualityInput->value ();
    }

    return m_baseDocumentSaveOptions.quality ();
}

// public
void kpDocumentSaveOptionsWidget::setQuality (int newQuality)
{
#if DEBUG_KP_DOCUMENT_SAVE_OPTIONS_WIDGET
    qCDebug(kpLogWidgets) << "kpDocumentSaveOptionsWidget::setQuality("
               << newQuality << ")" << endl;
#endif

    m_baseDocumentSaveOptions.setQuality (newQuality);
    m_qualityInput->setValue (newQuality == -1/*QImage::save() default*/ ?
                                  75 :
                                  newQuality);
}


// public
kpDocumentSaveOptions kpDocumentSaveOptionsWidget::documentSaveOptions () const
{
    return kpDocumentSaveOptions (mimeType (), colorDepth (), dither (), quality ());
}

// public
void kpDocumentSaveOptionsWidget::setDocumentSaveOptions (
    const kpDocumentSaveOptions &saveOptions)
{
    setMimeType (saveOptions.mimeType ());
    setColorDepthDither (saveOptions.colorDepth (), saveOptions.dither ());
    setQuality (saveOptions.quality ());
}


// public
void kpDocumentSaveOptionsWidget::setDocumentPixmap (const QImage &documentPixmap)
{
    delete m_documentPixmap;
    m_documentPixmap = new QImage (documentPixmap);

    updatePreview ();
}

// public
void kpDocumentSaveOptionsWidget::setDocumentMetaInfo (
    const kpDocumentMetaInfo &metaInfo)
{
    m_documentMetaInfo = metaInfo;

    updatePreview ();
}


// public
kpDocumentSaveOptionsWidget::Mode kpDocumentSaveOptionsWidget::mode () const
{
    return m_mode;
}

// public
void kpDocumentSaveOptionsWidget::setMode (Mode mode)
{
    m_mode = mode;


    // If mode == None, we show still show the Color Depth widgets but disabled
    m_colorDepthLabel->setVisible (mode != Quality);
    m_colorDepthCombo->setVisible (mode != Quality);
    m_colorDepthSpaceWidget->setVisible (mode != Quality);

    m_qualityLabel->setVisible (mode == Quality);
    m_qualityInput->setVisible (mode == Quality);


    m_colorDepthLabel->setEnabled (mode == ColorDepth);
    m_colorDepthCombo->setEnabled (mode == ColorDepth);

    m_qualityLabel->setEnabled (mode == Quality);
    m_qualityInput->setEnabled (mode == Quality);


    // SYNC: HACK: When changing between color depth and quality widgets,
    //       we change the height of "this", causing the text on the labels
    //       to move but the first instance of the text doesn't get erased.
    //       Qt bug.
    QTimer::singleShot (0, this, &kpDocumentSaveOptionsWidget::repaintLabels);
}

// protected slot
void kpDocumentSaveOptionsWidget::repaintLabels ()
{
    if (mode () != Quality) {
        m_colorDepthLabel->repaint ();
    }
    if (mode () == Quality) {
        m_qualityLabel->repaint ();
    }
}


// protected slot
void kpDocumentSaveOptionsWidget::showPreview (bool yes)
{
#if DEBUG_KP_DOCUMENT_SAVE_OPTIONS_WIDGET
    qCDebug(kpLogWidgets) << "kpDocumentSaveOptionsWidget::showPreview(" << yes << ")"
               << " m_previewDialog=" << bool (m_previewDialog)
               << endl;
#endif

    if (yes == bool (m_previewDialog)) {
        return;
    }

    if (!m_visualParent) {
        return;
    }

    if (yes)
    {
        m_previewDialog = new kpDocumentSaveOptionsPreviewDialog( m_visualParent );
        m_previewDialog->setObjectName( QStringLiteral( "previewSaveDialog" ) );
        updatePreview ();

        connect (m_previewDialog, &kpDocumentSaveOptionsPreviewDialog::finished,
                 this, &kpDocumentSaveOptionsWidget::hidePreview);


        KConfigGroup cfg (KSharedConfig::openConfig (), kpSettingsGroupPreviewSave);

        if (cfg.hasKey (kpSettingPreviewSaveUpdateDelay))
        {
            m_updatePreviewDelay = cfg.readEntry (kpSettingPreviewSaveUpdateDelay, 0);
        }
        else
        {
            cfg.writeEntry (kpSettingPreviewSaveUpdateDelay, m_updatePreviewDelay);
            cfg.sync ();
        }

        if (m_updatePreviewDelay < 0) {
            m_updatePreviewDelay = 0;
        }
    #if DEBUG_KP_DOCUMENT_SAVE_OPTIONS_WIDGET
        qCDebug(kpLogWidgets) << "\tread cfg preview dialog update delay=" << m_updatePreviewDelay;
    #endif


        if (m_previewDialogLastRelativeGeometry.isEmpty ())
        {
        #if DEBUG_KP_DOCUMENT_SAVE_OPTIONS_WIDGET
            qCDebug(kpLogWidgets) << "\tread cfg preview dialog last rel geometry";
        #endif
            KConfigGroup cfg (KSharedConfig::openConfig (), kpSettingsGroupPreviewSave);

            m_previewDialogLastRelativeGeometry = cfg.readEntry (
                kpSettingPreviewSaveGeometry, QRect ());
        }

    #if DEBUG_KP_DOCUMENT_SAVE_OPTIONS_WIDGET
        qCDebug(kpLogWidgets) << "\tpreviewDialogLastRelativeGeometry="
                   << m_previewDialogLastRelativeGeometry
                   << " visualParent->rect()=" << m_visualParent->rect ()
                   << endl;
    #endif

        QRect relativeGeometry;
        if (!m_previewDialogLastRelativeGeometry.isEmpty () &&
            m_visualParent->rect ().intersects (m_previewDialogLastRelativeGeometry))
        {
        #if DEBUG_KP_DOCUMENT_SAVE_OPTIONS_WIDGET
            qCDebug(kpLogWidgets) << "\tok";
        #endif
            relativeGeometry = m_previewDialogLastRelativeGeometry;
        }
        else
        {
        #if DEBUG_KP_DOCUMENT_SAVE_OPTIONS_WIDGET
            qCDebug(kpLogWidgets) << "\t\tinvalid";
        #endif
            const int margin = 20;

            relativeGeometry =
                QRect (m_visualParent->width () -
                           m_previewDialog->preferredMinimumSize ().width () -
                               margin,
                       margin * 2,  // Avoid folder combo
                       m_previewDialog->preferredMinimumSize ().width (),
                       m_previewDialog->preferredMinimumSize ().height ());
        }


        const QRect globalGeometry =
            kpWidgetMapper::toGlobal (m_visualParent,
                                      relativeGeometry);
    #if DEBUG_KP_DOCUMENT_SAVE_OPTIONS_WIDGET
        qCDebug(kpLogWidgets) << "\trelativeGeometry=" << relativeGeometry
                   << " globalGeometry=" << globalGeometry
                   << endl;
    #endif

        m_previewDialog->resize (globalGeometry.size ());
        m_previewDialog->move (globalGeometry.topLeft ());


        m_previewDialog->show ();


    #if DEBUG_KP_DOCUMENT_SAVE_OPTIONS_WIDGET
        qCDebug(kpLogWidgets) << "\tgeometry after show="
                   << QRect (m_previewDialog->x (), m_previewDialog->y (),
                              m_previewDialog->width (), m_previewDialog->height ())
                   << endl;
    #endif

        updatePreviewDialogLastRelativeGeometry ();

        connect (m_previewDialog, &kpDocumentSaveOptionsPreviewDialog::moved,
                 this, &kpDocumentSaveOptionsWidget::updatePreviewDialogLastRelativeGeometry);

        connect (m_previewDialog, &kpDocumentSaveOptionsPreviewDialog::resized,
                 this, &kpDocumentSaveOptionsWidget::updatePreviewDialogLastRelativeGeometry);

        m_updatePreviewDialogLastRelativeGeometryTimer->start (200/*ms*/);
    }
    else
    {
        m_updatePreviewDialogLastRelativeGeometryTimer->stop ();

        KConfigGroup cfg (KSharedConfig::openConfig (), kpSettingsGroupPreviewSave);

        cfg.writeEntry (kpSettingPreviewSaveGeometry, m_previewDialogLastRelativeGeometry);
        cfg.sync ();

    #if DEBUG_KP_DOCUMENT_SAVE_OPTIONS_WIDGET
        qCDebug(kpLogWidgets) << "\tsaving preview geometry "
                   << m_previewDialogLastRelativeGeometry
                   << " (Qt would have us believe "
                   << kpWidgetMapper::fromGlobal (m_visualParent,
                          QRect (m_previewDialog->x (), m_previewDialog->y (),
                                 m_previewDialog->width (), m_previewDialog->height ()))
                   << ")"
                   << endl;
    #endif

        m_previewDialog->deleteLater ();
        m_previewDialog = nullptr;
    }
}

// protected slot
void kpDocumentSaveOptionsWidget::hidePreview ()
{
    if (m_previewButton->isChecked ()) {
        m_previewButton->toggle ();
    }
}


// protected slot
void kpDocumentSaveOptionsWidget::updatePreviewDelayed ()
{
    // (single shot)
    m_updatePreviewTimer->start (m_updatePreviewDelay);
}

// protected slot
void kpDocumentSaveOptionsWidget::updatePreview ()
{
    if (!m_previewDialog || !m_documentPixmap) {
        return;
    }


    m_updatePreviewTimer->stop ();


    QApplication::setOverrideCursor (Qt::WaitCursor);

    QByteArray data;

    QBuffer buffer (&data);
    buffer.open (QIODevice::WriteOnly);
    bool savedOK = kpDocument::savePixmapToDevice (*m_documentPixmap,
                                    &buffer,
                                    documentSaveOptions (),
                                    m_documentMetaInfo,
                                    false/*no lossy prompt*/,
                                    this);
    buffer.close ();


    QImage image;

    // Ignore any failed saves.
    //
    // Failed saves might literally have written half a file.  The final
    // save (when the user clicks OK), _will_ fail so we shouldn't have a
    // preview even if this "half a file" is actually loadable by
    // QImage::loadFormData().
    if (savedOK)
    {
        image.loadFromData(data);
    }
    else
    {
        // Leave <image> as invalid.
        // TODO: This code path has not been well tested.
        //       Will we trigger divide by zero errors in "m_previewDialog"?
    }

    // REFACTOR: merge with kpDocument::getPixmapFromFile()
    m_previewDialog->setFilePixmapAndSize (image, data.size ());

    QApplication::restoreOverrideCursor ();
}

// protected slot
void kpDocumentSaveOptionsWidget::updatePreviewDialogLastRelativeGeometry ()
{
#if DEBUG_KP_DOCUMENT_SAVE_OPTIONS_WIDGET
    qCDebug(kpLogWidgets) << "kpDocumentSaveOptionsWidget::"
               << "updatePreviewDialogLastRelativeGeometry()"
               << endl;
#endif

    if (m_previewDialog && m_previewDialog->isVisible ())
    {
        m_previewDialogLastRelativeGeometry =
            kpWidgetMapper::fromGlobal (m_visualParent,
                QRect (m_previewDialog->x (), m_previewDialog->y (),
                       m_previewDialog->width (), m_previewDialog->height ()));
    #if DEBUG_KP_DOCUMENT_SAVE_OPTIONS_WIDGET
        qCDebug(kpLogWidgets) << "\tcaching pos = "
                   << m_previewDialogLastRelativeGeometry;
    #endif
    }
    else
    {
    #if DEBUG_KP_DOCUMENT_SAVE_OPTIONS_WIDGET
        qCDebug(kpLogWidgets) << "\tnot visible - ignoring geometry";
    #endif
    }
}


