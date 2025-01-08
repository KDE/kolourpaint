
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef kpToolFloodFillCommand_H
#define kpToolFloodFillCommand_H

#include "commands/kpCommand.h"
#include "imagelib/kpFloodFill.h"

class kpColor;
class kpCommandEnvironment;

struct kpToolFloodFillCommandPrivate;

class kpToolFloodFillCommand : public kpCommand, public kpFloodFill
{
public:
    kpToolFloodFillCommand(int x, int y, const kpColor &color, int processedColorSimilarity, kpCommandEnvironment *environ);
    ~kpToolFloodFillCommand() override;

    QString name() const override;

    kpCommandSize::SizeType size() const override;

    // Optimization hack: filling a fresh, unmodified document does not require
    //                    reading any pixels - just set the whole document to
    //                    <color>.
    void setFillEntireImage(bool yes = true);

    void execute() override;
    void unexecute() override;

private:
    kpToolFloodFillCommandPrivate *const d;
};

#endif // kpToolFloodFillCommand_H
