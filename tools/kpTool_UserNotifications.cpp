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

//
// Tools' statusbar updates.
//

#include "tools/kpTool.h"
#include "kpToolPrivate.h"

#include <KLocalizedString>

//---------------------------------------------------------------------

// public static
QString kpTool::cancelUserMessage (int mouseButton)
{
    if (mouseButton == 0) {
        return i18n ("Right click to cancel.");
    }

    return i18n ("Left click to cancel.");
}

//---------------------------------------------------------------------

// public
QString kpTool::cancelUserMessage () const
{
    return cancelUserMessage (d->mouseButton);
}

//---------------------------------------------------------------------

// public
QString kpTool::userMessage () const
{
    return d->userMessage;
}

//---------------------------------------------------------------------

// public
void kpTool::setUserMessage (const QString &userMessage)
{
    d->userMessage = userMessage;

    if (d->userMessage.isEmpty ()) {
        d->userMessage = text ();
    }
    else {
        d->userMessage.prepend (i18n ("%1: ", text ()));
    }

    emit userMessageChanged (d->userMessage);
}

//---------------------------------------------------------------------

// public
QPoint kpTool::userShapeStartPoint () const
{
    return d->userShapeStartPoint;
}

//---------------------------------------------------------------------

// public
QPoint kpTool::userShapeEndPoint () const
{
    return d->userShapeEndPoint;
}

//---------------------------------------------------------------------

// public
void kpTool::setUserShapePoints (const QPoint &startPoint,
                                 const QPoint &endPoint,
                                 bool setSize)
{
    d->userShapeStartPoint = startPoint;
    d->userShapeEndPoint = endPoint;
    emit userShapePointsChanged (d->userShapeStartPoint, d->userShapeEndPoint);

    if (setSize)
    {
        if (startPoint != KP_INVALID_POINT &&
            endPoint != KP_INVALID_POINT)
        {
            setUserShapeSize (calculateLength (startPoint.x (), endPoint.x ()),
                              calculateLength (startPoint.y (), endPoint.y ()));
        }
        else
        {
            setUserShapeSize ();
        }
    }
}

//---------------------------------------------------------------------

// public
QSize kpTool::userShapeSize () const
{
    return d->userShapeSize;
}

//---------------------------------------------------------------------

// public
int kpTool::userShapeWidth () const
{
    return d->userShapeSize.width ();
}

//---------------------------------------------------------------------

// public
int kpTool::userShapeHeight () const
{
    return d->userShapeSize.height ();
}

//---------------------------------------------------------------------

// public
void kpTool::setUserShapeSize (const QSize &size)
{
    d->userShapeSize = size;

    emit userShapeSizeChanged (d->userShapeSize);
}

//---------------------------------------------------------------------

// public
void kpTool::setUserShapeSize (int width, int height)
{
    setUserShapeSize (QSize (width, height));
}

//---------------------------------------------------------------------
