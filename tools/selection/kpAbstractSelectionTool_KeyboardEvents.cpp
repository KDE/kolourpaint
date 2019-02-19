
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


#include "kpAbstractSelectionTool.h"
#include "kpAbstractSelectionToolPrivate.h"

#include "document/kpDocument.h"
#include "environments/tools/selection/kpToolSelectionEnvironment.h"
#include "commands/tools/selection/kpToolSelectionMoveCommand.h"
#include "layers/selections/kpAbstractSelection.h"

#include <QKeyEvent>

#include "kpLogCategories.h"

//---------------------------------------------------------------------

// protected virtual [base kpTool]
void kpAbstractSelectionTool::keyPressEvent (QKeyEvent *e)
{
#if DEBUG_KP_TOOL_SELECTION && 0
    qCDebug(kpLogTools) << "kpAbstractSelectionTool::keyPressEvent(e->text='"
              << e->text () << "')";
#endif

    e->ignore ();

    if (document ()->selection () &&
        !hasBegunDraw () &&
         e->key () == Qt::Key_Escape)
    {
    #if DEBUG_KP_TOOL_SELECTION && 0
        qCDebug(kpLogTools) << "\tescape pressed with sel when not begun draw - deselecting";
    #endif

        pushOntoDocument ();
        e->accept ();
    }
    else
    {
    #if DEBUG_KP_TOOL_SELECTION && 0
        qCDebug(kpLogTools) << "\tkey processing did not accept (text was '"
                   << e->text ()
                   << "') - passing on event to kpTool";
    #endif

      if ( document()->selection() && !hasBegunDraw() &&
           ((e->key() == Qt::Key_Left) ||
            (e->key() == Qt::Key_Right) ||
            (e->key() == Qt::Key_Up) ||
            (e->key() == Qt::Key_Down)) )
      {
        // move selection with cursor keys pixel-wise
        giveContentIfNeeded();

        if ( !d->currentMoveCommand )
        {
          d->currentMoveCommand = new kpToolSelectionMoveCommand(
              QString()/*uninteresting child of macro cmd*/,
              environ()->commandEnvironment());
          d->currentMoveCommandIsSmear = false;
        }

        int dx, dy;
        arrowKeyPressDirection(e, &dx, &dy);
        d->currentMoveCommand->moveTo(document()->selection()->topLeft() + QPoint(dx, dy));
        endDrawMove();
      }
      else
        kpTool::keyPressEvent(e);
    }
}

//---------------------------------------------------------------------
