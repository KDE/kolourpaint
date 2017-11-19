
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


#ifndef kpToolSelectionMoveCommand_H
#define kpToolSelectionMoveCommand_H


#include <QPoint>
#include <QPolygon>
#include <QRect>

#include "imagelib/kpImage.h"
#include "commands/kpNamedCommand.h"


class kpAbstractSelection;


class kpToolSelectionMoveCommand : public kpNamedCommand
{
public:
    kpToolSelectionMoveCommand (const QString &name, kpCommandEnvironment *environ);
    ~kpToolSelectionMoveCommand () override;

    kpAbstractSelection *originalSelectionClone () const;

    kpCommandSize::SizeType size () const override;

    void execute () override;
    void unexecute () override;

    void moveTo (const QPoint &point, bool moveLater = false);
    void moveTo (int x, int y, bool moveLater = false);
    void copyOntoDocument ();
    void finalize ();

private:
    QPoint m_startPoint, m_endPoint;

    kpImage m_oldDocumentImage;

    // area of document affected (not the bounding rect of the sel)
    QRect m_documentBoundingRect;

    QPolygon m_copyOntoDocumentPoints;
};


#endif  // kpToolSelectionMoveCommand_H
