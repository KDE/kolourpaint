
/*
   Copyright (c) 2003-2006 Clarence Dang <dang@kde.org>
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


#define DEBUG_KP_TOOL_FLOW_COMMAND 0


#include <kptoolflowcommand.h>

#include <qpixmap.h>
#include <qrect.h>

#include <kpdocument.h>
#include <kppixmapfx.h>
#include <kptool.h>
#include <kpviewmanager.h>


struct kpToolFlowCommandPrivate
{
    QPixmap m_pixmap;
    QRect m_boundingRect;
};


kpToolFlowCommand::kpToolFlowCommand (const QString &name, kpMainWindow *mainWindow)
    : kpNamedCommand (name, mainWindow),
      d (new kpToolFlowCommandPrivate ())
{
    d->m_pixmap = *document ()->pixmap ();
}

kpToolFlowCommand::~kpToolFlowCommand ()
{
    delete d;
}


// public virtual [base kpCommand]
int kpToolFlowCommand::size () const
{
    return kpPixmapFX::pixmapSize (d->m_pixmap);
}


// public virtual [base kpCommand]
void kpToolFlowCommand::execute ()
{
    swapOldAndNew ();
}

// public virtual [base kpCommand]
void kpToolFlowCommand::unexecute ()
{
    swapOldAndNew ();
}


// private
void kpToolFlowCommand::swapOldAndNew ()
{
    if (d->m_boundingRect.isValid ())
    {
        QPixmap oldPixmap = document ()->getPixmapAt (d->m_boundingRect);

        document ()->setPixmapAt (d->m_pixmap, d->m_boundingRect.topLeft ());

        d->m_pixmap = oldPixmap;
    }
}

// public
void kpToolFlowCommand::updateBoundingRect (const QPoint &point)
{
    updateBoundingRect (QRect (point, point));
}

// public
void kpToolFlowCommand::updateBoundingRect (const QRect &rect)
{
#if DEBUG_KP_TOOL_FLOW_COMMAND & 0
    kDebug () << "kpToolFlowCommand::updateBoundingRect()  existing="
               << d->m_boundingRect
               << " plus="
               << rect
               << endl;
#endif
    d->m_boundingRect = d->m_boundingRect.unite (rect);
#if DEBUG_KP_TOOL_FLOW_COMMAND & 0
    kDebug () << "\tresult=" << d->m_boundingRect << endl;
#endif
}

// public
void kpToolFlowCommand::finalize ()
{
    if (d->m_boundingRect.isValid ())
    {
        // store only needed part of doc pixmap
        d->m_pixmap = kpTool::neededPixmap (d->m_pixmap, d->m_boundingRect);
    }
    else
    {
        d->m_pixmap = QPixmap ();
    }
}

// public
void kpToolFlowCommand::cancel ()
{
    if (d->m_boundingRect.isValid ())
    {
        viewManager ()->setFastUpdates ();
        document ()->setPixmapAt (d->m_pixmap, d->m_boundingRect.topLeft ());
        viewManager ()->restoreFastUpdates ();
    }
}
