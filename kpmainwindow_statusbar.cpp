
/*
   Copyright (c) 2003-2004 Clarence Dang <dang@kde.org>
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


#include <kpmainwindow.h>

#include <qlabel.h>
#include <qstring.h>

#include <kdebug.h>
#include <klocale.h>
#include <kstatusbar.h>

#include <kpdefs.h>
#include <kpdocument.h>
#include <kpsqueezedtextlabel.h>
#include <kptool.h>
#include <kpview.h>


// private
void kpMainWindow::addPermanentStatusBarItem (int id, int maxTextLen)
{
    KStatusBar *sb = statusBar ();

    QString textWithMaxLen;
    textWithMaxLen.fill (QString::number (8/*big fat*/).at (0),
                         maxTextLen); //+ 2/*spaces on either side*/);

    sb->insertFixedItem (textWithMaxLen, id, true/*permanent, place on the right*/);
    sb->changeItem (QString::null, id);
}

// private
void kpMainWindow::createStatusBar ()
{
    KStatusBar *sb = statusBar ();

    // 9999 pixels "ought to be enough for anybody"
    const int maxDimenLength = 4;

    //sb->insertItem (QString::null, StatusBarItemMessage, 1/*stretch*/);
    //sb->setItemAlignment (StatusBarItemMessage, Qt::AlignLeft | Qt::AlignVCenter);

    m_statusBarMessageLabel = new kpSqueezedTextLabel (sb);
    //m_statusBarMessageLabel->setShowEllipsis (false);
    sb->addWidget (m_statusBarMessageLabel, 1/*stretch*/);

    addPermanentStatusBarItem (StatusBarItemShapePoints,
                               (maxDimenLength + 1/*,*/ + maxDimenLength) * 2 + 3/* - */);
    addPermanentStatusBarItem (StatusBarItemShapeSize,
                               (1/*+/-*/ + maxDimenLength) * 2 + 1/*x*/);

    addPermanentStatusBarItem (StatusBarItemDocSize,
                               maxDimenLength + 1/*x*/ + maxDimenLength);
    addPermanentStatusBarItem (StatusBarItemDocDepth,
                               5/*XXbpp*/);

    addPermanentStatusBarItem (StatusBarItemZoom,
                               5/*1600%*/);

    d->m_statusBarShapeLastPointsInitialised = false;
    d->m_statusBarShapeLastSizeInitialised = false;
    m_statusBarCreated = true;
}



// private slot
void kpMainWindow::setStatusBarMessage (const QString &message)
{
#if DEBUG_STATUS_BAR && 1
    kdDebug () << "kpMainWindow::setStatusBarMessage("
               << message
               << ") ok=" << m_statusBarCreated
               << endl;
#endif

    if (!m_statusBarCreated)
        return;

    //statusBar ()->changeItem (message, StatusBarItemMessage);
    m_statusBarMessageLabel->setText (message);
}

// private slot
void kpMainWindow::setStatusBarShapePoints (const QPoint &startPoint,
                                            const QPoint &endPoint)
{
#if DEBUG_STATUS_BAR && 1
    kdDebug () << "kpMainWindow::setStatusBarShapePoints("
               << startPoint << "," << endPoint
               << ") ok=" << m_statusBarCreated
               << endl;
#endif

    if (!m_statusBarCreated)
        return;

    if (d->m_statusBarShapeLastPointsInitialised &&
        startPoint == d->m_statusBarShapeLastStartPoint &&
        endPoint == d->m_statusBarShapeLastEndPoint)
    {
    #if DEBUG_STATUS_BAR && 1
        kdDebug () << "\tNOP" << endl;
    #endif
        return;
    }

    if (startPoint == KP_INVALID_POINT)
    {
        statusBar ()->changeItem (QString::null, StatusBarItemShapePoints);
    }
    else if (endPoint == KP_INVALID_POINT)
    {
        statusBar ()->changeItem (i18n ("%1,%2")
                                    .arg (startPoint.x ())
                                    .arg (startPoint.y ()),
                                  StatusBarItemShapePoints);
    }
    else
    {
        statusBar ()->changeItem (i18n  ("%1,%2 - %3,%4")
                                    .arg (startPoint.x ())
                                    .arg (startPoint.y ())
                                    .arg (endPoint.x ())
                                    .arg (endPoint.y ()),
                                  StatusBarItemShapePoints);
    }

    d->m_statusBarShapeLastStartPoint = startPoint;
    d->m_statusBarShapeLastEndPoint = endPoint;
    d->m_statusBarShapeLastPointsInitialised = true;
}

// private slot
void kpMainWindow::setStatusBarShapeSize (const QSize &size)
{
#if DEBUG_STATUS_BAR && 1
    kdDebug () << "kpMainWindow::setStatusBarShapeSize("
               << size
               << ") ok=" << m_statusBarCreated
               << endl;
#endif

    if (!m_statusBarCreated)
        return;

    if (d->m_statusBarShapeLastSizeInitialised &&
        size == d->m_statusBarShapeLastSize)
    {
    #if DEBUG_STATUS_BAR && 1
        kdDebug () << "\tNOP" << endl;
    #endif
        return;
    }

    if (size == KP_INVALID_SIZE)
    {
        statusBar ()->changeItem (QString::null, StatusBarItemShapeSize);
    }
    else
    {
        statusBar ()->changeItem (i18n ("%1x%2")
                                    .arg (size.width ())
                                    .arg (size.height ()),
                                  StatusBarItemShapeSize);
    }

    d->m_statusBarShapeLastSize = size;
    d->m_statusBarShapeLastSizeInitialised = true;
}

// private slot
void kpMainWindow::setStatusBarDocSize (const QSize &size)
{
#if DEBUG_STATUS_BAR && 1
    kdDebug () << "kpMainWindow::setStatusBarDocSize("
               << size
               << ") ok=" << m_statusBarCreated
               << endl;
#endif

    if (!m_statusBarCreated)
        return;

    if (size == KP_INVALID_SIZE)
    {
        statusBar ()->changeItem (QString::null, StatusBarItemDocSize);
    }
    else
    {
        statusBar ()->changeItem (i18n ("%1x%2")
                                    .arg (size.width ())
                                    .arg (size.height ()),
                                  StatusBarItemDocSize);
    }
}

// private slot
void kpMainWindow::setStatusBarDocDepth (int depth)
{
#if DEBUG_STATUS_BAR && 1
    kdDebug () << "kpMainWindow::setStatusBarDocDepth("
               << depth
               << ") ok=" << m_statusBarCreated
               << endl;
#endif

    if (!m_statusBarCreated)
        return;

    if (depth <= 0)
    {
        statusBar ()->changeItem (QString::null, StatusBarItemDocDepth);
    }
    else
    {
        statusBar ()->changeItem (i18n ("%1bpp").arg (depth),
                                  StatusBarItemDocDepth);
    }
}

// private slot
void kpMainWindow::setStatusBarZoom (int zoom)
{
#if DEBUG_STATUS_BAR && 1
    kdDebug () << "kpMainWindow::setStatusBarZoom("
               << zoom
               << ") ok=" << m_statusBarCreated
               << endl;
#endif

    if (!m_statusBarCreated)
        return;

    if (zoom <= 0)
    {
        statusBar ()->changeItem (QString::null, StatusBarItemZoom);
    }
    else
    {
        statusBar ()->changeItem (i18n ("%1%").arg (zoom),
                                  StatusBarItemZoom);
    }
}

void kpMainWindow::recalculateStatusBarMessage ()
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

// private slot
void kpMainWindow::recalculateStatusBarShape ()
{
    const kpTool *t = tool ();
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

// private slot
void kpMainWindow::recalculateStatusBar ()
{
#if DEBUG_STATUS_BAR && 1
    kdDebug () << "kpMainWindow::recalculateStatusBar() ok="
               << m_statusBarCreated
               << endl;
#endif

    if (!m_statusBarCreated)
        return;

    recalculateStatusBarMessage ();
    recalculateStatusBarShape ();

    if (m_document)
    {
        setStatusBarDocSize (QSize (m_document->width (), m_document->height ()));
        setStatusBarDocDepth (m_document->colorDepth ());
    }
    else
    {
        setStatusBarDocSize ();
        setStatusBarDocDepth ();
    }

    if (m_mainView)
    {
        setStatusBarZoom (m_mainView->zoomLevelX ());
    }
    else
    {
        setStatusBarZoom ();
    }
}
