
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef kpToolColorPickerCommand_H
#define kpToolColorPickerCommand_H

#include "commands/kpCommand.h"
#include "imagelib/kpColor.h"

class kpToolColorPickerCommand : public kpCommand
{
public:
    kpToolColorPickerCommand(int mouseButton, const kpColor &newColor, const kpColor &oldColor, kpCommandEnvironment *environ);
    ~kpToolColorPickerCommand() override;

    QString name() const override;

    SizeType size() const override;

    void execute() override;
    void unexecute() override;

private:
    int m_mouseButton;
    kpColor m_newColor;
    kpColor m_oldColor;
};

#endif // kpToolColorPickerCommand_H
