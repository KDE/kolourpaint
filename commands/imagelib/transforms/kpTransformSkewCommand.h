
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef kpTransformSkewCommand_H
#define kpTransformSkewCommand_H

#include "commands/kpCommand.h"
#include "imagelib/kpColor.h"
#include "imagelib/kpImage.h"

class kpTransformSkewCommand : public kpCommand
{
public:
    kpTransformSkewCommand(bool actOnSelection, int hangle, int vangle, kpCommandEnvironment *environ);
    ~kpTransformSkewCommand() override;

    QString name() const override;

    SizeType size() const override;

    void execute() override;
    void unexecute() override;

private:
    bool m_actOnSelection;
    int m_hangle, m_vangle;

    kpColor m_backgroundColor;
    kpImage m_oldImage;
    kpAbstractImageSelection *m_oldSelectionPtr;
};

#endif // kpTransformSkewCommand_H
