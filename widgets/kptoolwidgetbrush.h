
/*
   Copyright (c) 2003,2004,2005 Clarence Dang <dang@kde.org>
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


#ifndef __kptoolwidgetbrush_h__
#define __kptoolwidgetbrush_h__

#include <qpixmap.h>

#include <kptoolwidgetbase.h>

class kpToolWidgetBrush : public kpToolWidgetBase
{
Q_OBJECT

public:
    kpToolWidgetBrush (QWidget *parent, const char *name);
    virtual ~kpToolWidgetBrush ();

private:
    QString brushName (int shape, int whichSize);
    
public:
    QPixmap brush () const;
    bool brushIsDiagonalLine () const;

signals:
    void brushChanged (const QPixmap &pixmap, bool isDiagonalLine);

protected slots:
    virtual bool setSelected (int row, int col, bool saveAsDefault);

private:
    QPixmap m_brushBitmaps [16];
};

#endif  // __kptoolwidgetbrush_h__
