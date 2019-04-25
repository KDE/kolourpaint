
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


#define DEBUG_KP_TOOL_FLOOD_FILL 0


#include "kpToolFloodFill.h"

#include "commands/kpCommandHistory.h"
#include "kpDefs.h"
#include "document/kpDocument.h"
#include "environments/tools/kpToolEnvironment.h"
#include "commands/tools/kpToolFloodFillCommand.h"

#include "kpLogCategories.h"
#include <KLocalizedString>

#include <QApplication>

//---------------------------------------------------------------------

struct kpToolFloodFillPrivate
{
    kpToolFloodFillCommand *currentCommand;
};

//---------------------------------------------------------------------

kpToolFloodFill::kpToolFloodFill (kpToolEnvironment *environ, QObject *parent)
    : kpTool (i18n ("Flood Fill"), i18n ("Fills regions in the image"),
              Qt::Key_F,
              environ, parent, QStringLiteral("tool_flood_fill")),
      d (new kpToolFloodFillPrivate ())
{
    d->currentCommand = nullptr;
}

//---------------------------------------------------------------------

kpToolFloodFill::~kpToolFloodFill ()
{
    delete d;
}

//---------------------------------------------------------------------

// private
QString kpToolFloodFill::haventBegunDrawUserMessage () const
{
    return i18n ("Click to fill a region.");
}

//---------------------------------------------------------------------

// public virtual [base kpTool]
void kpToolFloodFill::begin ()
{
    setUserMessage (haventBegunDrawUserMessage ());
}

//---------------------------------------------------------------------

// public virtual [base kpTool]
void kpToolFloodFill::beginDraw ()
{
#if DEBUG_KP_TOOL_FLOOD_FILL && 1
    qCDebug(kpLogTools) << "kpToolFloodFill::beginDraw()";
#endif

    QApplication::setOverrideCursor (Qt::WaitCursor);
    {
        environ ()->flashColorSimilarityToolBarItem ();

        // Flood Fill is an expensive CPU operation so we only fill at a
        // mouse click (beginDraw ()), not on mouse move (virtually draw())
        d->currentCommand = new kpToolFloodFillCommand (
            currentPoint ().x (), currentPoint ().y (),
            color (mouseButton ()), processedColorSimilarity (),
            environ ()->commandEnvironment ());

    #if DEBUG_KP_TOOL_FLOOD_FILL && 1
        qCDebug(kpLogTools) << "\tperforming new-doc-corner-case check";
    #endif

        if (document ()->url ().isEmpty () && !document ()->isModified ())
        {
            // Collect the colour that gets changed before we change the pixels
            // (execute() below).  Needed in unexecute().
            d->currentCommand->prepareColorToChange ();

            d->currentCommand->setFillEntireImage ();
        }

        d->currentCommand->execute ();
    }
    QApplication::restoreOverrideCursor ();

    setUserMessage (cancelUserMessage ());
}

//---------------------------------------------------------------------

// public virtual [base kpTool]
void kpToolFloodFill::draw (const QPoint &thisPoint, const QPoint &, const QRect &)
{
    setUserShapePoints (thisPoint);
}

//---------------------------------------------------------------------

// public virtual [base kpTool]
void kpToolFloodFill::cancelShape ()
{
    d->currentCommand->unexecute ();

    delete d->currentCommand;
    d->currentCommand = nullptr;

    setUserMessage (i18n ("Let go of all the mouse buttons."));
}

//---------------------------------------------------------------------

// public virtual [base kpTool]
void kpToolFloodFill::releasedAllButtons ()
{
    setUserMessage (haventBegunDrawUserMessage ());
}

//---------------------------------------------------------------------

// public virtual [base kpTool]
void kpToolFloodFill::endDraw (const QPoint &, const QRect &)
{
    environ ()->commandHistory ()->addCommand (d->currentCommand,
        false/*no exec - we already did it up there*/);

    // Don't delete - it just got added to the history.
    d->currentCommand = nullptr;
    setUserMessage (haventBegunDrawUserMessage ());
}

//---------------------------------------------------------------------

