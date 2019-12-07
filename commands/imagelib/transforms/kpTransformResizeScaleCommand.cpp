
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


#define DEBUG_KP_TOOL_RESIZE_SCALE_COMMAND 0
#define DEBUG_KP_TOOL_RESIZE_SCALE_DIALOG 0


#include "kpTransformResizeScaleCommand.h"

#include "layers/selections/image/kpAbstractImageSelection.h"
#include "environments/commands/kpCommandEnvironment.h"
#include "kpDefs.h"
#include "document/kpDocument.h"
#include "layers/selections/image/kpFreeFormImageSelection.h"
#include "pixmapfx/kpPixmapFX.h"
#include "layers/selections/image/kpRectangularImageSelection.h"
#include "layers/selections/text/kpTextSelection.h"


#include <QApplication>
#include <QPoint>
#include <QPolygon>
#include <QRect>
#include <QSize>
#include <QTransform>

#include "kpLogCategories.h"

#include <KLocalizedString>

//--------------------------------------------------------------------------------

kpTransformResizeScaleCommand::kpTransformResizeScaleCommand (bool actOnSelection,
        int newWidth, int newHeight,
        Type type,
        kpCommandEnvironment *environ)
    : kpCommand (environ),
      m_actOnSelection (actOnSelection),
      m_type (type),
      m_backgroundColor (environ->backgroundColor ()),
      m_oldSelectionPtr (nullptr)
{
    kpDocument *doc = document ();
    Q_ASSERT (doc);

    m_oldWidth = doc->width (m_actOnSelection);
    m_oldHeight = doc->height (m_actOnSelection);

    m_actOnTextSelection = (m_actOnSelection &&
                            doc->textSelection ());

    resize (newWidth, newHeight);

    // If we have a selection _border_ (but not a floating selection),
    // then scale the selection with the document
    m_scaleSelectionWithImage = (!m_actOnSelection &&
                                 (m_type == Scale || m_type == SmoothScale) &&
                                 document ()->selection () &&
                                 !document ()->selection ()->hasContent ());
}

kpTransformResizeScaleCommand::~kpTransformResizeScaleCommand ()
{
    delete m_oldSelectionPtr;
}


// public virtual [base kpCommand]
QString kpTransformResizeScaleCommand::name () const
{
    if (m_actOnSelection)
    {
        if (m_actOnTextSelection)
        {
            if (m_type == Resize) {
                return i18n ("Text: Resize Box");
            }
        }
        else
        {
            if (m_type == Scale) {
                return i18n ("Selection: Scale");
            }

            if (m_type == SmoothScale) {
                return i18n ("Selection: Smooth Scale");
            }
        }
    }
    else
    {
        switch (m_type)
        {
        case Resize:
            return i18n ("Resize");
        case Scale:
            return i18n ("Scale");
        case SmoothScale:
            return i18n ("Smooth Scale");
        }
    }

    return {};
}

// public virtual [base kpCommand]
kpCommandSize::SizeType kpTransformResizeScaleCommand::size () const
{
    return ImageSize (m_oldImage) +
           ImageSize (m_oldRightImage) +
           ImageSize (m_oldBottomImage) +
           SelectionSize (m_oldSelectionPtr);
}


// public
int kpTransformResizeScaleCommand::newWidth () const
{
    return m_newWidth;
}

// public
void kpTransformResizeScaleCommand::setNewWidth (int width)
{
    resize (width, newHeight ());
}


// public
int kpTransformResizeScaleCommand::newHeight () const
{
    return m_newHeight;
}

// public
void kpTransformResizeScaleCommand::setNewHeight (int height)
{
    resize (newWidth (), height);
}


// public
QSize kpTransformResizeScaleCommand::newSize () const
{
    return  {newWidth (), newHeight ()};
}

// public virtual
void kpTransformResizeScaleCommand::resize (int width, int height)
{
    m_newWidth = width;
    m_newHeight = height;

    m_isLosslessScale = ((m_type == Scale) &&
                         (m_newWidth / m_oldWidth * m_oldWidth == m_newWidth) &&
                         (m_newHeight / m_oldHeight * m_oldHeight == m_newHeight));
}


// public
bool kpTransformResizeScaleCommand::scaleSelectionWithImage () const
{
    return m_scaleSelectionWithImage;
}


// private
void kpTransformResizeScaleCommand::scaleSelectionRegionWithDocument ()
{
#if DEBUG_KP_TOOL_RESIZE_SCALE_COMMAND
    qCDebug(kpLogCommands) << "kpTransformResizeScaleCommand::scaleSelectionRegionWithDocument";
#endif

    Q_ASSERT (m_oldSelectionPtr);
    Q_ASSERT (!m_oldSelectionPtr->hasContent ());


    const double horizScale = double (m_newWidth) / double (m_oldWidth);
    const double vertScale = double (m_newHeight) / double (m_oldHeight);

    const int newX = static_cast<int> (m_oldSelectionPtr->x () * horizScale);
    const int newY = static_cast<int> (m_oldSelectionPtr->y () * vertScale);


    QPolygon currentPoints = m_oldSelectionPtr->calculatePoints ();
    currentPoints.translate (-currentPoints.boundingRect ().x (),
                             -currentPoints.boundingRect ().y ());

    // TODO: refactor into kpPixmapFX
    // TODO: Can we get to size 0x0 accidentally?
    QTransform scaleMatrix;
    scaleMatrix.scale (horizScale, vertScale);
    currentPoints = scaleMatrix.map (currentPoints);

    currentPoints.translate (
        -currentPoints.boundingRect ().x () + newX,
        -currentPoints.boundingRect ().y () + newY);

    auto *imageSel = dynamic_cast <kpAbstractImageSelection *> (m_oldSelectionPtr);
    auto *textSel = dynamic_cast <kpTextSelection *> (m_oldSelectionPtr);

    if (imageSel)
    {
        document ()->setSelection (
            kpFreeFormImageSelection (currentPoints, kpImage (),
                imageSel->transparency ()));
    }
    else if (textSel)
    {
        document ()->setSelection (
            kpTextSelection (currentPoints.boundingRect (),
                textSel->textLines (),
                textSel->textStyle ()));
    }
    else {
        Q_ASSERT (!"Unknown selection type");
    }


    environ ()->somethingBelowTheCursorChanged ();
}


// public virtual [base kpCommand]
void kpTransformResizeScaleCommand::execute ()
{
#if DEBUG_KP_TOOL_RESIZE_SCALE_COMMAND
    qCDebug(kpLogCommands) << "kpTransformResizeScaleCommand::execute() type="
               << (int) m_type
               << " oldWidth=" << m_oldWidth
               << " oldHeight=" << m_oldHeight
               << " newWidth=" << m_newWidth
               << " newHeight=" << m_newHeight;
#endif

    if (m_oldWidth == m_newWidth && m_oldHeight == m_newHeight)
        return;

    if (m_type == Resize)
    {
        if (m_actOnSelection)
        {
            if (!m_actOnTextSelection) {
                Q_ASSERT (!"kpTransformResizeScaleCommand::execute() resizing sel doesn't make sense");
            }

            QApplication::setOverrideCursor (Qt::WaitCursor);

            kpTextSelection *textSel = textSelection ();
            Q_ASSERT (textSel);

            kpTextSelection *newSel = textSel->resized (m_newWidth, m_newHeight);
            document ()->setSelection (*newSel);
            delete newSel;

            environ ()->somethingBelowTheCursorChanged ();

            QApplication::restoreOverrideCursor ();
        }
        else
        {
            QApplication::setOverrideCursor (Qt::WaitCursor);


            if (m_newWidth < m_oldWidth)
            {
                m_oldRightImage = document ()->getImageAt (
                    QRect (m_newWidth, 0,
                        m_oldWidth - m_newWidth, m_oldHeight));
            }

            if (m_newHeight < m_oldHeight)
            {
                m_oldBottomImage = document ()->getImageAt (
                    QRect (0, m_newHeight,
                        m_newWidth, m_oldHeight - m_newHeight));
            }

            document ()->resize (m_newWidth, m_newHeight, m_backgroundColor);


            QApplication::restoreOverrideCursor ();
        }
    }
    // Scale
    else
    {
        QApplication::setOverrideCursor (Qt::WaitCursor);


        kpImage oldImage = document ()->image (m_actOnSelection);

        if (!m_isLosslessScale) {
            m_oldImage = oldImage;
        }

        kpImage newImage = kpPixmapFX::scale (oldImage, m_newWidth, m_newHeight,
                                               m_type == SmoothScale);


        if (!m_oldSelectionPtr && document ()->selection ())
        {
            // Save sel border
            m_oldSelectionPtr = document ()->selection ()->clone ();
            m_oldSelectionPtr->deleteContent ();
        }

        if (m_actOnSelection)
        {
            if (m_actOnTextSelection) {
                Q_ASSERT (!"kpTransformResizeScaleCommand::execute() scaling text sel doesn't make sense");
            }

            Q_ASSERT (m_oldSelectionPtr);
            if ( !m_oldSelectionPtr ) {  // make coverity happy
                return;
            }

            QRect newRect = QRect (m_oldSelectionPtr->x (), m_oldSelectionPtr->y (),
                                   newImage.width (), newImage.height ());

            // Not possible to retain non-rectangular selection borders on scale
            // (think about e.g. a 45 deg line as part of the border & 2x scale)
            Q_ASSERT (dynamic_cast <kpAbstractImageSelection *> (m_oldSelectionPtr));
            document ()->setSelection (
                kpRectangularImageSelection (newRect, newImage,
                    dynamic_cast <kpAbstractImageSelection *> (m_oldSelectionPtr)
                        ->transparency ()));

            environ ()->somethingBelowTheCursorChanged ();
        }
        else
        {
            document ()->setImage (newImage);

            if (m_scaleSelectionWithImage)
            {
                scaleSelectionRegionWithDocument ();
            }
        }


        QApplication::restoreOverrideCursor ();
    }
}

// public virtual [base kpCommand]
void kpTransformResizeScaleCommand::unexecute ()
{
#if DEBUG_KP_TOOL_RESIZE_SCALE_COMMAND
    qCDebug(kpLogCommands) << "kpTransformResizeScaleCommand::unexecute() type="
               << m_type;
#endif

    if (m_oldWidth == m_newWidth && m_oldHeight == m_newHeight) {
        return;
    }

    kpDocument *doc = document ();
    Q_ASSERT (doc);

    if (m_type == Resize)
    {
        if (m_actOnSelection)
        {
            if (!m_actOnTextSelection) {
                Q_ASSERT (!"kpTransformResizeScaleCommand::unexecute() resizing sel doesn't make sense");
            }

            QApplication::setOverrideCursor (Qt::WaitCursor);

            kpTextSelection *textSel = textSelection ();
            Q_ASSERT (textSel);

            kpTextSelection *newSel = textSel->resized (m_oldWidth, m_oldHeight);
            document ()->setSelection (*newSel);
            delete newSel;

            environ ()->somethingBelowTheCursorChanged ();

            QApplication::restoreOverrideCursor ();
        }
        else
        {
            QApplication::setOverrideCursor (Qt::WaitCursor);


            kpImage newImage (m_oldWidth, m_oldHeight, QImage::Format_ARGB32_Premultiplied);

            kpPixmapFX::setPixmapAt (&newImage, QPoint (0, 0),
                                     doc->image ());

            if (m_newWidth < m_oldWidth)
            {
                kpPixmapFX::setPixmapAt (&newImage,
                                        QPoint (m_newWidth, 0),
                                        m_oldRightImage);
            }

            if (m_newHeight < m_oldHeight)
            {
                kpPixmapFX::setPixmapAt (&newImage,
                                        QPoint (0, m_newHeight),
                                        m_oldBottomImage);
            }

            doc->setImage (newImage);


            QApplication::restoreOverrideCursor ();
        }
    }
    // Scale
    else
    {
        QApplication::setOverrideCursor (Qt::WaitCursor);


        kpImage oldImage;

        if (!m_isLosslessScale) {
            oldImage = m_oldImage;
        } else {
            oldImage = kpPixmapFX::scale (doc->image (m_actOnSelection),
                                          m_oldWidth, m_oldHeight);
        }


        if (m_actOnSelection)
        {
            if (m_actOnTextSelection) {
                Q_ASSERT (!"kpTransformResizeScaleCommand::unexecute() scaling text sel doesn't make sense");
            }

            Q_ASSERT (dynamic_cast <kpAbstractImageSelection *> (m_oldSelectionPtr));
            auto *oldImageSel = dynamic_cast <kpAbstractImageSelection *> (m_oldSelectionPtr);

            kpAbstractImageSelection *oldSelection = oldImageSel->clone ();
            oldSelection->setBaseImage (oldImage);
            doc->setSelection (*oldSelection);
            delete oldSelection;

            environ ()->somethingBelowTheCursorChanged ();
        }
        else
        {
            doc->setImage (oldImage);

            if (m_scaleSelectionWithImage)
            {
                doc->setSelection (*m_oldSelectionPtr);

                environ ()->somethingBelowTheCursorChanged ();
            }
        }


        QApplication::restoreOverrideCursor ();
    }
}

