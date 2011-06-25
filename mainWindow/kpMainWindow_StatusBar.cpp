/*
   Copyright (c) 2003-2007 Clarence Dang <dang@kde.org>
   Copyright (c) 2011 Martin Koller <kollix@aon.at>
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

#define DEBUG_STATUS_BAR (DEBUG_KP_MAIN_WINDOW && 0)


#include <kpMainWindow.h>
#include <kpMainWindowPrivate.h>

#include <qlabel.h>
#include <qstring.h>

#include <kdebug.h>
#include <klocale.h>
#include <kstatusbar.h>
#include <KSqueezedTextLabel>

#include <kpDefs.h>
#include <kpDocument.h>
#include <kpTool.h>
#include <kpViewManager.h>
#include <kpViewScrollableContainer.h>
#include <kpZoomedView.h>

//---------------------------------------------------------------------

// private
void kpMainWindow::addPermanentStatusBarItem (int id, int maxTextLen)
{
    KStatusBar *sb = statusBar ();

    QString textWithMaxLen;
    textWithMaxLen.fill (QString::number (8/*big fat*/).at (0),
                         maxTextLen); //+ 2/*spaces on either side*/);

    // Permanent --> place on the right
    sb->insertPermanentFixedItem (textWithMaxLen, id);
    sb->changeItem (QString(), id);
}

//---------------------------------------------------------------------

// private
void kpMainWindow::createStatusBar ()
{
    KStatusBar *sb = statusBar ();

    // 9999 pixels "ought to be enough for anybody"
    const int maxDimenLength = 4;

    d->statusBarMessageLabel = new KSqueezedTextLabel(sb);
    // this is done to have the same height as the other labels in status bar; done like in kstatusbar.cpp
    d->statusBarMessageLabel->setFixedHeight(d->statusBarMessageLabel->fontMetrics().height() + 2);
    d->statusBarMessageLabel->setTextElideMode(Qt::ElideRight);  // this is the reason why we explicitly set a widget
    sb->addWidget(d->statusBarMessageLabel, 1/*stretch*/);

    addPermanentStatusBarItem (StatusBarItemShapePoints,
                               (maxDimenLength + 1/*,*/ + maxDimenLength) * 2 + 3/* - */);
    addPermanentStatusBarItem (StatusBarItemShapeSize,
                               (1/*+/-*/ + maxDimenLength) * 2 + 1/*x*/);

    QString numSample = i18n("%1 x %2", 5000, 5000);  // localized string; can e.g. be "5 000"
    addPermanentStatusBarItem(StatusBarItemDocSize, numSample.length());

    addPermanentStatusBarItem(StatusBarItemDocDepth, 5/*XXbpp*/);

    addPermanentStatusBarItem (StatusBarItemZoom,
                               5/*1600%*/);

    d->statusBarShapeLastPointsInitialised = false;
    d->statusBarShapeLastSizeInitialised = false;
    d->statusBarCreated = true;
}

//---------------------------------------------------------------------

// private slot
void kpMainWindow::setStatusBarMessage (const QString &message)
{
#if DEBUG_STATUS_BAR && 1
    kDebug () << "kpMainWindow::setStatusBarMessage("
               << message
               << ") ok=" << d->statusBarCreated
               << endl;
#endif

    if (!d->statusBarCreated)
        return;

    d->statusBarMessageLabel->setText (message);
}

//---------------------------------------------------------------------

// private slot
void kpMainWindow::setStatusBarShapePoints (const QPoint &startPoint,
                                            const QPoint &endPoint)
{
#if DEBUG_STATUS_BAR && 0
    kDebug () << "kpMainWindow::setStatusBarShapePoints("
               << startPoint << "," << endPoint
               << ") ok=" << d->statusBarCreated
               << endl;
#endif

    if (!d->statusBarCreated)
        return;

    if (d->statusBarShapeLastPointsInitialised &&
        startPoint == d->statusBarShapeLastStartPoint &&
        endPoint == d->statusBarShapeLastEndPoint)
    {
    #if DEBUG_STATUS_BAR && 0
        kDebug () << "\tNOP";
    #endif
        return;
    }

    if (startPoint == KP_INVALID_POINT)
    {
        statusBar ()->changeItem (QString(), StatusBarItemShapePoints);
    }
    else if (endPoint == KP_INVALID_POINT)
    {
        statusBar ()->changeItem (i18n ("%1,%2",
                                      startPoint.x (),
                                      startPoint.y ()),
                                  StatusBarItemShapePoints);
    }
    else
    {
        statusBar ()->changeItem (i18n  ("%1,%2 - %3,%4",
                                      startPoint.x (),
                                      startPoint.y (),
                                      endPoint.x (),
                                      endPoint.y ()),
                                  StatusBarItemShapePoints);
    }

    d->statusBarShapeLastStartPoint = startPoint;
    d->statusBarShapeLastEndPoint = endPoint;
    d->statusBarShapeLastPointsInitialised = true;
}

//---------------------------------------------------------------------

// private slot
void kpMainWindow::setStatusBarShapeSize (const QSize &size)
{
#if DEBUG_STATUS_BAR && 0
    kDebug () << "kpMainWindow::setStatusBarShapeSize("
               << size
               << ") ok=" << d->statusBarCreated
               << endl;
#endif

    if (!d->statusBarCreated)
        return;

    if (d->statusBarShapeLastSizeInitialised &&
        size == d->statusBarShapeLastSize)
    {
    #if DEBUG_STATUS_BAR && 0
        kDebug () << "\tNOP";
    #endif
        return;
    }

    if (size == KP_INVALID_SIZE)
    {
        statusBar ()->changeItem (QString(), StatusBarItemShapeSize);
    }
    else
    {
        statusBar ()->changeItem (i18n ("%1x%2",
                                      size.width (),
                                      size.height ()),
                                  StatusBarItemShapeSize);
    }

    d->statusBarShapeLastSize = size;
    d->statusBarShapeLastSizeInitialised = true;
}

//---------------------------------------------------------------------

// private slot
void kpMainWindow::setStatusBarDocSize (const QSize &size)
{
#if DEBUG_STATUS_BAR && 0
    kDebug () << "kpMainWindow::setStatusBarDocSize("
               << size
               << ") ok=" << d->statusBarCreated
               << endl;
#endif

    if (!d->statusBarCreated)
        return;

    if (size == KP_INVALID_SIZE)
    {
        statusBar ()->changeItem (QString(), StatusBarItemDocSize);
    }
    else
    {
        statusBar ()->changeItem (i18n ("%1 x %2",
                                      size.width (),
                                      size.height ()),
                                  StatusBarItemDocSize);
    }
}

//---------------------------------------------------------------------

// private slot
void kpMainWindow::setStatusBarDocDepth (int depth)
{
#if DEBUG_STATUS_BAR && 0
    kDebug () << "kpMainWindow::setStatusBarDocDepth("
               << depth
               << ") ok=" << d->statusBarCreated
               << endl;
#endif

    if (!d->statusBarCreated)
        return;

    if (depth <= 0)
    {
        statusBar ()->changeItem (QString(), StatusBarItemDocDepth);
    }
    else
    {
        statusBar ()->changeItem (i18n ("%1bpp", depth),
                                  StatusBarItemDocDepth);
    }
}

//---------------------------------------------------------------------

// private slot
void kpMainWindow::setStatusBarZoom (int zoom)
{
#if DEBUG_STATUS_BAR && 0
    kDebug () << "kpMainWindow::setStatusBarZoom("
               << zoom
               << ") ok=" << d->statusBarCreated
               << endl;
#endif

    if (!d->statusBarCreated)
        return;

    if (zoom <= 0)
    {
        statusBar ()->changeItem (QString(), StatusBarItemZoom);
    }
    else
    {
        statusBar ()->changeItem (i18n ("%1%", zoom),
                                  StatusBarItemZoom);
    }
}

//---------------------------------------------------------------------

void kpMainWindow::recalculateStatusBarMessage ()
{
#if DEBUG_STATUS_BAR && 1
    kDebug () << "kpMainWindow::recalculateStatusBarMessage()";
#endif
    QString scrollViewMessage = d->scrollView->statusMessage ();
#if DEBUG_STATUS_BAR && 1
    kDebug () << "\tscrollViewMessage=" << scrollViewMessage;
    kDebug () << "\tresizing doc? " << !d->scrollView->newDocSize ().isEmpty ()
               << endl;
    kDebug () << "\tviewUnderCursor? "
               << (d->viewManager && d->viewManager->viewUnderCursor ())
               << endl;
#endif

    // HACK: To work around kpViewScrollableContainer's unreliable
    //       status messages (which in turn is due to Qt not updating
    //       QWidget::underMouse() on drags and we needing to hack around it)
    if (!scrollViewMessage.isEmpty () &&
        d->scrollView->newDocSize ().isEmpty () &&
        d->viewManager && d->viewManager->viewUnderCursor ())
    {
    #if DEBUG_STATUS_BAR && 1
        kDebug () << "\t\tnot resizing & viewUnderCursor - message is wrong - clearing"
                   << endl;
    #endif
        d->scrollView->blockSignals (true);
        d->scrollView->clearStatusMessage ();
        d->scrollView->blockSignals (false);

        scrollViewMessage.clear ();
    #if DEBUG_STATUS_BAR && 1
        kDebug () << "\t\t\tdone";
    #endif
    }

    if (!scrollViewMessage.isEmpty ())
    {
        setStatusBarMessage (scrollViewMessage);
    }
    else
    {
        const kpTool *t = tool ();
        if (t)
        {
            setStatusBarMessage (t->userMessage ());
        }
        else
        {
            setStatusBarMessage ();
        }
    }
}

//---------------------------------------------------------------------

// private slot
void kpMainWindow::recalculateStatusBarShape ()
{
#if DEBUG_STATUS_BAR && 0
    kDebug () << "kpMainWindow::recalculateStatusBarShape()";
#endif

    QSize docResizeTo = d->scrollView->newDocSize ();
#if DEBUG_STATUS_BAR && 0
    kDebug () << "\tdocResizeTo=" << docResizeTo;
#endif
    if (docResizeTo.isValid ())
    {
        const QPoint startPoint (d->document->width (), d->document->height ());
    #if DEBUG_STATUS_BAR && 0
        kDebug () << "\thavedMovedFromOrgSize="
                   << d->scrollView->haveMovedFromOriginalDocSize () << endl;
    #endif
        if (!d->scrollView->haveMovedFromOriginalDocSize ())
        {
            setStatusBarShapePoints (startPoint);
            setStatusBarShapeSize ();
        }
        else
        {
            const int newWidth = docResizeTo.width ();
            const int newHeight = docResizeTo.height ();

            setStatusBarShapePoints (startPoint, QPoint (newWidth, newHeight));
            const QPoint sizeAsPoint (QPoint (newWidth, newHeight) - startPoint);
            setStatusBarShapeSize (QSize (sizeAsPoint.x (), sizeAsPoint.y ()));
        }
    }
    else
    {
        const kpTool *t = tool ();
    #if DEBUG_STATUS_BAR && 0
        kDebug () << "\ttool=" << t;
    #endif
        if (t)
        {
            setStatusBarShapePoints (t->userShapeStartPoint (),
                                     t->userShapeEndPoint ());
            setStatusBarShapeSize (t->userShapeSize ());
        }
        else
        {
            setStatusBarShapePoints ();
            setStatusBarShapeSize ();
        }
    }
}

//---------------------------------------------------------------------

// private slot
void kpMainWindow::recalculateStatusBar ()
{
#if DEBUG_STATUS_BAR && 1
    kDebug () << "kpMainWindow::recalculateStatusBar() ok="
               << d->statusBarCreated
               << endl;
#endif

    if (!d->statusBarCreated)
        return;

    recalculateStatusBarMessage ();
    recalculateStatusBarShape ();

    if (d->document)
    {
        setStatusBarDocSize (QSize (d->document->width (), d->document->height ()));
        setStatusBarDocDepth (d->document->image ().depth ());
    }
    else
    {
        setStatusBarDocSize ();
        setStatusBarDocDepth ();
    }

    if (d->mainView)
    {
        setStatusBarZoom (d->mainView->zoomLevelX ());
    }
    else
    {
        setStatusBarZoom ();
    }
}

//---------------------------------------------------------------------
