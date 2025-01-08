
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef kpToolSelectionPullFromDocumentCommand_H
#define kpToolSelectionPullFromDocumentCommand_H

#include "imagelib/kpColor.h"
#include "kpAbstractSelectionContentCommand.h"

class kpAbstractImageSelection;

class kpToolSelectionPullFromDocumentCommand : public kpAbstractSelectionContentCommand
{
public:
    kpToolSelectionPullFromDocumentCommand(const kpAbstractImageSelection &originalSelBorder,
                                           const kpColor &backgroundColor,
                                           const QString &name,
                                           kpCommandEnvironment *environ);
    ~kpToolSelectionPullFromDocumentCommand() override;

    void execute() override;
    void unexecute() override;

private:
    kpColor m_backgroundColor;
};

#endif // kpToolSelectionPullFromDocumentCommand_H
