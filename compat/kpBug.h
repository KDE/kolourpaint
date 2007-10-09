
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


#ifndef KP_BUG_H
#define KP_BUG_H


class QAbstractButton;
class QButtonGroup;
class QRect;


/**
 * @short Hacks around Qt and KDE library bugs (until they're fixed).
 *
 * SYNC: Remove hacks when fixed.
 *
 * @author Clarence Dang <dang@kde.org>
 */
class kpBug
{
public:
    // http://www.trolltech.com/developer/tasktracker.html?method=entry&id=108906:
    //
    // Qt 4.1.1 QButtonGroup::checkedButton() does not return 0 after a
    // button is unchecked.
    //
    //
    // QButtonGroup_CheckedButton() will return 0 correctly.
    static QAbstractButton *QButtonGroup_CheckedButton (const QButtonGroup *buttonGroup);

    // From: Clarence Dang <dang@kde.org>
    // To: qt-bugs@trolltech.com
    // Subject: Re: [Issue N108897] Dangerous, subtle change to QRect::normalize()
    // Date: Fri, 31 Mar 2006 20:51:36 +1000
    // Cc: kde-core-devel@kde.org
    //
    // Thread extracts:
    //
    // Qt 4.1.1 qt-copy/src/corelib/tools/qrect.cpp:
    //
    //     QRect QRect::normalized() const
    //     {
    //         if (isNull() || width() == 0 || height() == 0)
    //             return *this;
    //
    // This check was not in Qt3.  It is also wrong.
    //
    // e.g. QRect rect = QRect (thisPoint, lastPoint).normalize ()
    //
    // This innocent looking line will work for all values of "thisPoint" and
    // "lastPoint", _except_ for 5 special cases by my count:
    //
    //     +++
    //     +*
    //     +
    //
    // Where "*" is the topLeft point and "+" are the troublesome bottomRight
    // points.
    //
    //
    // QRect_Normalized() will use the Qt3 behaviour and therefore, work for
    // all input.
    static QRect QRect_Normalized (const QRect &rect);
};


#endif  // KP_BUG_H
