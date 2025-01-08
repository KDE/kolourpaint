
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef kpToolPrivate_H
#define kpToolPrivate_H

#include <QPoint>

#include <QSize>
#ifdef Q_OS_WIN
#include <stdlib.h>
#undef environ // macro on win32
#endif

#include "views/kpView.h"

class kpToolEnvironment;
class KToggleAction;

struct kpToolPrivate {
    // Initialisation / properties.
    QString text;
    QString description;
    int key;

    KToggleAction *action;

    // Drawing state.
    bool began;
    bool beganDraw; // set after beginDraw() is called, unset before endDraw() is called
    int mouseButton; /* 0 = left, 1 = right */
    bool shiftPressed, controlPressed, altPressed; // m_altPressed is unreliable
    QPoint startPoint, currentPoint, currentViewPoint, lastPoint;

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

#endif // kpToolPrivate_H
