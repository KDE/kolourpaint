
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
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
    enum RenderMode {
        SetImage,
        PaintImage,
        UserFunction
    };

    // REFACTOR: Function pointers imply a need for a proper class hierarchy.
    typedef void (*UserFunctionType)(kpImage * /*destImage*/, const QPoint & /*topLeft*/, void * /*userData*/);

    /*
     * <isBrush>    Specifies that its visibility is dependent on whether
     *              the mouse cursor is inside a view.  If false, the
     *              image is always displayed.
     *
     * <userFunction>   This is the only way of specifying the "UserFunction"
     *                  <renderMode>.  <userFunction> must not draw outside
     *                  the claimed rectangle.
     */
    kpTempImage(bool isBrush, RenderMode renderMode, const QPoint &topLeft, const kpImage &image);
    kpTempImage(bool isBrush, const QPoint &topLeft, UserFunctionType userFunction, void *userData, int width, int height);
    kpTempImage(const kpTempImage &rhs);
    kpTempImage &operator=(const kpTempImage &rhs);

    bool isBrush() const;
    RenderMode renderMode() const;
    QPoint topLeft() const;
    kpImage image() const;
    UserFunctionType userFunction() const;
    void *userData() const;

    bool isVisible(const kpViewManager *vm) const;
    QRect rect() const;
    int width() const;
    int height() const;

    // Returns whether a call to paint() may add a mask to <*destImage>.
    bool paintMayAddMask() const;

    /*
     * Draws itself onto <*destImage>, given that <*destImage> represents
     * the unzoomed <docRect> of the kpDocument.  You should check for
     * visibility before calling this function.
     */
    void paint(kpImage *destImage, const QRect &docRect) const;

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

#endif // kpTempImage_H
