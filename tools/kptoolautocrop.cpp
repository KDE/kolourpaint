
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

// TODO: Color Similarity is obviously useful in Autocrop but it isn't
//       obvious as to how to implement it.  The current heuristic,
//       for each side, chooses an arbitrary reference color for which
//       all other candidate pixels in that side are tested against
//       for similarity.  But if the reference color happens to be at
//       one extreme of the range of colors in that side, then pixels
//       at the other extreme would not be deemed similar enough.  The
//       key is to find the median color as the reference but how do
//       you do this if you don't know which pixels to sample in the first
//       place (that's what you're trying to find)?  Chicken and egg situation.
//
//       The other heuristic that is in doubt is the use of the average
//       color in determining the similarity of sides (it is possible
//       to get vastly differently colors in both sides yet they will be
//       considered similar).

#define DEBUG_KP_TOOL_AUTO_CROP 1

#include <qapplication.h>
#include <qbitmap.h>
#include <qimage.h>
#include <qpainter.h>

#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>

#include <kpcolortoolbar.h>
#include <kpcommandhistory.h>
#include <kpdocument.h>
#include <kpmainwindow.h>
#include <kppixmapfx.h>
#include <kpselection.h>
#include <kptool.h>
#include <kptoolautocrop.h>
#include <kpviewmanager.h>


kpToolAutoCropBorder::kpToolAutoCropBorder (const QPixmap *pixmapPtr,
                                            int processedColorSimilarity)
    : m_pixmapPtr (pixmapPtr),
      m_processedColorSimilarity (processedColorSimilarity)
{
    invalidate ();
}


// public
const QPixmap *kpToolAutoCropBorder::pixmap () const
{
    return m_pixmapPtr;
}

// public
int kpToolAutoCropBorder::processedColorSimilarity () const
{
    return m_processedColorSimilarity;
}

// public
QRect kpToolAutoCropBorder::rect () const
{
    return m_rect;
}

// public
int kpToolAutoCropBorder::left () const
{
    return m_rect.left ();
}

// public
int kpToolAutoCropBorder::right () const
{
    return m_rect.right ();
}

// public
int kpToolAutoCropBorder::top () const
{
    return m_rect.top ();
}

// public
int kpToolAutoCropBorder::bottom () const
{
    return m_rect.bottom ();
}

// public
kpColor kpToolAutoCropBorder::referenceColor () const
{
    return m_referenceColor;
}

// public
kpColor kpToolAutoCropBorder::averageColor () const
{
    if (!m_rect.isValid ())
        return kpColor::invalid;

    if (m_referenceColor.isTransparent ())
        return kpColor::transparent;
    else if (m_processedColorSimilarity == 0)
        return m_referenceColor;
    else
    {
        int numPixels = (m_rect.width () * m_rect.height ());
        if (numPixels <= 0)
        {
            kdError () << "kpToolAutoCropBorder::averageColor() rect=" << m_rect << endl;
            return kpColor::invalid;
        }

        return kpColor (m_redSum / numPixels,
                        m_greenSum / numPixels,
                        m_blueSum / numPixels);
    }
}

bool kpToolAutoCropBorder::isSingleColor () const
{
    return m_isSingleColor;
}


// public
bool kpToolAutoCropBorder::calculate (int isX, int dir)
{
#if DEBUG_KP_TOOL_AUTO_CROP && 1
    kdDebug () << "kpToolAutoCropBorder::calculate() CALLED!" << endl;
#endif
    int maxX = m_pixmapPtr->width () - 1;
    int maxY = m_pixmapPtr->height () - 1;

    QImage image = kpPixmapFX::convertToImage (*m_pixmapPtr);
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

        kpColor col = kpPixmapFX::getColorAtPixel (image, startX, 0);
        for (int x = startX;
             x >= 0 && x <= maxX;
             x += dir)
        {
            int y;
            for (y = 0; y <= maxY; y++)
            {
                if (!kpPixmapFX::getColorAtPixel (image, x, y).isSimilarTo (col, m_processedColorSimilarity))
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
            m_referenceColor = col;
        }
    }
    else
    {
        int numRows = 0;
        int startY = (dir > 0) ? 0 : maxY;

        kpColor col = kpPixmapFX::getColorAtPixel (image, 0, startY);
        for (int y = startY;
             y >= 0 && y <= maxY;
             y += dir)
        {
            int x;
            for (x = 0; x <= maxX; x++)
            {
                if (!kpPixmapFX::getColorAtPixel (image, x, y).isSimilarTo (col, m_processedColorSimilarity))
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
            m_referenceColor = col;
        }
    }


    if (m_rect.isValid ())
    {
        m_isSingleColor = true;

        if (m_referenceColor.isOpaque () && m_processedColorSimilarity != 0)
        {
            for (int y = m_rect.top (); y <= m_rect.bottom (); y++)
            {
                for (int x = m_rect.left (); x <= m_rect.right (); x++)
                {
                    kpColor colAtPixel = kpPixmapFX::getColorAtPixel (image, x, y);

                    if (m_isSingleColor && colAtPixel != m_referenceColor)
                        m_isSingleColor = false;

                    m_redSum += colAtPixel.red ();
                    m_greenSum += colAtPixel.green ();
                    m_blueSum += colAtPixel.blue ();
                }
            }
        }
    }


    return true;
}

// public
bool kpToolAutoCropBorder::fillsEntirePixmap () const
{
    return (m_rect == m_pixmapPtr->rect ());
}

// public
bool kpToolAutoCropBorder::exists () const
{
    // (will use in an addition so make sure returns 1 or 0)
    return (m_rect.isValid () ? 1 : 0);
}

// public
void kpToolAutoCropBorder::invalidate ()
{
    m_rect = QRect ();
    m_referenceColor = kpColor::invalid;
    m_redSum = m_greenSum = m_blueSum = 0;
    m_isSingleColor = false;
}


class kpSetOverrideCursorSaver
{
public:
    kpSetOverrideCursorSaver (const QCursor &cursor)
    {
        QApplication::setOverrideCursor (cursor);
    }

    ~kpSetOverrideCursorSaver ()
    {
        QApplication::restoreOverrideCursor ();
    }
};


void showNothingToAutocropMessage (kpMainWindow *mainWindow, bool actOnSelection)
{
    kpSetOverrideCursorSaver cursorSaver (Qt::arrowCursor);

    KMessageBox::information (mainWindow,
        i18n ("Autocrop could not find the %1 border so could not crop it.")
            .arg (actOnSelection ? i18n ("selection's") : i18n ("image's")),
        i18n ("Nothing to Autocrop"),
        "NothingToAutoCrop");
}

bool kpToolAutoCrop (kpMainWindow *mainWindow)
{
#if DEBUG_KP_TOOL_AUTO_CROP
    kdDebug () << "kpToolAutoCrop() CALLED!" << endl;
#endif

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

    // OPT: if already pulled selection pixmap, no need to do it again here
    QPixmap pixmap = doc->selection () ? doc->getSelectedPixmap () : *doc->pixmap ();
    if (pixmap.isNull ())
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

    int processedColorSimilarity = mainWindow->colorToolBar ()->processedColorSimilarity ();
    kpToolAutoCropBorder leftBorder (&pixmap, processedColorSimilarity),
                         rightBorder (&pixmap, processedColorSimilarity),
                         topBorder (&pixmap, processedColorSimilarity),
                         botBorder (&pixmap, processedColorSimilarity);


    kpSetOverrideCursorSaver cursorSaver (Qt::waitCursor);

    // TODO: With Colour Similarity, a lot of weird (and wonderful) things can
    //       happen resulting in a huge number of code paths.  Needs refactoring
    //       and regression testing.
    //
    // TODO: e.g. When the top fills entire rect but bot doesn't we could
    //       invalidate top and continue autocrop.
    int numRegions = 0;
    if (!leftBorder.calculate (true/*x*/, +1/*going right*/) ||
        leftBorder.fillsEntirePixmap () ||
        !rightBorder.calculate (true/*x*/, -1/*going left*/) ||
        rightBorder.fillsEntirePixmap () ||
        !topBorder.calculate (false/*y*/, +1/*going down*/) ||
        topBorder.fillsEntirePixmap () ||
        !botBorder.calculate (false/*y*/, -1/*going up*/) ||
        botBorder.fillsEntirePixmap () ||
        ((numRegions = leftBorder.exists () +
                       rightBorder.exists () +
                       topBorder.exists () +
                       botBorder.exists ()) == 0))
    {
    #if DEBUG_KP_TOOL_AUTO_CROP
        kdDebug () << "\tcan't find border; leftBorder.rect=" << leftBorder.rect ()
                   << " rightBorder.rect=" << rightBorder.rect ()
                   << " topBorder.rect=" << topBorder.rect ()
                   << " botBorder.rect=" << botBorder.rect ()
                   << endl;
    #endif
        ::showNothingToAutocropMessage (mainWindow, (bool) doc->selection ());
        return false;
    }

#if DEBUG_KP_TOOL_AUTO_CROP
    kdDebug () << "\tnumRegions=" << numRegions << endl;
    kdDebug () << "\t\tleft=" << leftBorder.rect ()
               << " refCol=" << (leftBorder.exists () ? (int *) leftBorder.referenceColor ().toQRgb () : 0)
               << " avgCol=" << (leftBorder.exists () ? (int *) leftBorder.averageColor ().toQRgb () : 0)
               << endl;
    kdDebug () << "\t\tright=" << rightBorder.rect ()
               << " refCol=" << (rightBorder.exists () ? (int *) rightBorder.referenceColor ().toQRgb () : 0)
               << " avgCol=" << (rightBorder.exists () ? (int *) rightBorder.averageColor ().toQRgb () : 0)
               << endl;
    kdDebug () << "\t\ttop=" << topBorder.rect ()
               << " refCol=" << (topBorder.exists () ? (int *) topBorder.referenceColor ().toQRgb () : 0)
               << " avgCol=" << (topBorder.exists () ? (int *) topBorder.averageColor ().toQRgb () : 0)
               << endl;
    kdDebug () << "\t\tbot=" << botBorder.rect ()
               << " refCol=" << (botBorder.exists () ? (int *) botBorder.referenceColor ().toQRgb () : 0)
               << " avgCol=" << (botBorder.exists () ? (int *) botBorder.averageColor ().toQRgb () : 0)
               << endl;
#endif


    // In case e.g. the user pastes a solid, coloured-in rectangle,
    // we favour killing the bottom and right regions
    // (these regions probably contain the unwanted whitespace due
    //  to the doc being bigger than the pasted selection to start with).
    //
    // We also kill if they kiss or even overlap.

    if (leftBorder.exists () && rightBorder.exists ())
    {
        const kpColor leftCol = leftBorder.averageColor ();
        const kpColor rightCol = rightBorder.averageColor ();

        if ((numRegions == 2 && !leftCol.isSimilarTo (rightCol, processedColorSimilarity)) ||
            leftBorder.right () >= rightBorder.left () - 1)  // kissing or overlapping
        {
        #if DEBUG_KP_TOOL_AUTO_CROP
            kdDebug () << "\tignoring left border" << endl;
        #endif
            leftBorder.invalidate ();
        }
    }

    if (topBorder.exists () && botBorder.exists ())
    {
        const kpColor topCol = topBorder.averageColor ();
        const kpColor botCol = botBorder.averageColor ();

        if ((numRegions == 2 && !topCol.isSimilarTo (botCol, processedColorSimilarity)) ||
            topBorder.bottom () >= botBorder.top () - 1)  // kissing or overlapping
        {
        #if DEBUG_KP_TOOL_AUTO_CROP
            kdDebug () << "\tignoring top border" << endl;
        #endif
            topBorder.invalidate ();
        }
    }


    mainWindow->addImageOrSelectionCommand (
        new kpToolAutoCropCommand (
            (bool) doc->selection (),
            leftBorder, rightBorder,
            topBorder, botBorder,
            mainWindow));


    return true;
}


kpToolAutoCropCommand::kpToolAutoCropCommand (bool actOnSelection,
                                              const kpToolAutoCropBorder &leftBorder,
                                              const kpToolAutoCropBorder &rightBorder,
                                              const kpToolAutoCropBorder &topBorder,
                                              const kpToolAutoCropBorder &botBorder,
                                              kpMainWindow *mainWindow)
    : m_actOnSelection (actOnSelection),
      m_leftBorder (leftBorder),
      m_rightBorder (rightBorder),
      m_topBorder (topBorder),
      m_botBorder (botBorder),
      m_leftPixmap (0),
      m_rightPixmap (0),
      m_topPixmap (0),
      m_botPixmap (0),
      m_mainWindow (mainWindow)
{
    kpDocument *doc = document ();
    if (!doc)
    {
        kdError () << "kpToolAutoCropCommand::<ctor>() without doc" << endl;
        m_oldWidth = 0;
        m_oldHeight = 0;
        return;
    }

    m_oldWidth = doc->width (m_actOnSelection);
    m_oldHeight = doc->height (m_actOnSelection);
}

// public virtual [base KCommand]
QString kpToolAutoCropCommand::name () const
{
    const QString opName = i18n ("Autocrop");

    if (m_actOnSelection)
        return i18n ("Selection: %1").arg (opName);
    else
        return opName;
}

kpToolAutoCropCommand::~kpToolAutoCropCommand ()
{
    deleteUndoPixmaps ();
}


// private
kpDocument *kpToolAutoCropCommand::document () const
{
    return m_mainWindow ? m_mainWindow->document () : 0;
}

// private
kpViewManager *kpToolAutoCropCommand::viewManager () const
{
    return m_mainWindow ? m_mainWindow->viewManager () : 0;
}


// private
void kpToolAutoCropCommand::getUndoPixmap (const kpToolAutoCropBorder &border, QPixmap **pixmap)
{
    kpDocument *doc = document ();

#if DEBUG_KP_TOOL_AUTO_CROP && 1
    kdDebug () << "kpToolAutoCropCommand::getUndoPixmap()" << endl;
    kdDebug () << "\tpixmap=" << pixmap
               << " border: rect=" << border.rect ()
               << " isSingleColor=" << border.isSingleColor ()
               << endl;
#endif

    if (!doc)
        return;

    if (pixmap && border.exists () && !border.isSingleColor ())
    {
        if (*pixmap)
        {
        #if DEBUG_KP_TOOL_AUTO_CROP && 1
            kdDebug () << "\talready have *pixmap - delete it" << endl;
        #endif
            delete *pixmap;
        }

        *pixmap = new QPixmap (
            kpPixmapFX::getPixmapAt (*doc->pixmap (m_actOnSelection),
                                     border.rect ()));
    }
}


// private
void kpToolAutoCropCommand::getUndoPixmaps ()
{
    getUndoPixmap (m_leftBorder, &m_leftPixmap);
    getUndoPixmap (m_rightBorder, &m_rightPixmap);
    getUndoPixmap (m_topBorder, &m_topPixmap);
    getUndoPixmap (m_botBorder, &m_botPixmap);
}

// private
void kpToolAutoCropCommand::deleteUndoPixmaps ()
{
#if DEBUG_KP_TOOL_AUTO_CROP && 1
    kdDebug () << "kpToolAutoCropCommand::deleteUndoPixmaps()" << endl;
#endif

    delete m_leftPixmap; m_leftPixmap = 0;
    delete m_rightPixmap; m_rightPixmap = 0;
    delete m_topPixmap; m_topPixmap = 0;
    delete m_botPixmap; m_botPixmap = 0;
}


// public virtual [base KCommand]
void kpToolAutoCropCommand::execute ()
{
    if (!m_contentsRect.isValid ())
        m_contentsRect = contentsRect ();


    getUndoPixmaps ();


    kpDocument *doc = document ();
    if (!doc)
        return;


    QPixmap pixmapWithoutBorder =
        kpTool::neededPixmap (*doc->pixmap (m_actOnSelection),
                              m_contentsRect);


    if (!m_actOnSelection)
        doc->setPixmap (pixmapWithoutBorder);
    else
    {
        m_oldSelection = *doc->selection ();
        m_oldSelection.setPixmap (QPixmap ());

        // m_contentsRect is relative to the top of the sel
        // while sel is relative to the top of the doc
        QRect rect = m_contentsRect;
        rect.moveBy (m_oldSelection.x (), m_oldSelection.y ());

        kpSelection sel (kpSelection::Rectangle,
                         rect,
                         pixmapWithoutBorder,
                         m_oldSelection.transparency ());

        doc->setSelection (sel);
    }
}

// public virtual [base KCommand]
void kpToolAutoCropCommand::unexecute ()
{
#if DEBUG_KP_TOOL_AUTO_CROP && 1
    kdDebug () << "kpToolAutoCropCommand::unexecute()" << endl;
#endif

    kpDocument *doc = document ();
    if (!doc)
        return;

    QPixmap pixmap (m_oldWidth, m_oldHeight);
    QBitmap maskBitmap;

    // restore the position of the centre image
    kpPixmapFX::setPixmapAt (&pixmap, m_contentsRect,
                             *doc->pixmap (m_actOnSelection));

    // draw the borders

    QPainter painter (&pixmap);
    QPainter maskPainter;

    const kpToolAutoCropBorder *borders [] =
    {
        &m_leftBorder, &m_rightBorder,
        &m_topBorder, &m_botBorder,
        0
    };

    const QPixmap *pixmaps [] =
    {
        m_leftPixmap, m_rightPixmap,
        m_topPixmap, m_botPixmap,
        0
    };

    const QPixmap **p = pixmaps;
    for (const kpToolAutoCropBorder **b = borders; *b; b++, p++)
    {
        if (!(*b)->exists ())
            continue;

        if ((*b)->isSingleColor ())
        {
            kpColor col = (*b)->referenceColor ();
        #if DEBUG_KP_TOOL_AUTO_CROP && 1
            kdDebug () << "\tdrawing border " << (*b)->rect ()
                       << " rgb=" << (int *) col.toQRgb () /* %X hack */ << endl;
        #endif

            if (col.isOpaque ())
            {
                painter.fillRect ((*b)->rect (), col.toQColor ());
            }
            else
            {
                if (maskBitmap.isNull ())
                {
                    // TODO: dangerous when a painter is active on pixmap?
                    maskBitmap = kpPixmapFX::getNonNullMask (pixmap);
                    maskPainter.begin (&maskBitmap);
                }

                maskPainter.fillRect ((*b)->rect (), Qt::color0/*transparent*/);
            }
        }
        else
        {
        #if DEBUG_KP_TOOL_AUTO_CROP && 1
            kdDebug () << "\trestoring border pixmap " << (*b)->rect () << endl;
        #endif
            if (*p)
                painter.drawPixmap ((*b)->rect (), **p);
        }
    }

    if (maskPainter.isActive ())
        maskPainter.end ();

    painter.end ();

    if (!maskBitmap.isNull ())
        pixmap.setMask (maskBitmap);


    if (!m_actOnSelection)
        doc->setPixmap (pixmap);
    else
    {
        kpSelection sel = m_oldSelection;
        sel.setPixmap (pixmap);

        doc->setSelection (sel);
    }


    deleteUndoPixmaps ();
}


// private
QRect kpToolAutoCropCommand::contentsRect () const
{
    const QPixmap *pixmap = document ()->pixmap (m_actOnSelection);

    QPoint topLeft (m_leftBorder.exists () ?
                        m_leftBorder.rect ().right () + 1 :
                        0,
                    m_topBorder.exists () ?
                        m_topBorder.rect ().bottom () + 1 :
                        0);
    QPoint botRight (m_rightBorder.exists () ?
                         m_rightBorder.rect ().left () - 1 :
                         pixmap->width () - 1,
                     m_botBorder.exists () ?
                         m_botBorder.rect ().top () - 1 :
                         pixmap->height () - 1);

    return QRect (topLeft, botRight);
}
