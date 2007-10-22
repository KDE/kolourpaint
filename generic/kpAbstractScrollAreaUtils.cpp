
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


#define DEBUG_KP_SCROLL_AREA_UTILS 0


#include <kpAbstractScrollAreaUtils.h>

#include <Q3ScrollView>
#include <QAbstractScrollArea>
#include <QScrollBar>

#include <KDebug>


static int EstimateVerticalScrollBarWidth (QScrollBar *sb)
{
    int scrollBarAdjust = sb ?
        sb->sizeHint ().width () :
        0;
#if DEBUG_KP_SCROLL_AREA_UTILS
    kDebug () << "verticalScrollBar=" << sb
              << " sizeHint=" << scrollBarAdjust;
#endif

    if (scrollBarAdjust <= 0)
    {
        kError () << "verticalScrollBar sizeHint of" << scrollBarAdjust
                  << "is invalid.  Qt's behavior changed so find another"
                  << "way to get the scrollbar size.";

        // Should be big enough for most styles.
        scrollBarAdjust = 20;
    }

    return scrollBarAdjust;
}

// public static
int kpAbstractScrollAreaUtils::EstimateVerticalScrollBarWidth (
        QAbstractScrollArea *scrollArea)
{
    return ::EstimateVerticalScrollBarWidth (scrollArea->verticalScrollBar ());
}

// public static
int kpAbstractScrollAreaUtils::EstimateVerticalScrollBarWidth (
        Q3ScrollView *scrollView)
{
    return ::EstimateVerticalScrollBarWidth (scrollView->verticalScrollBar ());
}


static int EstimateHorizontalScrollBarHeight (QScrollBar *sb)
{
    int scrollBarAdjust = sb ?
        sb->sizeHint ().height () :
        0;
#if DEBUG_KP_SCROLL_AREA_UTILS
    kDebug () << "horizontalScrollBar=" << sb
              << " sizeHint=" << scrollBarAdjust;
#endif

    if (scrollBarAdjust <= 0)
    {
        kError () << "horizontalScrollBar sizeHint of" << scrollBarAdjust
                  << "is invalid.  Qt's behavior changed so find another"
                  << "way to get the scrollbar size.";

        // Should be big enough for most styles.
        scrollBarAdjust = 20;
    }

    return scrollBarAdjust;
}

// public static
int kpAbstractScrollAreaUtils::EstimateHorizontalScrollBarHeight (
        QAbstractScrollArea *scrollArea)
{
    return ::EstimateHorizontalScrollBarHeight (scrollArea->horizontalScrollBar ());
}

// public static
int kpAbstractScrollAreaUtils::EstimateHorizontalScrollBarHeight (
        Q3ScrollView *scrollView)
{
    return ::EstimateHorizontalScrollBarHeight (scrollView->horizontalScrollBar ());
}


// public static
QSize kpAbstractScrollAreaUtils::EstimateUsableArea (QAbstractScrollArea *scrollArea)
{
    const int w = kpAbstractScrollAreaUtils::EstimateVerticalScrollBarWidth (scrollArea),
              h = kpAbstractScrollAreaUtils::EstimateHorizontalScrollBarHeight (scrollArea);
    return QSize (scrollArea->width () - w - scrollArea->frameWidth () * 2,
                  scrollArea->height () - h - scrollArea->frameWidth () * 2);
}

// public static
QSize kpAbstractScrollAreaUtils::EstimateUsableArea (Q3ScrollView *scrollView)
{
    const int w = kpAbstractScrollAreaUtils::EstimateVerticalScrollBarWidth (scrollView),
              h = kpAbstractScrollAreaUtils::EstimateHorizontalScrollBarHeight (scrollView);
    return QSize (scrollView->width () - w - scrollView->frameWidth () * 2,
                  scrollView->height () - h - scrollView->frameWidth () * 2);
}
