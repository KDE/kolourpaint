
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


#include <kpAbstractSelectionTool.h>
#include <kpAbstractSelectionToolPrivate.h>

#include <qapplication.h>
#include <qbitmap.h>
#include <qcursor.h>
#include <qevent.h>
#include <qmenu.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qpolygon.h>
#include <qtimer.h>

#include <kdebug.h>
#include <klocale.h>

#include <kpAbstractImageSelection.h>
#include <kpAbstractSelection.h>
#include <kpBug.h>
#include <kpCommandHistory.h>
#include <kpDefs.h>
#include <kpDocument.h>
#include <kpMacroCommand.h>
#include <kpToolSelectionCreateCommand.h>
#include <kpToolSelectionDestroyCommand.h>
#include <kpToolSelectionEnvironment.h>
#include <kpToolSelectionMoveCommand.h>
#include <kpToolSelectionResizeScaleCommand.h>
#include <kpToolImageSelectionTransparencyCommand.h>
#include <kpToolToolBar.h>
#include <kpToolWidgetOpaqueOrTransparent.h>
#include <kpView.h>
#include <kpViewManager.h>


// protected virtual [base kpTool]
void kpAbstractSelectionTool::keyPressEvent (QKeyEvent *e)
{
#if DEBUG_KP_TOOL_SELECTION && 0
    kDebug () << "kpAbstractSelectionTool::keyPressEvent(e->text='"
              << e->text () << "')" << endl;
#endif


    e->ignore ();


    if (document ()->selection () &&
        !hasBegunDraw () &&
         e->key () == Qt::Key_Escape)
    {
    #if DEBUG_KP_TOOL_SELECTION && 0
        kDebug () << "\tescape pressed with sel when not begun draw - deselecting"
                  << endl;
    #endif

        pushOntoDocument ();
        e->accept ();
    }


    if (!e->isAccepted ())
    {
    #if DEBUG_KP_TOOL_SELECTION && 0
        kDebug () << "\tkey processing did not accept (text was '"
                   << e->text ()
                   << "') - passing on event to kpTool"
                   << endl;
    #endif

        kpTool::keyPressEvent (e);
        return;
    }
}
