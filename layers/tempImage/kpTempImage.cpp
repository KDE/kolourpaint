
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#include "layers/tempImage/kpTempImage.h"

#include "pixmapfx/kpPixmapFX.h"
#include "views/manager/kpViewManager.h"

//---------------------------------------------------------------------

kpTempImage::kpTempImage(bool isBrush, RenderMode renderMode, const QPoint &topLeft, const kpImage &image)
    : m_isBrush(isBrush)
    , m_renderMode(renderMode)
    , m_topLeft(topLeft)
    , m_image(image)
    , m_width(image.width())
    , m_height(image.height())
    , m_userFunction(nullptr)
    , m_userData(nullptr)
{
    // Use below constructor for that.
    Q_ASSERT(renderMode != UserFunction);
}

//---------------------------------------------------------------------

kpTempImage::kpTempImage(bool isBrush, const QPoint &topLeft, UserFunctionType userFunction, void *userData, int width, int height)
    : m_isBrush(isBrush)
    , m_renderMode(UserFunction)
    , m_topLeft(topLeft)
    , m_width(width)
    , m_height(height)
    , m_userFunction(userFunction)
    , m_userData(userData)
{
    Q_ASSERT(m_userFunction);
}

//---------------------------------------------------------------------

kpTempImage::kpTempImage(const kpTempImage &rhs)
    : m_isBrush(rhs.m_isBrush)
    , m_renderMode(rhs.m_renderMode)
    , m_topLeft(rhs.m_topLeft)
    , m_image(rhs.m_image)
    , m_width(rhs.m_width)
    , m_height(rhs.m_height)
    , m_userFunction(rhs.m_userFunction)
    , m_userData(rhs.m_userData)
{
}

//---------------------------------------------------------------------

kpTempImage &kpTempImage::operator=(const kpTempImage &rhs)
{
    if (this == &rhs) {
        return *this;
    }

    m_isBrush = rhs.m_isBrush;
    m_renderMode = rhs.m_renderMode;
    m_topLeft = rhs.m_topLeft;
    m_image = rhs.m_image;
    m_width = rhs.m_width;
    m_height = rhs.m_height;
    m_userFunction = rhs.m_userFunction;
    m_userData = rhs.m_userData;

    return *this;
}

//---------------------------------------------------------------------

// public
bool kpTempImage::isBrush() const
{
    return m_isBrush;
}

//---------------------------------------------------------------------

// public
kpTempImage::RenderMode kpTempImage::renderMode() const
{
    return m_renderMode;
}

//---------------------------------------------------------------------

// public
QPoint kpTempImage::topLeft() const
{
    return m_topLeft;
}

//---------------------------------------------------------------------

// public
kpImage kpTempImage::image() const
{
    return m_image;
}

//---------------------------------------------------------------------

// public
kpTempImage::UserFunctionType kpTempImage::userFunction() const
{
    return m_userFunction;
}

//---------------------------------------------------------------------

// public
void *kpTempImage::userData() const
{
    return m_userData;
}

//---------------------------------------------------------------------

// public
bool kpTempImage::isVisible(const kpViewManager *vm) const
{
    return m_isBrush ? static_cast<bool>(vm->viewUnderCursor()) : true;
}

//---------------------------------------------------------------------

// public
QRect kpTempImage::rect() const
{
    return {m_topLeft.x(), m_topLeft.y(), m_width, m_height};
}

//---------------------------------------------------------------------

// public
int kpTempImage::width() const
{
    return m_width;
}

//---------------------------------------------------------------------

// public
int kpTempImage::height() const
{
    return m_height;
}

//---------------------------------------------------------------------

// public
bool kpTempImage::paintMayAddMask() const
{
    return (m_renderMode == SetImage || m_renderMode == UserFunction);
}

//---------------------------------------------------------------------

// public
void kpTempImage::paint(kpImage *destImage, const QRect &docRect) const
{
    const QPoint REL_TOP_LEFT = m_topLeft - docRect.topLeft();

    switch (m_renderMode) {
    case SetImage: {
        kpPixmapFX::setPixmapAt(destImage, REL_TOP_LEFT, m_image);
        break;
    }

    case PaintImage: {
        kpPixmapFX::paintPixmapAt(destImage, REL_TOP_LEFT, m_image);
        break;
    }

    case UserFunction: {
        m_userFunction(destImage, REL_TOP_LEFT, m_userData);
        break;
    }
    }
}

//---------------------------------------------------------------------
