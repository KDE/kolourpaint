
/*
   Copyright (c) 2003,2004,2005 Clarence Dang <dang@kde.org>
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
#include <kpviewmanager.h>
#include <kpviewscrollablecontainer.h>
#include <kpzoomedview.h>


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

    m_statusBarShapeLastPointsInitialised = false;
    m_statusBarShapeLastSizeInitialised = false;
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
#if DEBUG_STATUS_BAR && 0
    kdDebug () << "kpMainWindow::setStatusBarShapePoints("
               << startPoint << "," << endPoint
               << ") ok=" << m_statusBarCreated
               << endl;
#endif

    if (!m_statusBarCreated)
        return;

    if (m_statusBarShapeLastPointsInitialised &&
        startPoint == m_statusBarShapeLastStartPoint &&
        endPoint == m_statusBarShapeLastEndPoint)
    {
    #if DEBUG_STATUS_BAR && 0
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

    m_statusBarShapeLastStartPoint = startPoint;
    m_statusBarShapeLastEndPoint = endPoint;
    m_statusBarShapeLastPointsInitialised = true;
}

// private slot
void kpMainWindow::setStatusBarShapeSize (const QSize &size)
{
#if DEBUG_STATUS_BAR && 0
    kdDebug () << "kpMainWindow::setStatusBarShapeSize("
               << size
               << ") ok=" << m_statusBarCreated
               << endl;
#endif

    if (!m_statusBarCreated)
        return;

    if (m_statusBarShapeLastSizeInitialised &&
        size == m_statusBarShapeLastSize)
    {
    #if DEBUG_STATUS_BAR && 0
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

    m_statusBarShapeLastSize = size;
    m_statusBarShapeLastSizeInitialised = true;
}

// private slot
void kpMainWindow::setStatusBarDocSize (const QSize &size)
{
#if DEBUG_STATUS_BAR && 0
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
#if DEBUG_STATUS_BAR && 0
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
#if DEBUG_STATUS_BAR && 0
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
#if DEBUG_STATUS_BAR && 1
    kdDebug () << "kpMainWindow::recalculateStatusBarMessage()" << endl;
#endif
    QString scrollViewMessage = m_scrollView->statusMessage ();
#if DEBUG_STATUS_BAR && 1
    kdDebug () << "\tscrollViewMessage=" << scrollViewMessage << endl;
    kdDebug () << "\tresizing doc? " << !m_scrollView->newDocSize ().isEmpty ()
               << endl;
    kdDebug () << "\tviewUnderCursor? "
               << (m_viewManager && m_viewManager->viewUnderCursor ())
               << endl;
#endif

    // HACK: To work around kpViewScrollableContainer's unreliable
    //       status messages (which in turn is due to Qt not updating
    //       QWidget::hasMouse() on drags and we needing to hack around it)
    if (!scrollViewMessage.isEmpty () &&
        m_scrollView->newDocSize ().isEmpty () &&
        m_viewManager && m_viewManager->viewUnderCursor ())
    {
    #if DEBUG_STATUS_BAR && 1
        kdDebug () << "\t\tnot resizing & viewUnderCursor - message is wrong - clearing"
                   << endl;
    #endif
        m_scrollView->blockSignals (true);
        m_scrollView->clearStatusMessage ();
        m_scrollView->blockSignals (false);

        scrollViewMessage = QString::null;
    #if DEBUG_STATUS_BAR && 1
        kdDebug () << "\t\t\tdone" << endl;
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

// private slot
void kpMainWindow::recalculateStatusBarShape ()
{
#if DEBUG_STATUS_BAR && 0
    kdDebug () << "kpMainWindow::recalculateStatusBarShape()" << endl;
#endif

    QSize docResizeTo = m_scrollView->newDocSize ();
#if DEBUG_STATUS_BAR && 0
    kdDebug () << "\tdocResizeTo=" << docResizeTo << endl;
#endif
    if (docResizeTo.isValid ())
    {
        const QPoint startPoint (m_document->width (), m_document->height ());
    #if DEBUG_STATUS_BAR && 0
        kdDebug () << "\thavedMovedFromOrgSize="
                   << m_scrollView->haveMovedFromOriginalDocSize () << endl;
    #endif
        if (!m_scrollView->haveMovedFromOriginalDocSize ())
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
        kdDebug () << "\ttool=" << t << endl;
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
        setStatusBarDocDepth (m_document->pixmap ()->depth ());
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
