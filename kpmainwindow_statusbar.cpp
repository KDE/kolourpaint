
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

#include <qlabel.h>
#include <qstring.h>

#include <kdebug.h>
#include <klocale.h>
#include <kstatusbar.h>
#include <kpsqueezedtextlabel.h>

#include <kpdefs.h>
#include <kpdocument.h>
#include <kpmainwindow.h>
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

    d->m_statusBarMessageLabel = new kpSqueezedTextLabel (sb);
    sb->addWidget (d->m_statusBarMessageLabel, 1/*stretch*/);

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

    d->m_statusBarCreated = true;
}



// private slot
void kpMainWindow::slotUpdateStatusBarMessage (const QString &message)
{
#if DEBUG_STATUS_BAR && 1
    kdDebug () << "kpMainWindow::slotUpdateStatusBarMessage("
               << message
               << ") ok=" << d->m_statusBarCreated
               << endl;
#endif

    if (!d->m_statusBarCreated)
        return;

    //statusBar ()->changeItem (message, StatusBarItemMessage);
    d->m_statusBarMessageLabel->setText (message);
}

// private slot
void kpMainWindow::slotUpdateStatusBarShapePoints (const QPoint &startPoint,
                                                   const QPoint &endPoint)
{
#if DEBUG_STATUS_BAR && 1
    kdDebug () << "kpMainWindow::slotUpdateStatusBarShapePoints("
               << startPoint << "," << endPoint
               << ") ok=" << d->m_statusBarCreated
               << endl;
#endif

    if (!d->m_statusBarCreated)
        return;

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
}

// private slot
void kpMainWindow::slotUpdateStatusBarShapeSize (const QSize &size)
{
#if DEBUG_STATUS_BAR && 1
    kdDebug () << "kpMainWindow::slotUpdateStatusBarShapeSize("
               << size
               << ") ok=" << d->m_statusBarCreated
               << endl;
#endif

    if (!d->m_statusBarCreated)
        return;

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
}

// private slot
void kpMainWindow::slotUpdateStatusBarDocSize (const QSize &size)
{
#if DEBUG_STATUS_BAR && 1
    kdDebug () << "kpMainWindow::slotUpdateStatusBarDocSize("
               << size
               << ") ok=" << d->m_statusBarCreated
               << endl;
#endif

    if (!d->m_statusBarCreated)
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
void kpMainWindow::slotUpdateStatusBarDocDepth (int depth)
{
#if DEBUG_STATUS_BAR && 1
    kdDebug () << "kpMainWindow::slotUpdateStatusBarDocDepth("
               << depth
               << ") ok=" << d->m_statusBarCreated
               << endl;
#endif

    if (!d->m_statusBarCreated)
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
void kpMainWindow::slotUpdateStatusBarZoom (int zoom)
{
#if DEBUG_STATUS_BAR && 1
    kdDebug () << "kpMainWindow::slotUpdateStatusBarZoom("
               << zoom
               << ") ok=" << d->m_statusBarCreated
               << endl;
#endif

    if (!d->m_statusBarCreated)
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


// private slot
void kpMainWindow::slotUpdateStatusBar ()
{
#if DEBUG_STATUS_BAR && 1
    kdDebug () << "kpMainWindow::slotUpdateStatusBar() ok="
               << d->m_statusBarCreated
               << endl;
#endif

    if (!d->m_statusBarCreated)
        return;

    const kpTool *t = tool ();
    if (t)
    {
        slotUpdateStatusBarMessage (t->userMessage ());
        slotUpdateStatusBarShapePoints (t->userShapeStartPoint (),
                                        t->userShapeEndPoint ());
        slotUpdateStatusBarShapeSize (t->userShapeSize ());
    }
    else
    {
        slotUpdateStatusBarMessage ();
        slotUpdateStatusBarShapePoints ();
        slotUpdateStatusBarShapeSize ();
    }

    if (m_document)
    {
        slotUpdateStatusBarDocSize (QSize (m_document->width (), m_document->height ()));
        slotUpdateStatusBarDocDepth (m_document->colorDepth ());
    }
    else
    {
        slotUpdateStatusBarDocSize ();
        slotUpdateStatusBarDocDepth ();
    }

    if (m_mainView)
    {
        slotUpdateStatusBarZoom (m_mainView->zoomLevelX ());
    }
    else
    {
        slotUpdateStatusBarZoom ();
    }
}
