
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


#ifndef KP_TOOL_RECTANGLE_H
#define KP_TOOL_RECTANGLE_H


#include <qbrush.h>
#include <qpen.h>
#include <qpixmap.h>
#include <qpoint.h>
#include <qrect.h>

#include <kpcommandhistory.h>
#include <kptool.h>


class QString;

class kpColor;
class kpMainWindow;
class kpToolWidgetFillStyle;
class kpToolWidgetLineWidth;
class kpViewManager;


struct kpToolRectanglePrivate;

class kpToolRectangle : public kpTool
{
Q_OBJECT

public:
    // it turns out that these shapes are all really the same thing
    // (same options, same feel) - the only real difference is the
    // drawing functions (a one line change)
    enum Mode {Rectangle, RoundedRectangle, Ellipse};

    kpToolRectangle (Mode mode,
                     const QString &text, const QString &description,
                     int key,
                     kpMainWindow *mainWindow,
                     const char *name);
    kpToolRectangle (kpMainWindow *);
    virtual ~kpToolRectangle ();

    void setMode (Mode mode);

    virtual bool careAboutModifierState () const { return true; }

private slots:
    virtual void slotLineWidthChanged ();
    virtual void slotFillStyleChanged ();

private:
    QString haventBegunDrawUserMessage () const;

public:
    virtual void begin ();
    virtual void end ();

private:
    void applyModifiers ();
    virtual void beginDraw ();
private:
    kpColor drawingForegroundColor () const;
    kpColor drawingBackgroundColor () const;
    void updateShape ();
public:
    virtual void draw (const QPoint &, const QPoint &, const QRect &);
    virtual void cancelShape ();
    virtual void releasedAllButtons ();
    virtual void endDraw (const QPoint &, const QRect &);

private:
    kpToolRectanglePrivate * const d;
};


struct kpToolRectangleCommandPrivate;

class kpToolRectangleCommand : public kpCommand
{
public:
    kpToolRectangleCommand (kpToolRectangle::Mode mode,
        const QRect &rect,
        const kpColor &fcolor, int penWidth,
        const kpColor &bcolor,
        kpMainWindow *mainWindow);
    virtual ~kpToolRectangleCommand ();

    virtual QString name () const;

    virtual int size () const;

    virtual void execute ();
    virtual void unexecute ();

private:
    kpToolRectangleCommandPrivate * const d;
    kpToolRectangleCommand &operator= (const kpToolRectangleCommand &) const;
};


#endif  // KP_TOOL_RECTANGLE_H
