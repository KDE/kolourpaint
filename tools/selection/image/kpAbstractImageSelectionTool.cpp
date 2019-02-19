
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


#include "kpAbstractImageSelectionTool.h"

#include <KLocalizedString>

#include "layers/selections/image/kpAbstractImageSelection.h"
#include "document/kpDocument.h"
#include "environments/tools/selection/kpToolSelectionEnvironment.h"
#include "commands/tools/selection/kpToolSelectionPullFromDocumentCommand.h"

//---------------------------------------------------------------------

kpAbstractImageSelectionTool::kpAbstractImageSelectionTool (
        const QString &text, const QString &description,
        int key,
        kpToolSelectionEnvironment *environ, QObject *parent,
        const QString &name)
    : kpAbstractSelectionTool (text, description,
        key, environ, parent, name)
{
}

//---------------------------------------------------------------------

// protected virtual [kpAbstractSelectionTool]
kpAbstractSelectionContentCommand *kpAbstractImageSelectionTool::newGiveContentCommand () const
{
    kpAbstractImageSelection *imageSel = document ()->imageSelection ();
    Q_ASSERT (imageSel && !imageSel->hasContent ());

    if (imageSel->transparency ().isTransparent ()) {
        environ ()->flashColorSimilarityToolBarItem ();
    }

    return new kpToolSelectionPullFromDocumentCommand (
            *imageSel,
            environ ()->backgroundColor (),
            QString()/*uninteresting child of macro cmd*/,
            environ ()->commandEnvironment ());
}

//---------------------------------------------------------------------
// protected virtual [kpAbstractSelectionTool]

QString kpAbstractImageSelectionTool::nameOfCreateCommand () const
{
    return i18n ("Selection: Create");
}

//---------------------------------------------------------------------
// protected virtual [kpAbstractSelectionTool]

QString kpAbstractImageSelectionTool::haventBegunDrawUserMessageCreate () const
{
    // TODO: This is wrong because you can still use RMB.
    return i18n ("Left drag to create selection.");
}

//---------------------------------------------------------------------
// protected virtual [kpAbstractSelectionTool]

QString kpAbstractImageSelectionTool::haventBegunDrawUserMessageMove () const
{
    return i18n ("Left drag to move selection.");
}

//---------------------------------------------------------------------
// protected virtual [kpAbstractSelectionTool]

QString kpAbstractImageSelectionTool::haventBegunDrawUserMessageResizeScale () const
{
    return i18n ("Left drag to scale selection.");
}

//---------------------------------------------------------------------

