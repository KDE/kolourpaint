
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


#ifndef KP_TOOL_RECTANGULAR_BASE_H
#define KP_TOOL_RECTANGULAR_BASE_H


#include "imagelib/kpImage.h"
#include "tools/kpTool.h"


class QPoint;
class QRect;
class QString;

class kpColor;


struct kpToolRectangularBasePrivate;


// it turns out that these shapes are all really the same thing
// (same options, same feel) - the only real difference is the
// drawing function i.e. drawShape().
class kpToolRectangularBase : public kpTool
{
Q_OBJECT

public:
    typedef void (*DrawShapeFunc) (kpImage * /*image*/,
        int /*x*/, int /*y*/, int /*width*/, int /*height*/,
        const kpColor &/*fcolor*/, int /*penWidth = 1*/,
        const kpColor &/*bcolor = kpColor::Invalid*/);

    kpToolRectangularBase (const QString &text, const QString &description,
        DrawShapeFunc drawShapeFunc,
        int key,
        kpToolEnvironment *environ, QObject *parent,
        const QString &name);

    ~kpToolRectangularBase () override;

    bool careAboutModifierState () const override { return true; }

private slots:
    virtual void slotLineWidthChanged ();
    virtual void slotFillStyleChanged ();

private:
    QString haventBegunDrawUserMessage () const;

public:
    void begin () override;
    void end () override;

private:
    void applyModifiers ();
    void beginDraw () override;
private:
    kpColor drawingForegroundColor () const;
    kpColor drawingBackgroundColor () const;
    void updateShape ();
public:
    void draw (const QPoint &, const QPoint &, const QRect &) override;
    void cancelShape () override;
    void releasedAllButtons () override;
    void endDraw (const QPoint &, const QRect &) override;

private:
    kpToolRectangularBasePrivate * const d;
};


#endif  // KP_TOOL_RECTANGULAR_BASE_H
