
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#define DEBUG_KP_TOOL_CROP 0

#include "kpTransformCrop.h"
#include "kpTransformCropPrivate.h"

#include "commands/imagelib/transforms/kpTransformResizeScaleCommand.h"
#include "document/kpDocument.h"
#include "layers/selections/image/kpAbstractImageSelection.h"
#include "layers/selections/kpAbstractSelection.h"
#include "layers/selections/text/kpTextSelection.h"
#include "mainWindow/kpMainWindow.h"

#include <KLocalizedString>

void kpTransformCrop(kpMainWindow *mainWindow)
{
    kpDocument *doc = mainWindow->document();
    Q_ASSERT(doc);

    kpAbstractSelection *sel = doc->selection();
    Q_ASSERT(sel);

    kpCommand *resizeDocCommand = new kpTransformResizeScaleCommand(false /*act on doc, not sel*/,
                                                                    sel->width(),
                                                                    sel->height(),
                                                                    kpTransformResizeScaleCommand::Resize,
                                                                    mainWindow->commandEnvironment());

    auto *textSel = dynamic_cast<kpTextSelection *>(sel);
    auto *imageSel = dynamic_cast<kpAbstractImageSelection *>(sel);
    // It's either a text selection or an image selection, but cannot be
    // neither and cannot be both.
    Q_ASSERT(!!textSel != !!imageSel);

    if (textSel) {
        ::kpTransformCrop_TextSelection(mainWindow, i18n("Set as Image"), resizeDocCommand);
    } else if (imageSel) {
        ::kpTransformCrop_ImageSelection(mainWindow, i18n("Set as Image"), resizeDocCommand);
    } else {
        Q_ASSERT(!"unreachable");
    }
}
