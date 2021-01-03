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


#include "mainWindow/kpMainWindow.h"
#include "kpMainWindowPrivate.h"

#include <QLabel>
#include <QStatusBar>
#include <QString>

#include "kpLogCategories.h"
#include "kpDefs.h"
#include "document/kpDocument.h"
#include "tools/kpTool.h"
#include "views/manager/kpViewManager.h"
#include "kpViewScrollableContainer.h"
#include "views/kpZoomedView.h"

#include <KSqueezedTextLabel>
#include <KLocalizedString>

//---------------------------------------------------------------------

// private
void kpMainWindow::addPermanentStatusBarItem (int id, int maxTextLen)
{
    QStatusBar *sb = statusBar ();

    QLabel *label = new QLabel (sb);
    label->setAlignment (Qt::AlignCenter);
    label->setFixedHeight (label->fontMetrics ().height () + 2);
    int maxWidth = label->fontMetrics().horizontalAdvance(QLatin1Char ('8')) * maxTextLen;
    // add some margins
    maxWidth += label->fontMetrics ().height ();
    label->setFixedWidth (maxWidth);

    // Permanent --> place on the right
    sb->addPermanentWidget (label);

    d->statusBarLabels.append (label);
    Q_ASSERT (d->statusBarLabels.at(id) == label);
}

//---------------------------------------------------------------------

// private
void kpMainWindow::createStatusBar ()
{
    QStatusBar *sb = statusBar();

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
    qCDebug(kpLogMainWindow) << "kpMainWindow::setStatusBarMessage("
               << message
               << ") ok=" << d->statusBarCreated;
#endif

    if (!d->statusBarCreated) {
        return;
    }

    d->statusBarMessageLabel->setText (message);
}

//---------------------------------------------------------------------

// private slot
void kpMainWindow::setStatusBarShapePoints (const QPoint &startPoint,
                                            const QPoint &endPoint)
{
#if DEBUG_STATUS_BAR && 0
    qCDebug(kpLogMainWindow) << "kpMainWindow::setStatusBarShapePoints("
               << startPoint << "," << endPoint
               << ") ok=" << d->statusBarCreated;
#endif

    if (!d->statusBarCreated) {
        return;
    }

    if (d->statusBarShapeLastPointsInitialised &&
        startPoint == d->statusBarShapeLastStartPoint &&
        endPoint == d->statusBarShapeLastEndPoint)
    {
    #if DEBUG_STATUS_BAR && 0
        qCDebug(kpLogMainWindow) << "\tNOP";
    #endif
        return;
    }

    QLabel *statusBarLabel = d->statusBarLabels.at (StatusBarItemShapePoints);
    if (startPoint == KP_INVALID_POINT)
    {
        statusBarLabel->setText (QString());
    }
    else if (endPoint == KP_INVALID_POINT)
    {
        statusBarLabel->setText (i18n ("%1,%2",
                                      startPoint.x (),
                                      startPoint.y ()));
    }
    else
    {
        statusBarLabel->setText (i18n ("%1,%2 - %3,%4",
                                      startPoint.x (),
                                      startPoint.y (),
                                      endPoint.x (),
                                      endPoint.y ()));
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
    qCDebug(kpLogMainWindow) << "kpMainWindow::setStatusBarShapeSize("
               << size
               << ") ok=" << d->statusBarCreated;
#endif

    if (!d->statusBarCreated) {
        return;
    }

    if (d->statusBarShapeLastSizeInitialised &&
        size == d->statusBarShapeLastSize)
    {
    #if DEBUG_STATUS_BAR && 0
        qCDebug(kpLogMainWindow) << "\tNOP";
    #endif
        return;
    }

    QLabel *statusBarLabel = d->statusBarLabels.at (StatusBarItemShapeSize);
    if (size == KP_INVALID_SIZE)
    {
        statusBarLabel->setText (QString());
    }
    else
    {
        statusBarLabel->setText (i18n ("%1x%2",
                                      size.width (),
                                      size.height ()));
    }

    d->statusBarShapeLastSize = size;
    d->statusBarShapeLastSizeInitialised = true;
}

//---------------------------------------------------------------------

// private slot
void kpMainWindow::setStatusBarDocSize (const QSize &size)
{
#if DEBUG_STATUS_BAR && 0
    qCDebug(kpLogMainWindow) << "kpMainWindow::setStatusBarDocSize("
               << size
               << ") ok=" << d->statusBarCreated;
#endif

    if (!d->statusBarCreated) {
        return;
    }

    QLabel *statusBarLabel = d->statusBarLabels.at (StatusBarItemDocSize);
    if (size == KP_INVALID_SIZE)
    {
        statusBarLabel->setText (QString());
    }
    else
    {
        statusBarLabel->setText (i18n ("%1 x %2",
                                      size.width (),
                                      size.height ()));
    }
}

//---------------------------------------------------------------------

// private slot
void kpMainWindow::setStatusBarDocDepth (int depth)
{
#if DEBUG_STATUS_BAR && 0
    qCDebug(kpLogMainWindow) << "kpMainWindow::setStatusBarDocDepth("
               << depth
               << ") ok=" << d->statusBarCreated;
#endif

    if (!d->statusBarCreated) {
        return;
    }

    QLabel *statusBarLabel = d->statusBarLabels.at (StatusBarItemDocDepth);
    if (depth <= 0)
    {
        statusBarLabel->setText (QString());
    }
    else
    {
        statusBarLabel->setText (i18n ("%1bpp", depth));
    }
}

//---------------------------------------------------------------------

// private slot
void kpMainWindow::setStatusBarZoom (int zoom)
{
#if DEBUG_STATUS_BAR && 0
    qCDebug(kpLogMainWindow) << "kpMainWindow::setStatusBarZoom("
               << zoom
               << ") ok=" << d->statusBarCreated;
#endif

    if (!d->statusBarCreated) {
        return;
    }

    QLabel *statusBarLabel = d->statusBarLabels.at (StatusBarItemZoom);
    if (zoom <= 0)
    {
        statusBarLabel->setText (QString());
    }
    else
    {
        statusBarLabel->setText (i18n ("%1%", zoom));
    }
}

//---------------------------------------------------------------------

void kpMainWindow::recalculateStatusBarMessage ()
{
#if DEBUG_STATUS_BAR && 1
    qCDebug(kpLogMainWindow) << "kpMainWindow::recalculateStatusBarMessage()";
#endif
    QString scrollViewMessage = d->scrollView->statusMessage ();
#if DEBUG_STATUS_BAR && 1
    qCDebug(kpLogMainWindow) << "\tscrollViewMessage=" << scrollViewMessage;
    qCDebug(kpLogMainWindow) << "\tresizing doc? " << !d->scrollView->newDocSize ().isEmpty ();
    qCDebug(kpLogMainWindow) << "\tviewUnderCursor? "
               << (d->viewManager && d->viewManager->viewUnderCursor ());
#endif

    // HACK: To work around kpViewScrollableContainer's unreliable
    //       status messages (which in turn is due to Qt not updating
    //       QWidget::underMouse() on drags and we needing to hack around it)
    if (!scrollViewMessage.isEmpty () &&
        d->scrollView->newDocSize ().isEmpty () &&
        d->viewManager && d->viewManager->viewUnderCursor ())
    {
    #if DEBUG_STATUS_BAR && 1
        qCDebug(kpLogMainWindow) << "\t\tnot resizing & viewUnderCursor - message is wrong - clearing";
    #endif
        d->scrollView->blockSignals (true);
        d->scrollView->clearStatusMessage ();
        d->scrollView->blockSignals (false);

        scrollViewMessage.clear ();
    #if DEBUG_STATUS_BAR && 1
        qCDebug(kpLogMainWindow) << "\t\t\tdone";
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
    qCDebug(kpLogMainWindow) << "kpMainWindow::recalculateStatusBarShape()";
#endif

    QSize docResizeTo = d->scrollView->newDocSize ();
#if DEBUG_STATUS_BAR && 0
    qCDebug(kpLogMainWindow) << "\tdocResizeTo=" << docResizeTo;
#endif
    if (docResizeTo.isValid ())
    {
        const QPoint startPoint (d->document->width (), d->document->height ());
    #if DEBUG_STATUS_BAR && 0
        qCDebug(kpLogMainWindow) << "\thavedMovedFromOrgSize="
                   << d->scrollView->haveMovedFromOriginalDocSize ();
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
        qCDebug(kpLogMainWindow) << "\ttool=" << t;
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
    qCDebug(kpLogMainWindow) << "kpMainWindow::recalculateStatusBar() ok="
               << d->statusBarCreated;
#endif

    if (!d->statusBarCreated) {
        return;
    }

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
