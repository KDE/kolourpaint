
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef kpTransformRotateCommand_H
#define kpTransformRotateCommand_H

#include "commands/kpCommand.h"
#include "imagelib/kpColor.h"
#include "imagelib/kpImage.h"

class kpAbstractImageSelection;

class kpTransformRotateCommand : public kpCommand
{
public:
    kpTransformRotateCommand(bool actOnSelection,
                             double angle, // 0 <= angle < 360 (clockwise)
                             kpCommandEnvironment *environ);
    ~kpTransformRotateCommand() override;

    QString name() const override;

    SizeType size() const override;

    void execute() override;
    void unexecute() override;

private:
    bool m_actOnSelection;
    double m_angle;

    kpColor m_backgroundColor;

    bool m_losslessRotation;
    kpImage m_oldImage;
    kpAbstractImageSelection *m_oldSelectionPtr;
};

#endif // kpTransformRotateCommand_H
