
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


#include "layers/tempImage/kpTempImage.h"

#include "pixmapfx/kpPixmapFX.h"
#include "views/manager/kpViewManager.h"

#include <QPainter>

//---------------------------------------------------------------------

kpTempImage::kpTempImage (bool isBrush, RenderMode renderMode,
        const QPoint &topLeft, const kpImage &image)
    : m_isBrush (isBrush),
      m_renderMode (renderMode),
      m_topLeft (topLeft),
      m_image (image),
      m_width (image.width ()), m_height (image.height ()),
      m_userFunction (nullptr),
      m_userData (nullptr)
{
    // Use below constructor for that.
    Q_ASSERT (renderMode != UserFunction);
}

//---------------------------------------------------------------------

kpTempImage::kpTempImage (bool isBrush, const QPoint &topLeft,
        UserFunctionType userFunction, void *userData,
        int width, int height)
    : m_isBrush (isBrush),
      m_renderMode (UserFunction),
      m_topLeft (topLeft),
      m_width (width), m_height (height),
      m_userFunction (userFunction),
      m_userData (userData)
{
    Q_ASSERT (m_userFunction);
}

//---------------------------------------------------------------------

kpTempImage::kpTempImage (const kpTempImage &rhs)
    : m_isBrush (rhs.m_isBrush),
      m_renderMode (rhs.m_renderMode),
      m_topLeft (rhs.m_topLeft),
      m_image (rhs.m_image),
      m_width (rhs.m_width), m_height (rhs.m_height),
      m_userFunction (rhs.m_userFunction),
      m_userData (rhs.m_userData)
{
}

//---------------------------------------------------------------------

kpTempImage &kpTempImage::operator= (const kpTempImage &rhs)
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
bool kpTempImage::isBrush () const
{
    return m_isBrush;
}

//---------------------------------------------------------------------

// public
kpTempImage::RenderMode kpTempImage::renderMode () const
{
    return m_renderMode;
}

//---------------------------------------------------------------------

// public
QPoint kpTempImage::topLeft () const
{
    return m_topLeft;
}

//---------------------------------------------------------------------

// public
kpImage kpTempImage::image () const
{
    return m_image;
}

//---------------------------------------------------------------------

// public
kpTempImage::UserFunctionType kpTempImage::userFunction () const
{
    return m_userFunction;
}

//---------------------------------------------------------------------

// public
void *kpTempImage::userData () const
{
    return m_userData;
}

//---------------------------------------------------------------------

// public
bool kpTempImage::isVisible (const kpViewManager *vm) const
{
    return m_isBrush ? static_cast<bool> (vm->viewUnderCursor ()) : true;
}

//---------------------------------------------------------------------

// public
QRect kpTempImage::rect () const
{
    return  {m_topLeft.x (), m_topLeft.y (), m_width, m_height};
}

//---------------------------------------------------------------------

// public
int kpTempImage::width () const
{
    return m_width;
}

//---------------------------------------------------------------------

// public
int kpTempImage::height () const
{
    return m_height;
}

//---------------------------------------------------------------------

// public
bool kpTempImage::paintMayAddMask () const
{
    return (m_renderMode == SetImage ||
            m_renderMode == UserFunction);
}

//---------------------------------------------------------------------

// public
void kpTempImage::paint (kpImage *destImage, const QRect &docRect) const
{
    const QPoint REL_TOP_LEFT = m_topLeft - docRect.topLeft ();

    switch (m_renderMode)
    {
      case SetImage:
      {
        kpPixmapFX::setPixmapAt(destImage, REL_TOP_LEFT, m_image);
        break;
      }

      case PaintImage:
      {
        kpPixmapFX::paintPixmapAt(destImage, REL_TOP_LEFT, m_image);
        break;
      }

      case UserFunction:
      {
        m_userFunction(destImage, REL_TOP_LEFT, m_userData);
        break;
      }
    }
}

//---------------------------------------------------------------------
