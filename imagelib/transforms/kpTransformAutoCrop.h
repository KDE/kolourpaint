
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef KP_TRANSFORM_AUTO_CROP_H
#define KP_TRANSFORM_AUTO_CROP_H

#include "commands/kpNamedCommand.h"

class QRect;

// class kpImage;
class kpMainWindow;
class kpTransformAutoCropBorder;

// REFACTOR: This should be moved into /commands/
class kpTransformAutoCropCommand : public kpNamedCommand
{
public:
    kpTransformAutoCropCommand(bool actOnSelection,
                               const kpTransformAutoCropBorder &leftBorder,
                               const kpTransformAutoCropBorder &rightBorder,
                               const kpTransformAutoCropBorder &topBorder,
                               const kpTransformAutoCropBorder &botBorder,
                               kpCommandEnvironment *environ);
    ~kpTransformAutoCropCommand() override;

    enum NameOptions {
        DontShowAccel = 0,
        ShowAccel = 1
    };

    static QString text(bool actOnSelection, int options);

    SizeType size() const override;

private:
    void getUndoImage(const kpTransformAutoCropBorder &border, kpImage **image);
    void getUndoImages();
    void deleteUndoImages();

public:
    void execute() override;
    void unexecute() override;

private:
    QRect contentsRect() const;

    struct kpTransformAutoCropCommandPrivate *d;
};

// (returns true on success (even if it did nothing) or false on error)
bool kpTransformAutoCrop(kpMainWindow *mainWindow);

#endif // KP_TRANSFORM_AUTO_CROP_H
