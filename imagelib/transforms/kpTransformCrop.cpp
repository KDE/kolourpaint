
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


#define DEBUG_KP_TOOL_CROP 0


#include "kpTransformCrop.h"
#include "kpTransformCropPrivate.h"

#include "layers/selections/image/kpAbstractImageSelection.h"
#include "layers/selections/kpAbstractSelection.h"
#include "document/kpDocument.h"
#include "mainWindow/kpMainWindow.h"
#include "layers/selections/text/kpTextSelection.h"
#include "commands/imagelib/transforms/kpTransformResizeScaleCommand.h"

#include <KLocalizedString>


void kpTransformCrop (kpMainWindow *mainWindow)
{
    kpDocument *doc = mainWindow->document ();
    Q_ASSERT (doc);

    kpAbstractSelection *sel = doc->selection ();
    Q_ASSERT (sel);


    kpCommand *resizeDocCommand =
        new kpTransformResizeScaleCommand (
            false/*act on doc, not sel*/,
            sel->width (), sel->height (),
            kpTransformResizeScaleCommand::Resize,
            mainWindow->commandEnvironment ());


    auto *textSel = dynamic_cast <kpTextSelection *> (sel);
    auto *imageSel = dynamic_cast <kpAbstractImageSelection *> (sel);
    // It's either a text selection or an image selection, but cannot be
    // neither or both.
    Q_ASSERT (!!textSel != !!imageSel);

    if (textSel) {
        ::kpTransformCrop_TextSelection (mainWindow, i18n ("Set as Image"), resizeDocCommand);
    }
    else if (imageSel) {
        ::kpTransformCrop_ImageSelection (mainWindow, i18n ("Set as Image"), resizeDocCommand);
    }
    else {
        Q_ASSERT (!"unreachable");
    }
}
