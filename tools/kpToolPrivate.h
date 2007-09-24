
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


#ifndef kpToolPrivate_H
#define kpToolPrivate_H


#include <QPoint>
#include <QPointer>

#include <QSize>
#ifdef Q_OS_WIN
  #include <stdlib.h>
  #undef environ  // macro on win32
#endif

class kpToolAction;
class kpToolEnvironment;


struct kpToolPrivate
{
    // Initialisation / properties.
    QString text;
    QString description;
    int key;

    kpToolAction *action;

    // Drawing state.
    bool began;
    bool beganDraw;  // set after beginDraw() is called, unset before endDraw() is called
    int mouseButton;  /* 0 = left, 1 = right */
    bool shiftPressed, controlPressed, altPressed;  // m_altPressed is unreliable
    QPoint startPoint,
           currentPoint, currentViewPoint,
           lastPoint;

    kpView *viewUnderStartPoint;


    // Set to 2 when the user swaps the foreground and background color.
    //
    // When nonzero, it suppresses the foreground and background "color changed"
    // signals and is decremented back down to 0 separately by the foreground
    // code and background code.
    int ignoreColorSignals;

    // Statusbar.
    QString userMessage;
    QPoint userShapeStartPoint, userShapeEndPoint;
    QSize userShapeSize;

    kpToolEnvironment *environ;
};


#endif  // kpToolPrivate_H
