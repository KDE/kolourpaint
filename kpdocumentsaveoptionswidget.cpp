
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

#define DEBUG_KP_DOCUMENT_SAVE_OPTIONS_WIDGET 1


#include <kpdocumentsaveoptionswidget.h>

#include <qbuffer.h>
#include <qimage.h>
#include <qlabel.h>
#include <qlayout.h>

#include <kcombobox.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kimageio.h>
#include <klocale.h>
#include <knuminput.h>
#include <kpdocument.h>
#include <kpushbutton.h>

#include <kppixmapfx.h>
#include <kpresizesignallinglabel.h>


kpDocumentSaveOptionsPreviewDialog::kpDocumentSaveOptionsPreviewDialog (
        QWidget *parent,
        const char *name)
    : KDialogBase (parent, name, false/*non-modal*/,
                   i18n ("Preview Save"),
                   0/*no buttons*/),
      m_filePixmap (0),
      m_fileSize (0)
{
    QWidget *baseWidget = new QWidget (this);
    setMainWidget (baseWidget);


    QGridLayout *lay = new QGridLayout (baseWidget, 2, 1,
                                        marginHint (), spacingHint ());

    m_filePixmapLabel = new kpResizeSignallingLabel (baseWidget);
    m_filePixmapLabel->setMinimumSize (320, 320)
    ;
    m_fileSizeLabel = new QLabel (baseWidget);


    lay->addWidget (m_filePixmapLabel, 0, 0, Qt::AlignCenter);
    lay->addWidget (m_fileSizeLabel, 1, 0, Qt::AlignCenter);


    lay->setRowStretch (0, 1);


    // TODO: doesn't work
    connect (m_filePixmapLabel, SIGNAL (resized ()),
             this, SLOT (updatePixmapPreview ()));
}

kpDocumentSaveOptionsPreviewDialog::~kpDocumentSaveOptionsPreviewDialog ()
{
    delete m_filePixmap;
}


// public slot
void kpDocumentSaveOptionsPreviewDialog::setFilePixmapAndSize (const QPixmap &pixmap,
                                                               int fileSize)
{
    delete m_filePixmap;
    m_filePixmap = new QPixmap (pixmap);
    updatePixmapPreview ();

    m_fileSize = fileSize;

    const int pixmapSize = kpPixmapFX::pixmapSize (pixmap);
    const int percent = pixmapSize ?
                            QMAX (1, QMIN (100, fileSize * 100 / pixmapSize)) :
                            0;
    // HACK: I don't know if the percentage thing will work well and we're
    //       really close to the message freeze so provide alt. texts to choose
    //       from during the message freeze :)
    const QString alternateText0 = i18n ("%1 bytes");
    const QString alternateText1 = i18n ("%1 bytes (%2%)");
    const QString alternateText2 = i18n ("%1 B");
    const QString alternateText3 = i18n ("%1 B (%2%)");
    const QString alternateText4 = i18n ("%1 B (approx. %2%)");
    const QString alternateText5 = i18n ("%1B");
    const QString alternateText6 = i18n ("%1B (%2%)");
    const QString alternateText7 = i18n ("%1B (approx. %2%)");
    m_fileSizeLabel->setText (i18n ("%1 bytes (approx. %2%)")
                                   .arg (KGlobal::locale ()->formatLong (m_fileSize))
                                   .arg (percent));
}

// public slot
void kpDocumentSaveOptionsPreviewDialog::updatePixmapPreview ()
{
#if DEBUG_KP_DOCUMENT_SAVE_OPTIONS_WIDGET
    kdDebug () << "kpDocumentSaveOptionsPreviewDialog::updatePreviewPixmap()"
               << " filePixmapLabel.size=" << m_filePixmapLabel->size ()
               << endl;
#endif

    if (m_filePixmap)
    {
        m_filePixmapLabel->setPixmap (
            kpPixmapFX::scale (*m_filePixmap,
                               m_filePixmapLabel->width (),
                               m_filePixmapLabel->height ()));
    }
    else
    {
        m_filePixmapLabel->setPixmap (QPixmap ());
    }
}



kpDocumentSaveOptionsWidget::kpDocumentSaveOptionsWidget (
        const QPixmap &docPixmap,
        const kpDocumentSaveOptions &saveOptions,
        const kpDocumentMetaInfo &metaInfo,
        QWidget *parent, const char *name)
    : QWidget (parent, name)
{
    init ();
    setDocumentSaveOptions (saveOptions);
    setDocumentPixmap (docPixmap);
    setDocumentMetaInfo (metaInfo);
}

kpDocumentSaveOptionsWidget::kpDocumentSaveOptionsWidget (
        QWidget *parent, const char *name)
    : QWidget (parent, name)
{
    init ();
}

// private
void kpDocumentSaveOptionsWidget::init ()
{
    m_documentPixmap = 0;
    m_previewDialog = 0;


    m_colorDepthLabel = new QLabel (i18n ("Convert &to:"), this);
    m_colorDepthCombo = new KComboBox (this);

    m_colorDepthSpaceWidget = new QWidget (this);

    m_qualityLabel = new QLabel (i18n ("Quali&ty:"), this);
    m_qualityInput = new KIntNumInput (this);
    // Note that we set min to 1 not 0 since "0 Quality" is a bit misleading
    // and 101 quality settings would be weird.  So we lose 1 quality setting
    // according to QImage::save().
    m_qualityInput->setRange (1, 100, 1/*step*/, true/*slider*/);

    m_previewButton = new KPushButton (i18n ("Preview..."), this);
    m_previewButton->setToggleButton (true);


    m_colorDepthLabel->setBuddy (m_colorDepthCombo);

    m_colorDepthCombo->insertItem (i18n ("Monochrome"));
    m_colorDepthCombo->insertItem (i18n ("Monochrome (Dithered)"));
    m_colorDepthCombo->insertItem (i18n ("256 Color"));
    m_colorDepthCombo->insertItem (i18n ("256 Color (Dithered)"));
    m_colorDepthCombo->insertItem (i18n ("24-bit Color"));

    m_qualityLabel->setBuddy (m_qualityInput);


    QHBoxLayout *lay = new QHBoxLayout (this, 0/*margin*/, KDialog::spacingHint ());

    lay->addWidget (m_colorDepthLabel, 0/*stretch*/, Qt::AlignLeft);
    lay->addWidget (m_colorDepthCombo, 0/*stretch*/);

    lay->addWidget (m_colorDepthSpaceWidget, 1/*stretch*/);

    lay->addWidget (m_qualityLabel, 0/*stretch*/, Qt::AlignLeft);
    lay->addWidget (m_qualityInput, 2/*stretch*/);

    lay->addWidget (m_previewButton, 0/*stretch*/, Qt::AlignRight);


    connect (m_colorDepthCombo, SIGNAL (activated (int)),
             this, SLOT (updatePreview ()));
    connect (m_qualityInput, SIGNAL (valueChanged (int)),
             this, SLOT (updatePreview ()));

    connect (m_previewButton, SIGNAL (toggled (bool)),
             this, SLOT (showPreview (bool)));


    setMode (None);
}

kpDocumentSaveOptionsWidget::~kpDocumentSaveOptionsWidget ()
{
    kdDebug () << "kpDocumentSaveOptionsWidget::<dtor>()" << endl;
    delete m_documentPixmap;
    delete m_previewDialog;  // just in case
}


// protected
bool kpDocumentSaveOptionsWidget::mimeTypeSupportsColorDepth () const
{
    return kpDocumentSaveOptions::mimeTypeSupportsColorDepth (mimeType ());
}

// protected
bool kpDocumentSaveOptionsWidget::mimeTypeSupportsQuality () const
{
    return kpDocumentSaveOptions::mimeTypeSupportsQuality (mimeType ());
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
    kdDebug () << "kpDocumentSaveOptionsWidget::setMimeType(" << string << ")" << endl;
#endif

    if (string == mimeType ())
        return;

    m_baseDocumentSaveOptions.setMimeType (string);

    if (mimeTypeSupportsColorDepth ())
        setMode (ColorDepth);
    else if (mimeTypeSupportsQuality ())
        setMode (Quality);
    else
        setMode (None);

    updatePreview ();
}


// public
int kpDocumentSaveOptionsWidget::colorDepth () const
{
    if (mode () & ColorDepth)
    {
        switch (m_colorDepthCombo->currentItem ())
        {
        case 0:
        case 1:
            return 1;

        case 2:
        case 3:
            return 8;

        case 4:
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
        return (m_colorDepthCombo->currentItem () == 1 ||
                m_colorDepthCombo->currentItem () == 3);
    }
    else
    {
        return m_baseDocumentSaveOptions.dither ();
    }
}

// public slots
void kpDocumentSaveOptionsWidget::setColorDepthDither (int newDepth, bool newDither)
{
#if DEBUG_KP_DOCUMENT_SAVE_OPTIONS_WIDGET
    kdDebug () << "kpDocumentSaveOptionsWidget::setColorDepthDither("
               << "depth=" << newDepth
               << ",dither=" << newDither
               << ")" << endl;
#endif

    m_baseDocumentSaveOptions.setColorDepth (newDepth);
    m_baseDocumentSaveOptions.setDither (newDither);

    if (newDepth == 1)
    {
        if (!newDither)
            m_colorDepthCombo->setCurrentItem (0);
        else
            m_colorDepthCombo->setCurrentItem (1);
    }
    else if (newDepth == 8)
    {
        if (!newDither)
            m_colorDepthCombo->setCurrentItem (2);
        else
            m_colorDepthCombo->setCurrentItem (3);
    }
    else if (newDepth == 32)
    {
        m_colorDepthCombo->setCurrentItem (4);
    }
}


// public
int kpDocumentSaveOptionsWidget::quality () const
{
    if (mode () & Quality)
    {
        return m_qualityInput->value ();
    }
    else
    {
        return m_baseDocumentSaveOptions.quality ();
    }
}

// public
void kpDocumentSaveOptionsWidget::setQuality (int newQuality)
{
#if DEBUG_KP_DOCUMENT_SAVE_OPTIONS_WIDGET
    kdDebug () << "kpDocumentSaveOptionsWidget::setQuality("
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
void kpDocumentSaveOptionsWidget::setDocumentPixmap (const QPixmap &documentPixmap)
{
    delete m_documentPixmap;
    m_documentPixmap = new QPixmap (documentPixmap);

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
    m_colorDepthLabel->setShown (mode != Quality);
    m_colorDepthCombo->setShown (mode != Quality);
    m_colorDepthSpaceWidget->setShown (mode != Quality);

    m_qualityLabel->setShown (mode == Quality);
    m_qualityInput->setShown (mode == Quality);


    m_colorDepthLabel->setEnabled (mode == ColorDepth);
    m_colorDepthCombo->setEnabled (mode == ColorDepth);

    m_qualityLabel->setEnabled (mode == Quality);
    m_qualityInput->setEnabled (mode == Quality);
}


// protected slot
void kpDocumentSaveOptionsWidget::showPreview (bool yes)
{
#if DEBUG_KP_DOCUMENT_SAVE_OPTIONS_WIDGET
    kdDebug () << "kpDocumentSaveOptionsWidget::showPreview(" << yes << ")"
               << " m_previewDialog=" << bool (m_previewDialog)
               << endl;
#endif

    if (yes == bool (m_previewDialog))
        return;

    if (yes)
    {
        m_previewDialog = new kpDocumentSaveOptionsPreviewDialog (dynamic_cast <QWidget *> (parent ()));
        updatePreview ();

        connect (m_previewDialog, SIGNAL (finished ()),
                 this, SLOT (hidePreview ()));
        // TODO: position is braindead
        m_previewDialog->show ();
    }
    else
    {
        m_previewDialog->deleteLater ();
        m_previewDialog = 0;
    }
}

// protected slot
void kpDocumentSaveOptionsWidget::hidePreview ()
{
    if (m_previewButton->isOn ())
        m_previewButton->toggle ();
}

// protected slot
void kpDocumentSaveOptionsWidget::updatePreview ()
{
    if (!m_previewDialog || !m_documentPixmap)
        return;


    QByteArray data;

    QBuffer buffer (data);
    buffer.open (IO_WriteOnly);
    kpDocument::savePixmapToDevice (*m_documentPixmap,
                                    &buffer,
                                    documentSaveOptions (),
                                    m_documentMetaInfo,
                                    false/*no lossy prompt*/,
                                    this);
    buffer.close ();


    QImage image;
    image.loadFromData (data,
        KImageIO::typeForMime (mimeType ()).latin1 ());

    // TODO: merge with kpDocument::getPixmapFromFile()
    m_previewDialog->setFilePixmapAndSize (
        kpPixmapFX::convertToPixmap (image, false/*no dither*/),
        data.size ());
}


#include <kpdocumentsaveoptionswidget.moc>
