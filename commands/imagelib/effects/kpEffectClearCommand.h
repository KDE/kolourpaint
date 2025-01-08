
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef kpEffectClearCommand_H
#define kpEffectClearCommand_H

#include "commands/kpCommand.h"

#include "imagelib/kpColor.h"
#include "imagelib/kpImage.h"

class kpEffectClearCommand : public kpCommand
{
public:
    kpEffectClearCommand(bool actOnSelection, const kpColor &newColor, kpCommandEnvironment *environ);
    ~kpEffectClearCommand() override;

    QString name() const override;

    SizeType size() const override;

    void execute() override;
    void unexecute() override;

private:
    bool m_actOnSelection;

    kpColor m_newColor;
    kpImage *m_oldImagePtr;
};

#endif // kpEffectClearCommand_H
