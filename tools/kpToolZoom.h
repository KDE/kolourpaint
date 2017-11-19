
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


#ifndef KP_TOOL_ZOOM_H
#define KP_TOOL_ZOOM_H


#include "tools/kpTool.h"


class kpToolZoom : public kpTool
{
Q_OBJECT

public:
    kpToolZoom (kpToolEnvironment *environ, QWidget *parent);
    ~kpToolZoom () override;

    bool returnToPreviousToolAfterEndDraw () const override;

private:
    QString haventBegunDrawUserMessage () const;

public:
    void begin () override;
    void end () override;

    void globalDraw () override;

    void beginDraw () override;
    void draw (const QPoint &thisPoint, const QPoint &, const QRect &) override;
    void cancelShape () override;
    void releasedAllButtons () override;
    void endDraw (const QPoint &thisPoint, const QRect &) override;

private:
    struct kpToolZoomPrivate *d;
};


#endif  // KP_TOOL_ZOOM_H
