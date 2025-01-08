
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#include "kpAbstractImageSelectionTool.h"

#include <KLocalizedString>

#include "commands/tools/selection/kpToolSelectionPullFromDocumentCommand.h"
#include "document/kpDocument.h"
#include "environments/tools/selection/kpToolSelectionEnvironment.h"
#include "layers/selections/image/kpAbstractImageSelection.h"

//---------------------------------------------------------------------

kpAbstractImageSelectionTool::kpAbstractImageSelectionTool(const QString &text,
                                                           const QString &description,
                                                           int key,
                                                           kpToolSelectionEnvironment *environ,
                                                           QObject *parent,
                                                           const QString &name)
    : kpAbstractSelectionTool(text, description, key, environ, parent, name)
{
}

//---------------------------------------------------------------------

// protected virtual [kpAbstractSelectionTool]
kpAbstractSelectionContentCommand *kpAbstractImageSelectionTool::newGiveContentCommand() const
{
    kpAbstractImageSelection *imageSel = document()->imageSelection();
    Q_ASSERT(imageSel && !imageSel->hasContent());

    if (imageSel->transparency().isTransparent()) {
        environ()->flashColorSimilarityToolBarItem();
    }

    return new kpToolSelectionPullFromDocumentCommand(*imageSel,
                                                      environ()->backgroundColor(),
                                                      QString() /*uninteresting child of macro cmd*/,
                                                      environ()->commandEnvironment());
}

//---------------------------------------------------------------------
// protected virtual [kpAbstractSelectionTool]

QString kpAbstractImageSelectionTool::nameOfCreateCommand() const
{
    return i18n("Selection: Create");
}

//---------------------------------------------------------------------
// protected virtual [kpAbstractSelectionTool]

QString kpAbstractImageSelectionTool::haventBegunDrawUserMessageCreate() const
{
    // TODO: This is wrong because you can still use RMB.
    return i18n("Left drag to create selection.");
}

//---------------------------------------------------------------------
// protected virtual [kpAbstractSelectionTool]

QString kpAbstractImageSelectionTool::haventBegunDrawUserMessageMove() const
{
    return i18n("Left drag to move selection.");
}

//---------------------------------------------------------------------
// protected virtual [kpAbstractSelectionTool]

QString kpAbstractImageSelectionTool::haventBegunDrawUserMessageResizeScale() const
{
    return i18n("Left drag to scale selection.");
}

//---------------------------------------------------------------------

#include "moc_kpAbstractImageSelectionTool.cpp"
