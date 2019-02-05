
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


#ifndef kpToolSelectionResizeScaleCommand_H
#define kpToolSelectionResizeScaleCommand_H


#include <QObject>
#include <QPoint>

#include "commands/kpNamedCommand.h"


class QTimer;

class kpAbstractSelection;


// You could subclass kpToolResizeScaleCommand and/or
// kpToolSelectionMoveCommand instead if want a disaster.
// This is different to kpToolResizeScaleCommand in that:
//
// 1. This only works for selections.
// 2. This is designed for the size and position to change several times
//    before execute().
//
// REFACTOR: Later: I take that all back.  We should merge with
//           kpToolResizeScaleCommand to reduce code duplication.
class kpToolSelectionResizeScaleCommand : public QObject,
                                          public kpNamedCommand
{
Q_OBJECT

public:
    kpToolSelectionResizeScaleCommand (kpCommandEnvironment *environ);
    ~kpToolSelectionResizeScaleCommand () override;

    kpCommandSize::SizeType size () const override;

public:
    const kpAbstractSelection *originalSelection () const;

    QPoint topLeft () const;
    void moveTo (const QPoint &point);

    int width () const;
    int height () const;
    void resize (int width, int height, bool delayed = false);

    // (equivalent to resize() followed by moveTo() but faster)
    void resizeAndMoveTo (int width, int height, const QPoint &point,
                          bool delayed = false);

protected:
    void killSmoothScaleTimer ();

    // If <delayed>, does a fast, low-quality scale and then calls itself
    // with <delayed> unset for a smooth scale, a short time later.
    // If acting on a text box, <delayed> is ignored.
    void resizeScaleAndMove (bool delayed = false);

public:
    void finalize ();

public:
    void execute () override;
    void unexecute () override;

protected:
    kpAbstractSelection *m_originalSelectionPtr;

    QPoint m_newTopLeft;
    int m_newWidth, m_newHeight;

    QTimer *m_smoothScaleTimer;
};


#endif  // kpToolSelectionResizeScaleCommand_H
