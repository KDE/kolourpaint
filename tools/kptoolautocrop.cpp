
/* This file is part of the KolourPaint project
   Copyright (c) 2003 Clarence Dang <dang@kde.org>
   All rights reserved.
   
   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:
   
   1. Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
   2. Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
   3. Neither the names of the copyright holders nor the names of
      contributors may be used to endorse or promote products derived from
      this software without specific prior written permission.
   
   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
   PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
   HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#define DEBUG_KP_TOOL_AUTO_CROP 1

#include <qapplication.h>
#include <qimage.h>
#include <qpainter.h>

#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>

#include <kpcommandhistory.h>
#include <kpdocument.h>
#include <kpmainwindow.h>
#include <kptool.h>
#include <kptoolautocrop.h>
#include <kpviewmanager.h>


kpToolAutoCropBorder::kpToolAutoCropBorder (const QPixmap *pixmapPtr)
{
    m_pixmapPtr = pixmapPtr;
}

bool kpToolAutoCropBorder::calculate (int isX, int dir)
{
    int maxX = m_pixmapPtr->width () - 1;
    int maxY = m_pixmapPtr->height () - 1;
    
    QImage image = m_pixmapPtr->convertToImage ();
    if (image.isNull ())
    {
        kdError () << "Border::calculate() could not convert to QImage" << endl;
        return false;
    }
    
    // (sync both branches)
    if (isX)
    {
        int numCols = 0;
        int startX = (dir > 0) ? 0 : maxX;
        
        QRgb rgb = image.pixel (startX, 0);
        for (int x = startX;
             x >= 0 && x <= maxX;
             x += dir)
        {
            int y;
            for (y = 0; y <= maxY; y++)
            {
                if (image.pixel (x, y) != rgb)
                    break;
            }
            
            if (y <= maxY)
                break;
            else
                numCols++;
        }
        
        if (numCols)
        {
            m_rect = QRect (QPoint (startX, 0),
                            QPoint (startX + (numCols - 1) * dir, maxY)).normalize ();
            m_rgb = rgb;
        }
    }
    else
    {
        int numRows = 0;
        int startY = (dir > 0) ? 0 : maxY;
        
        QRgb rgb = image.pixel (0, startY);
        for (int y = startY;
             y >= 0 & y <= maxY;
             y += dir)
        {
            int x;
            for (x = 0; x <= maxX; x++)
            {
                if (image.pixel (x, y) != rgb)
                    break;
            }
            
            if (x <= maxX)
                break;
            else
                numRows++;
        }
        
        if (numRows)
        {
            m_rect = QRect (QPoint (0, startY),
                            QPoint (maxX, startY + (numRows - 1) * dir)).normalize ();
            m_rgb = rgb;
        }
    }
    
    return true;
}

bool kpToolAutoCropBorder::fillsEntirePixmap () const
{
    return m_rect == m_pixmapPtr->rect ();
}

bool kpToolAutoCropBorder::exists () const
{
    // (will use in an addition so make sure returns 1 or 0)
    return m_rect.isValid () ? 1 : 0;
}

void kpToolAutoCropBorder::invalidate ()
{
    m_rect = QRect ();
    m_rgb = QRgb ();
}


bool kpToolAutoCrop (kpMainWindow *mainWindow)
{
#if DEBUG_KP_TOOL_AUTO_CROP
    kdDebug () << "kpToolAutoCrop() CALLED!" << endl;
#endif

    bool nothingToCrop = false;
    
    if (!mainWindow)
    {
        kdError () << "kpToolAutoCrop() passed NULL mainWindow" << endl;
        return false;
    }
    
    kpDocument *doc = mainWindow->document ();
    if (!doc)
    {
        kdError () << "kpToolAutoCrop() passed NULL document" << endl;
        return false;
    }

    QPixmap *pixmap = doc->pixmap ();
    if (!pixmap)
    {
        kdError () << "kptoolAutoCrop() pased NULL pixmap" << endl;
        return false;
    }
    
    kpViewManager *vm = mainWindow->viewManager ();
    if (!vm)
    {
        kdError () << "kpToolAutoCrop() passed NULL vm" << endl;
        return false;
    }
    
    kpToolAutoCropBorder leftBorder (pixmap), rightBorder (pixmap),
                         topBorder (pixmap), botBorder (pixmap);
    
    // sync: restoreOverrideCursor() for all exit paths
    QApplication::setOverrideCursor (Qt::waitCursor);
    
    if (!leftBorder.calculate (true/*x*/, +1/*going right*/))
    {
        QApplication::restoreOverrideCursor ();
        return false;
    }

    nothingToCrop = leftBorder.fillsEntirePixmap ();

#if DEBUG_KP_TOOL_AUTO_CROP
    if (nothingToCrop)
        kdDebug () << "\tleft border filled entire pixmap - nothing to crop" << endl;
#endif
    
    if (!nothingToCrop)
    {
        if (!rightBorder.calculate (true/*x*/, -1/*going left*/) ||
            !topBorder.calculate (false/*y*/, +1/*going down*/) ||
            !botBorder.calculate (false/*y*/, -1/*going up*/))
        {
            QApplication::restoreOverrideCursor ();
            return false;
        }
        
        int numRegions = leftBorder.exists () +
                         rightBorder.exists () +
                         topBorder.exists () +
                         botBorder.exists ();
        nothingToCrop = !numRegions;

    #if DEBUG_KP_TOOL_AUTO_CROP
        kdDebug () << "\tnumRegions=" << numRegions << endl;
        kdDebug () << "\t\tleft=" << leftBorder.m_rect << endl;
        kdDebug () << "\t\tright=" << rightBorder.m_rect << endl;
        kdDebug () << "\t\ttop=" << topBorder.m_rect << endl;
        kdDebug () << "\t\tbot=" << botBorder.m_rect << endl;
    #endif
        
        if (numRegions == 2)
        {
        #if DEBUG_KP_TOOL_AUTO_CROP
            kdDebug () << "\t2 regions:" << endl;
        #endif
        
            // in case e.g. the user pastes a solid, coloured-in rectangle,
            // we favour killing the bottom and right regions
            // (these regions probably contain the unwanted whitespace due
            //  to the doc being bigger than the pasted selection to start with)

            if (leftBorder.exists () && rightBorder.exists () &&
                leftBorder.m_rgb != rightBorder.m_rgb)
            {
            #if DEBUG_KP_TOOL_AUTO_CROP
                kdDebug () << "\t\tignoring left border" << endl;
            #endif
                leftBorder.invalidate ();
            }
            else if (topBorder.exists () && botBorder.exists () &&
                     topBorder.m_rgb != botBorder.m_rgb)
            {
            #if DEBUG_KP_TOOL_AUTO_CROP
                kdDebug () << "\t\tignoring right border" << endl;
            #endif
                topBorder.invalidate ();
            }
        #if DEBUG_KP_TOOL_AUTO_CROP
            else
            {
                kdDebug () << "\t\tok" << endl;
            }
        #endif
        }
    }
    
#if DEBUG_KP_TOOL_AUTO_CROP
    kdDebug () << "\tnothingToCrop=" << nothingToCrop << endl;
#endif

    if (!nothingToCrop)
    {
        mainWindow->commandHistory ()->addCommand (
            new kpToolAutoCropCommand (
                leftBorder, rightBorder,
                topBorder, botBorder,
                mainWindow));
    }
    else
    {
        KMessageBox::information (mainWindow,
            i18n ("Autocrop could not find any border to remove."),
            i18n ("Nothing to Autocrop"),
            "DoNotAskAgain_NothingToAutoCrop");
    }
    
    QApplication::restoreOverrideCursor ();
    return true;
}


kpToolAutoCropCommand::kpToolAutoCropCommand (const kpToolAutoCropBorder &leftBorder,
                                              const kpToolAutoCropBorder &rightBorder,
                                              const kpToolAutoCropBorder &topBorder,
                                              const kpToolAutoCropBorder &botBorder,
                                              kpMainWindow *mainWindow)
    : m_leftBorder (leftBorder),
      m_rightBorder (rightBorder),
      m_topBorder (topBorder),
      m_botBorder (botBorder),
      m_mainWindow (mainWindow)
{
    m_contentsRect = contentsRect ();
    m_oldWidth = mainWindow->document ()->width ();
    m_oldHeight = mainWindow->document ()->height ();
}

// public virtual [base KCommand]
QString kpToolAutoCropCommand::name () const
{
    return i18n ("Autocrop");
}

kpToolAutoCropCommand::~kpToolAutoCropCommand ()
{
}


// public virtual [base KCommand]
void kpToolAutoCropCommand::execute ()
{
    kpDocument *doc = m_mainWindow->document ();
    doc->setPixmap (kpTool::neededPixmap (*doc->pixmap (), m_contentsRect));
}

// public virtual [base KCommand]
void kpToolAutoCropCommand::unexecute ()
{
    kpDocument *doc = m_mainWindow->document ();
    
    QPixmap pixmap (m_oldWidth, m_oldHeight);
    
    // restore the position of the centre image
    QPainter painter (&pixmap);
    painter.drawPixmap (m_contentsRect, *doc->pixmap());
    
    // draw the borders
    
    const kpToolAutoCropBorder *borders [] =
    {
        &m_leftBorder, &m_rightBorder,
        &m_topBorder, &m_botBorder,
        0
    };

    for (const kpToolAutoCropBorder **b = borders; *b; b++)
    {
        if ((*b)->exists ())
        {
            painter.setPen (QColor ((*b)->m_rgb));
            painter.setBrush (QColor ((*b)->m_rgb));
        
            painter.drawRect ((*b)->m_rect);
        }
    }
    
    painter.end ();
    
    doc->setPixmap (pixmap);
}


// private
QRect kpToolAutoCropCommand::contentsRect () const
{
    QPixmap *pixmap = m_mainWindow->document ()->pixmap ();
    
    QPoint topLeft (m_leftBorder.exists () ?
                        m_leftBorder.m_rect.right () + 1 : 
                        0,
                    m_topBorder.exists () ?
                        m_topBorder.m_rect.bottom () + 1 :
                        0);
    QPoint botRight (m_rightBorder.exists () ?
                         m_rightBorder.m_rect.left () - 1 :
                         pixmap->width () - 1,
                     m_botBorder.exists () ?
                         m_botBorder.m_rect.top () - 1 :
                         pixmap->height () - 1);
                  
    return QRect (topLeft, botRight);
}
