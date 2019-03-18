
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


#define DEBUG_KP_TOOL_SELECTION 0


#include "kpToolSelectionResizeScaleCommand.h"

#include "layers/selections/kpAbstractSelection.h"
#include "layers/selections/image/kpAbstractImageSelection.h"
#include "environments/commands/kpCommandEnvironment.h"
#include "kpDefs.h"
#include "document/kpDocument.h"
#include "layers/selections/image/kpRectangularImageSelection.h"
#include "layers/selections/text/kpTextSelection.h"
#include "kpLogCategories.h"

#include <QApplication>
#include <QCursor>
#include <QTimer>

#include <KLocalizedString>

//--------------------------------------------------------------------------------

kpToolSelectionResizeScaleCommand::kpToolSelectionResizeScaleCommand (
        kpCommandEnvironment *environ)
    : kpNamedCommand (environ->textSelection () ?
                         i18n ("Text: Resize Box") :
                         i18n ("Selection: Smooth Scale"),
                      environ),
      m_smoothScaleTimer (new QTimer (this))
{
    m_originalSelectionPtr = selection ()->clone ();

    m_newTopLeft = selection ()->topLeft ();
    m_newWidth = selection ()->width ();
    m_newHeight = selection ()->height ();

    m_smoothScaleTimer->setSingleShot (true);
    connect (m_smoothScaleTimer, &QTimer::timeout, this, [this]{resizeScaleAndMove(false);});
}

kpToolSelectionResizeScaleCommand::~kpToolSelectionResizeScaleCommand ()
{
    delete m_originalSelectionPtr;
}


// public virtual
kpCommandSize::SizeType kpToolSelectionResizeScaleCommand::size () const
{
    return SelectionSize (m_originalSelectionPtr);
}


// public
const kpAbstractSelection *kpToolSelectionResizeScaleCommand::originalSelection () const
{
    return m_originalSelectionPtr;
}


// public
QPoint kpToolSelectionResizeScaleCommand::topLeft () const
{
    return m_newTopLeft;
}

// public
void kpToolSelectionResizeScaleCommand::moveTo (const QPoint &point)
{
    if (point == m_newTopLeft) {
        return;
    }

    m_newTopLeft = point;
    selection ()->moveTo (m_newTopLeft);
}


// public
int kpToolSelectionResizeScaleCommand::width () const
{
    return m_newWidth;
}

// public
int kpToolSelectionResizeScaleCommand::height () const
{
    return m_newHeight;
}

// public
void kpToolSelectionResizeScaleCommand::resize (int width, int height,
                                                bool delayed)
{
    if (width == m_newWidth && height == m_newHeight) {
        return;
    }

    m_newWidth = width;
    m_newHeight = height;

    resizeScaleAndMove (delayed);
}


// public
void kpToolSelectionResizeScaleCommand::resizeAndMoveTo (int width, int height,
                                                         const QPoint &point,
                                                         bool delayed)
{
    if (width == m_newWidth && height == m_newHeight &&
        point == m_newTopLeft)
    {
        return;
    }

    m_newWidth = width;
    m_newHeight = height;
    m_newTopLeft = point;

    resizeScaleAndMove (delayed);
}


// protected
void kpToolSelectionResizeScaleCommand::killSmoothScaleTimer ()
{
    m_smoothScaleTimer->stop ();
}


// protected
void kpToolSelectionResizeScaleCommand::resizeScaleAndMove (bool delayed)
{
#if DEBUG_KP_TOOL_SELECTION
    qCDebug(kpLogCommands) << "kpToolSelectionResizeScaleCommand::resizeScaleAndMove(delayed="
               << delayed << ")";
#endif

    killSmoothScaleTimer ();

    kpAbstractSelection *newSelPtr = nullptr;

    if (textSelection ())
    {
        Q_ASSERT (dynamic_cast <kpTextSelection *> (m_originalSelectionPtr));
        auto *orgTextSel = dynamic_cast <kpTextSelection *> (m_originalSelectionPtr);

        newSelPtr = orgTextSel->resized (m_newWidth, m_newHeight);
    }
    else
    {
        Q_ASSERT (dynamic_cast <kpAbstractImageSelection *> (m_originalSelectionPtr));
        auto *imageSel = dynamic_cast <kpAbstractImageSelection *> (m_originalSelectionPtr);

        newSelPtr = new kpRectangularImageSelection (
            QRect (imageSel->x (),
                   imageSel->y (),
                   m_newWidth,
                   m_newHeight),
            kpPixmapFX::scale (imageSel->baseImage (),
                               m_newWidth, m_newHeight,
                               !delayed/*if not delayed, smooth*/),
            imageSel->transparency ());

        if (delayed)
        {
            // Call self (once) with delayed==false in 200ms
            m_smoothScaleTimer->start (200/*ms*/);
        }
    }

    Q_ASSERT (newSelPtr);
    newSelPtr->moveTo (m_newTopLeft);

    document ()->setSelection (*newSelPtr);

    delete newSelPtr;
}


// public
void kpToolSelectionResizeScaleCommand::finalize ()
{
#if DEBUG_KP_TOOL_SELECTION
    qCDebug(kpLogCommands) << "kpToolSelectionResizeScaleCommand::finalize()"
               << " smoothScaleTimer->isActive="
               << m_smoothScaleTimer->isActive ();
#endif

    // Make sure the selection contains the final image and the timer won't
    // fire afterwards.
    if (m_smoothScaleTimer->isActive ())
    {
        resizeScaleAndMove ();
        Q_ASSERT (!m_smoothScaleTimer->isActive ());
    }
}


// public virtual [base kpToolResizeScaleCommand]
void kpToolSelectionResizeScaleCommand::execute ()
{
    QApplication::setOverrideCursor (Qt::WaitCursor);

    killSmoothScaleTimer ();

    resizeScaleAndMove ();

    environ ()->somethingBelowTheCursorChanged ();

    QApplication::restoreOverrideCursor ();
}

// public virtual [base kpToolResizeScaleCommand]
void kpToolSelectionResizeScaleCommand::unexecute ()
{
    QApplication::setOverrideCursor (Qt::WaitCursor);

    killSmoothScaleTimer ();

    document ()->setSelection (*m_originalSelectionPtr);

    environ ()->somethingBelowTheCursorChanged ();

    QApplication::restoreOverrideCursor ();
}


