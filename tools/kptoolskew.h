
/* This file is part of the KolourPaint project
   Copyright (c) 2003 Clarence Dang <dang@kde.org>
   All rights reserved.
   
   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:
   
   1. Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
   2. Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
   3. Neither the names of the copyright holders nor the names of
      contributors may be used to endorse or promote products derived from
      this software without specific prior written permission.
   
   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
   PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
   HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef __kptool_skew_h__
#define __kptool_skew_h__

#include <qcolor.h>

#include <kcommand.h>
#include <kdialogbase.h>

class QPixmap;

class KIntNumInput;

class kpDocument;
class kpToolSkewDialogWidget;
class kpViewManager;

class kpToolSkewCommand : public KCommand
{
public:
    kpToolSkewCommand (kpDocument *document, kpViewManager *m_viewManager,
                       double hangle, double vangle, const QColor &backgroundColor);
    virtual QString name () const;
    virtual ~kpToolSkewCommand ();

    virtual void execute ();
    virtual void unexecute ();

private:
    kpDocument *m_document;
    kpViewManager *m_viewManager;
    double m_hangle, m_vangle;
    QColor m_backgroundColor;
    QPixmap *m_oldPixmapPtr;
};

class kpToolSkewDialog : public KDialogBase
{
public:
    kpToolSkewDialog (QWidget *parent);
    virtual ~kpToolSkewDialog ();

    double horizontalAngle () const;
    double verticalAngle () const;

    bool isNoop () const;

private:
    kpToolSkewDialogWidget *m_mainWidget;
};

#endif  // __kptool_skew_h__
