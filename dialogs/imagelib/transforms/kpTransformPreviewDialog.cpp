
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


#define DEBUG_KP_TRANSFORM_PREVIEW_DIALOG 0


#include "dialogs/imagelib/transforms/kpTransformPreviewDialog.h"

#include <QApplication>
#include <QBoxLayout>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLayout>
#include <QPushButton>

#include "kpLogCategories.h"
#include <KLocalizedString>

#include "layers/selections/image/kpAbstractImageSelection.h"
#include "imagelib/kpColor.h"
#include "document/kpDocument.h"
#include "pixmapfx/kpPixmapFX.h"
#include "generic/widgets/kpResizeSignallingLabel.h"
#include "environments/dialogs/imagelib/transforms/kpTransformDialogEnvironment.h"


kpTransformPreviewDialog::kpTransformPreviewDialog (Features features,
        bool reserveTopRow,
        const QString &caption,
        const QString &afterActionText,
        bool actOnSelection,
        kpTransformDialogEnvironment *_env,
        QWidget *parent)
    : QDialog (parent),
      m_afterActionText (afterActionText),
      m_actOnSelection (actOnSelection),
      m_dimensionsGroupBox (nullptr),
      m_afterTransformDimensionsLabel (nullptr),
      m_previewGroupBox (nullptr),
      m_previewPixmapLabel (nullptr),
      m_gridLayout (nullptr),
      m_environ (_env)
{
    setWindowTitle (caption);
    QDialogButtonBox *buttons = new QDialogButtonBox (QDialogButtonBox::Ok |
                                                      QDialogButtonBox::Cancel, this);
    connect (buttons, &QDialogButtonBox::accepted, this, &kpTransformPreviewDialog::accept);
    connect (buttons, &QDialogButtonBox::rejected, this, &kpTransformPreviewDialog::reject);

    QWidget *baseWidget = new QWidget (this);
    m_mainWidget = baseWidget;

    auto *dialogLayout = new QVBoxLayout (this);
    dialogLayout->addWidget (baseWidget);
    dialogLayout->addWidget (buttons);


    if (document ())
    {
        m_oldWidth = document ()->width (actOnSelection);
        m_oldHeight = document ()->height (actOnSelection);
    }
    else
    {
        m_oldWidth = m_oldHeight = 1;
    }


    if (features & Dimensions) {
        createDimensionsGroupBox ();
    }

    if (features & Preview) {
        createPreviewGroupBox ();
    }


    m_gridLayout = new QGridLayout (baseWidget );
    m_gridLayout->setContentsMargins(0, 0, 0, 0);
    m_gridNumRows = reserveTopRow ? 1 : 0;
    if (m_dimensionsGroupBox || m_previewGroupBox)
    {
        if (m_dimensionsGroupBox && m_previewGroupBox)
        {
            m_gridLayout->addWidget (m_dimensionsGroupBox, m_gridNumRows, 0);
            m_gridLayout->addWidget (m_previewGroupBox, m_gridNumRows, 1);

            m_gridLayout->setColumnStretch (1, 1);
        }
        else if (m_dimensionsGroupBox)
        {
            m_gridLayout->addWidget (m_dimensionsGroupBox,
                                     m_gridNumRows, 0, 1, 2);
        }
        else if (m_previewGroupBox)
        {
            m_gridLayout->addWidget (m_previewGroupBox,
                                     m_gridNumRows, 0, 1, 2);
        }

        m_gridLayout->setRowStretch (m_gridNumRows, 1);
        m_gridNumRows++;
    }
}

kpTransformPreviewDialog::~kpTransformPreviewDialog () = default;


// private
void kpTransformPreviewDialog::createDimensionsGroupBox ()
{
    m_dimensionsGroupBox = new QGroupBox (i18n ("Dimensions"), mainWidget ());

    auto *originalLabel = new QLabel (i18n ("Original:"), m_dimensionsGroupBox);
    QString originalDimensions;
    if (document ())
    {
         originalDimensions = i18n ("%1 x %2",
                                    m_oldWidth,
                                    m_oldHeight);

         // Stop the Dimensions Group Box from resizing so often
         const QString minimumLengthString (QStringLiteral("100000 x 100000"));
         const int padLength = minimumLengthString.length ();
         for (int i = originalDimensions.length (); i < padLength; i++) {
             originalDimensions += ' ';
         }
    }
    auto *originalDimensionsLabel = new QLabel (originalDimensions, m_dimensionsGroupBox);

    auto *afterTransformLabel = new QLabel (m_afterActionText, m_dimensionsGroupBox);
    m_afterTransformDimensionsLabel = new QLabel (m_dimensionsGroupBox);


    auto *dimensionsLayout = new QGridLayout (m_dimensionsGroupBox );
    dimensionsLayout->addWidget (originalLabel, 0, 0, Qt::AlignBottom);
    dimensionsLayout->addWidget (originalDimensionsLabel, 0, 1, Qt::AlignBottom);
    dimensionsLayout->addWidget (afterTransformLabel, 1, 0, Qt::AlignTop);
    dimensionsLayout->addWidget (m_afterTransformDimensionsLabel, 1, 1, Qt::AlignTop);
}

// private
void kpTransformPreviewDialog::createPreviewGroupBox ()
{
    m_previewGroupBox = new QGroupBox (i18n ("Preview"), mainWidget ());

    m_previewPixmapLabel = new kpResizeSignallingLabel (m_previewGroupBox);
    m_previewPixmapLabel->setMinimumSize (150, 110);
    connect (m_previewPixmapLabel, &kpResizeSignallingLabel::resized,
             this, &kpTransformPreviewDialog::updatePreview);

    QPushButton *updatePushButton = new QPushButton (i18n ("&Update"),
                                                     m_previewGroupBox);
    connect (updatePushButton, &QPushButton::clicked,
             this, &kpTransformPreviewDialog::slotUpdateWithWaitCursor);


    auto *previewLayout = new QVBoxLayout (m_previewGroupBox);
    previewLayout->addWidget (m_previewPixmapLabel, 1/*stretch*/);
    previewLayout->addWidget (updatePushButton, 0/*stretch*/, Qt::AlignHCenter);
}


// protected
kpDocument *kpTransformPreviewDialog::document () const
{
    return m_environ->document ();
}

// protected
QWidget *kpTransformPreviewDialog::mainWidget () const
{
    return m_mainWidget;
}

// protected
void kpTransformPreviewDialog::addCustomWidgetToFront (QWidget *w)
{
    m_gridLayout->addWidget (w, 0, 0, 1, 2);
}

// protected
void kpTransformPreviewDialog::addCustomWidget (QWidget *w)
{
    m_gridLayout->addWidget (w, m_gridNumRows, 0, 1, 2);
    m_gridNumRows++;
}


// public override [base QWidget]
void kpTransformPreviewDialog::setUpdatesEnabled (bool enable)
{
    QDialog::setUpdatesEnabled (enable);

    if (enable) {
        slotUpdateWithWaitCursor ();
    }
}


// private
void kpTransformPreviewDialog::updateDimensions ()
{
    if (!m_dimensionsGroupBox) {
        return;
    }

    kpDocument *doc = document ();
    if (!doc) {
        return;
    }

    if (!updatesEnabled ())
    {
    #if DEBUG_KP_TRANSFORM_PREVIEW_DIALOG
        qCDebug(kpLogDialogs) << "updates not enabled - aborting";
    #endif
        return;
    }

    QSize newDim = newDimensions ();
#if DEBUG_KP_TRANSFORM_PREVIEW_DIALOG
    qCDebug(kpLogDialogs) << "kpTransformPreviewDialog::updateDimensions(): newDim=" << newDim;
#endif

    QString newDimString = i18n ("%1 x %2",
                                 newDim.width (),
                                 newDim.height ());
    m_afterTransformDimensionsLabel->setText (newDimString);
}


// public static
double kpTransformPreviewDialog::aspectScale (int newWidth, int newHeight,
                                         int oldWidth, int oldHeight)
{
    double widthScale = double (newWidth) / double (oldWidth);
    double heightScale = double (newHeight) / double (oldHeight);

    // Keeps aspect ratio
    return qMin (widthScale, heightScale);
}

// public static
int kpTransformPreviewDialog::scaleDimension (int dimension, double scale, int min, int max)
{
    return qMax (min,
                 qMin (max,
                       qRound (dimension * scale)));
}


// private
void kpTransformPreviewDialog::updateShrunkenDocumentPixmap ()
{
#if DEBUG_KP_TRANSFORM_PREVIEW_DIALOG
    qCDebug(kpLogDialogs) << "kpTransformPreviewDialog::updateShrunkenDocumentPixmap()"
               << " shrunkenDocPixmap.size="
               << m_shrunkenDocumentPixmap.size ()
               << " previewPixmapLabelSizeWhenUpdatedPixmap="
               << m_previewPixmapLabelSizeWhenUpdatedPixmap
               << " previewPixmapLabel.size="
               << m_previewPixmapLabel->size ()
               << endl;
#endif

    if (!m_previewGroupBox) {
        return;
    }


    kpDocument *doc = document ();
    Q_ASSERT (doc && !doc->image ().isNull ());

    if (m_shrunkenDocumentPixmap.isNull () ||
        m_previewPixmapLabel->size () != m_previewPixmapLabelSizeWhenUpdatedPixmap)
    {
    #if DEBUG_KP_TRANSFORM_PREVIEW_DIALOG
        qCDebug(kpLogDialogs) << "\tupdating shrunkenDocPixmap";
    #endif

        // TODO: Why the need to keep aspect ratio here?
        //       Isn't scaling the skewed result maintaining aspect enough?
        double keepsAspectScale = aspectScale (m_previewPixmapLabel->width (),
                                               m_previewPixmapLabel->height (),
                                               m_oldWidth,
                                               m_oldHeight);

        kpImage image;

        if (m_actOnSelection)
        {
            kpAbstractImageSelection *sel = doc->imageSelection ()->clone ();
            if (!sel->hasContent ()) {
                sel->setBaseImage (doc->getSelectedBaseImage ());
            }

            image = sel->transparentImage ();
            delete sel;
        }
        else
        {
            image = doc->image ();
        }

        m_shrunkenDocumentPixmap = kpPixmapFX::scale (
            image,
            scaleDimension (m_oldWidth,
                            keepsAspectScale,
                            1, m_previewPixmapLabel->width ()),
            scaleDimension (m_oldHeight,
                            keepsAspectScale,
                            1, m_previewPixmapLabel->height ()));

        m_previewPixmapLabelSizeWhenUpdatedPixmap = m_previewPixmapLabel->size ();
    }
}


// private
void kpTransformPreviewDialog::updatePreview ()
{
#if DEBUG_KP_TRANSFORM_PREVIEW_DIALOG
    qCDebug(kpLogDialogs) << "kpTransformPreviewDialog::updatePreview()";
#endif

    if (!m_previewGroupBox) {
        return;
    }


    kpDocument *doc = document ();
    if (!doc) {
        return;
    }


    if (!updatesEnabled ())
    {
    #if DEBUG_KP_TRANSFORM_PREVIEW_DIALOG
        qCDebug(kpLogDialogs) << "updates not enabled - aborting";
    #endif
        return;
    }


    updateShrunkenDocumentPixmap ();

    if (!m_shrunkenDocumentPixmap.isNull ())
    {
        QSize newDim = newDimensions ();
        double keepsAspectScale = aspectScale (m_previewPixmapLabel->width (),
                                               m_previewPixmapLabel->height (),
                                               newDim.width (),
                                               newDim.height ());

        int targetWidth = scaleDimension (newDim.width (),
                                          keepsAspectScale,
                                          1,  // min
                                          m_previewPixmapLabel->width ());  // max
        int targetHeight = scaleDimension (newDim.height (),
                                           keepsAspectScale,
                                           1,  // min
                                           m_previewPixmapLabel->height ());  // max

        // TODO: Some effects work directly on QImage; so could cache the
        //       QImage so that transformPixmap() is faster
        QImage transformedShrunkenDocumentPixmap =
            transformPixmap (m_shrunkenDocumentPixmap, targetWidth, targetHeight);

        QImage previewPixmap (m_previewPixmapLabel->width (),
                              m_previewPixmapLabel->height (), QImage::Format_ARGB32_Premultiplied);
        previewPixmap.fill(QColor(Qt::transparent).rgba());
        kpPixmapFX::setPixmapAt (&previewPixmap,
                                 (previewPixmap.width () - transformedShrunkenDocumentPixmap.width ()) / 2,
                                 (previewPixmap.height () - transformedShrunkenDocumentPixmap.height ()) / 2,
                                 transformedShrunkenDocumentPixmap);

#if DEBUG_KP_TRANSFORM_PREVIEW_DIALOG
    qCDebug(kpLogDialogs) << "kpTransformPreviewDialog::updatePreview ():"
               << "   shrunkenDocumentPixmap: w="
               << m_shrunkenDocumentPixmap.width ()
               << " h="
               << m_shrunkenDocumentPixmap.height ()
               << "   previewPixmapLabel: w="
               << m_previewPixmapLabel->width ()
               << " h="
               << m_previewPixmapLabel->height ()
               << "   transformedShrunkenDocumentPixmap: w="
               << transformedShrunkenDocumentPixmap.width ()
               << " h="
               << transformedShrunkenDocumentPixmap.height ()
               << "   previewPixmap: w="
               << previewPixmap.width ()
               << " h="
               << previewPixmap.height ()
               << endl;
#endif

        m_previewPixmapLabel->setPixmap (QPixmap::fromImage(previewPixmap));

        // immediate update esp. for expensive previews
        m_previewPixmapLabel->repaint ();

#if DEBUG_KP_TRANSFORM_PREVIEW_DIALOG
    qCDebug(kpLogDialogs) << "\tafter QLabel::setPixmap() previewPixmapLabel: w="
               << m_previewPixmapLabel->width ()
               << " h="
               << m_previewPixmapLabel->height ()
               << endl;
#endif
    }
}


// protected slot virtual
void kpTransformPreviewDialog::slotUpdate ()
{
#if DEBUG_KP_TRANSFORM_PREVIEW_DIALOG
    qCDebug(kpLogDialogs) << "kpTransformPreviewDialog::slotUpdate()";
#endif
    updateDimensions ();
    updatePreview ();
}

// protected slot virtual
void kpTransformPreviewDialog::slotUpdateWithWaitCursor ()
{
#if DEBUG_KP_TRANSFORM_PREVIEW_DIALOG
    qCDebug(kpLogDialogs) << "kpTransformPreviewDialog::slotUpdateWithWaitCursor()";
#endif

    QApplication::setOverrideCursor (Qt::WaitCursor);

    slotUpdate ();

    QApplication::restoreOverrideCursor ();
}


