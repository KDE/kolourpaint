
/*
   Copyright (c) 2003-2006 Clarence Dang <dang@kde.org>
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


#ifndef KP_TOOL_FLOOD_FILL_H
#define KP_TOOL_FLOOD_FILL_H


#include <kpcommandhistory.h>
#include <kpfloodfill.h>
#include <kptool.h>


class QString;

class kpColor;
class kpMainWindow;
class kpToolFloodFillCommand;


struct kpToolFloodFillPrivate;

class kpToolFloodFill : public kpTool
{
Q_OBJECT

public:
    kpToolFloodFill (kpMainWindow *);
    virtual ~kpToolFloodFill ();

private:
    QString haventBegunDrawUserMessage () const;

public:
    virtual void begin ();
    virtual void beginDraw ();
    virtual void draw (const QPoint &thisPoint, const QPoint &, const QRect &);
    virtual void cancelShape ();
    virtual void releasedAllButtons ();
    virtual void endDraw (const QPoint &, const QRect &);

private:
    kpToolFloodFillPrivate * const d;
};


struct kpToolFloodFillCommandPrivate;

class kpToolFloodFillCommand : public kpCommand, public kpFloodFill
{
public:
    kpToolFloodFillCommand (int x, int y,
                            const kpColor &color, int processedColorSimilarity,
                            kpMainWindow *mainWindow);
    virtual ~kpToolFloodFillCommand ();
    
    virtual QString name () const;

    virtual int size () const;

    // Optimisation hack: filling a fresh, unmodified document does not require
    //                    reading any pixels - just set the whole document to
    //                    <color>.
    void setFillEntirePixmap (bool yes = true);

    virtual void execute ();
    virtual void unexecute ();

private:
    kpToolFloodFillCommandPrivate * const d;
};


#endif  // KP_TOOL_FLOOD_FILL_H
