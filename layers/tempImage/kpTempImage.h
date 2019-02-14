
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

// REFACTOR: maybe make us and kpAbstractSelection share a new kpLayer base?


#ifndef kpTempImage_H
#define kpTempImage_H


#include <QPoint>

#include "imagelib/kpImage.h"


class kpViewManager;


class kpTempImage
{
public:
    // REFACTOR: Extract into class hierarchy.
    enum RenderMode
    {
        SetImage,
        PaintImage,
        UserFunction
    };

    // REFACTOR: Function pointers imply a need for a proper class hierarchy.
    typedef void (*UserFunctionType) (kpImage * /*destImage*/,
        const QPoint & /*topLeft*/,
        void * /*userData*/);

    /*
     * <isBrush>    Specifies that its visibility is dependent on whether or
     *              not the mouse cursor is inside a view.  If false, the
     *              image is always displayed.
     *
     * <userFunction>   This is the only way of specifying the "UserFunction"
     *                  <renderMode>.  <userFunction> must not draw outside
     *                  the claimed rectangle.
     */
    kpTempImage (bool isBrush, RenderMode renderMode, const QPoint &topLeft, const kpImage &image);
    kpTempImage (bool isBrush, const QPoint &topLeft,
        UserFunctionType userFunction, void *userData,
        int width, int height);
    kpTempImage (const kpTempImage &rhs);
    kpTempImage &operator= (const kpTempImage &rhs);

    bool isBrush () const;
    RenderMode renderMode () const;
    QPoint topLeft () const;
    kpImage image () const;
    UserFunctionType userFunction () const;
    void *userData () const;

    bool isVisible (const kpViewManager *vm) const;
    QRect rect () const;
    int width () const;
    int height () const;


    // Returns whether a call to paint() may add a mask to <*destImage>.
    bool paintMayAddMask () const;

    /*
     * Draws itself onto <*destImage>, given that <*destImage> represents
     * the unzoomed <docRect> of the kpDocument.  You should check for
     * visibility before calling this function.
     */
    void paint (kpImage *destImage, const QRect &docRect) const;

private:
    bool m_isBrush;
    RenderMode m_renderMode;
    QPoint m_topLeft;
    kpImage m_image;
    // == m_image.{width,height}() unless m_renderMode == UserFunction.
    int m_width, m_height;
    UserFunctionType m_userFunction;
    void *m_userData;
};


#endif  // kpTempImage_H
