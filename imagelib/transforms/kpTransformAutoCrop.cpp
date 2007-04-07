
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

#define DEBUG_KP_TOOL_AUTO_CROP 0


#include <kpTransformAutoCrop.h>

#include <qapplication.h>
#include <qbitmap.h>
#include <qimage.h>
#include <qpainter.h>
#include <qpixmap.h>

#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>

#include <kpBug.h>
#include <kpColorToolBar.h>
#include <kpCommandHistory.h>
#include <kpDocument.h>
#include <kpMainWindow.h>
#include <kpPixmapFX.h>
#include <kpSelection.h>
#include <kpSetOverrideCursorSaver.h>
#include <kpTool.h>
#include <kpViewManager.h>


class kpTransformAutoCropBorder
{
public:
    // WARNING: Only call the <ctor> with pixmapPtr = 0 if you are going to use
    //          operator= to fill it in with a valid pixmapPtr immediately
    //          afterwards.
    kpTransformAutoCropBorder (const QPixmap *pixmapPtr = 0, int processedColorSimilarity = 0);

    int size () const;

    const QPixmap *pixmap () const;
    int processedColorSimilarity () const;
    QRect rect () const;
    int left () const;
    int right () const;
    int top () const;
    int bottom () const;
    kpColor referenceColor () const;
    kpColor averageColor () const;
    bool isSingleColor () const;

    // (returns true on success (even if no rect) or false on error)
    bool calculate (int isX, int dir);

    bool fillsEntirePixmap () const;
    bool exists () const;
    void invalidate ();

private:
    const QPixmap *m_pixmapPtr;
    int m_processedColorSimilarity;

    QRect m_rect;
    kpColor m_referenceColor;
    int m_redSum, m_greenSum, m_blueSum;
    bool m_isSingleColor;
};

kpTransformAutoCropBorder::kpTransformAutoCropBorder (const QPixmap *pixmapPtr,
                                            int processedColorSimilarity)
    : m_pixmapPtr (pixmapPtr),
      m_processedColorSimilarity (processedColorSimilarity)
{
    invalidate ();
}


// public
int kpTransformAutoCropBorder::size () const
{
    return sizeof (kpTransformAutoCropBorder);
}


// public
const QPixmap *kpTransformAutoCropBorder::pixmap () const
{
    return m_pixmapPtr;
}

// public
int kpTransformAutoCropBorder::processedColorSimilarity () const
{
    return m_processedColorSimilarity;
}

// public
QRect kpTransformAutoCropBorder::rect () const
{
    return m_rect;
}

// public
int kpTransformAutoCropBorder::left () const
{
    return m_rect.left ();
}

// public
int kpTransformAutoCropBorder::right () const
{
    return m_rect.right ();
}

// public
int kpTransformAutoCropBorder::top () const
{
    return m_rect.top ();
}

// public
int kpTransformAutoCropBorder::bottom () const
{
    return m_rect.bottom ();
}

// public
kpColor kpTransformAutoCropBorder::referenceColor () const
{
    return m_referenceColor;
}

// public
kpColor kpTransformAutoCropBorder::averageColor () const
{
    if (!m_rect.isValid ())
        return kpColor::Invalid;

    if (m_referenceColor.isTransparent ())
        return kpColor::Transparent;
    else if (m_processedColorSimilarity == 0)
        return m_referenceColor;
    else
    {
        int numPixels = (m_rect.width () * m_rect.height ());
        Q_ASSERT (numPixels > 0);

        return kpColor (m_redSum / numPixels,
                        m_greenSum / numPixels,
                        m_blueSum / numPixels);
    }
}

bool kpTransformAutoCropBorder::isSingleColor () const
{
    return m_isSingleColor;
}


// public
bool kpTransformAutoCropBorder::calculate (int isX, int dir)
{
#if DEBUG_KP_TOOL_AUTO_CROP && 1
    kDebug () << "kpTransformAutoCropBorder::calculate() CALLED!" << endl;
#endif
    int maxX = m_pixmapPtr->width () - 1;
    int maxY = m_pixmapPtr->height () - 1;

    QImage image = kpPixmapFX::convertToImage (*m_pixmapPtr);
    Q_ASSERT (!image.isNull ());

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
            m_rect = kpBug::QRect_Normalized (
                QRect (QPoint (startX, 0),
                       QPoint (startX + (numCols - 1) * dir, maxY)));
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
            m_rect = kpBug::QRect_Normalized (
                QRect (QPoint (0, startY),
                       QPoint (maxX, startY + (numRows - 1) * dir)));
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
bool kpTransformAutoCropBorder::fillsEntirePixmap () const
{
    return (m_rect == m_pixmapPtr->rect ());
}

// public
bool kpTransformAutoCropBorder::exists () const
{
    // (will use in an addition so make sure returns 1 or 0)
    return (m_rect.isValid () ? 1 : 0);
}

// public
void kpTransformAutoCropBorder::invalidate ()
{
    m_rect = QRect ();
    m_referenceColor = kpColor::Invalid;
    m_redSum = m_greenSum = m_blueSum = 0;
    m_isSingleColor = false;
}


struct kpTransformAutoCropCommandPrivate
{
    bool actOnSelection;
    kpTransformAutoCropBorder leftBorder, rightBorder, topBorder, botBorder;
    QPixmap *leftPixmap, *rightPixmap, *topPixmap, *botPixmap;

    QRect contentsRect;
    int oldWidth, oldHeight;
    kpSelection oldSelection;
};

kpTransformAutoCropCommand::kpTransformAutoCropCommand (bool actOnSelection,
                                              const kpTransformAutoCropBorder &leftBorder,
                                              const kpTransformAutoCropBorder &rightBorder,
                                              const kpTransformAutoCropBorder &topBorder,
                                              const kpTransformAutoCropBorder &botBorder,
                                              kpMainWindow *mainWindow)
    : kpNamedCommand (name (actOnSelection, DontShowAccel), mainWindow),
      d (new kpTransformAutoCropCommandPrivate ())
{
    d->actOnSelection = actOnSelection;
    d->leftBorder = leftBorder;
    d->rightBorder = rightBorder;
    d->topBorder = topBorder;
    d->botBorder = botBorder;
    d->leftPixmap = 0;
    d->rightPixmap = 0;
    d->topPixmap = 0;
    d->botPixmap = 0;
      
    kpDocument *doc = document ();
    Q_ASSERT (doc);

    d->oldWidth = doc->width (d->actOnSelection);
    d->oldHeight = doc->height (d->actOnSelection);
}

kpTransformAutoCropCommand::~kpTransformAutoCropCommand ()
{
    deleteUndoPixmaps ();

    delete d;
}


// public static
QString kpTransformAutoCropCommand::name (bool actOnSelection, int options)
{
    if (actOnSelection)
    {
        if (options & kpTransformAutoCropCommand::ShowAccel)
            return i18n ("Remove Internal B&order");
        else
            return i18n ("Remove Internal Border");
    }
    else
    {
        if (options & kpTransformAutoCropCommand::ShowAccel)
            return i18n ("Autocr&op");
        else
            return i18n ("Autocrop");
    }
}


// public virtual [base kpCommand]
int kpTransformAutoCropCommand::size () const
{
    return d->leftBorder.size () +
           d->rightBorder.size () +
           d->topBorder.size () +
           d->botBorder.size () +
           kpPixmapFX::pixmapSize (d->leftPixmap) +
           kpPixmapFX::pixmapSize (d->rightPixmap) +
           kpPixmapFX::pixmapSize (d->topPixmap) +
           kpPixmapFX::pixmapSize (d->botPixmap) +
           d->oldSelection.size ();
}


// private
void kpTransformAutoCropCommand::getUndoPixmap (const kpTransformAutoCropBorder &border, QPixmap **pixmap)
{
    kpDocument *doc = document ();
    Q_ASSERT (doc);

#if DEBUG_KP_TOOL_AUTO_CROP && 1
    kDebug () << "kpTransformAutoCropCommand::getUndoPixmap()" << endl;
    kDebug () << "\tpixmap=" << pixmap
               << " border: rect=" << border.rect ()
               << " isSingleColor=" << border.isSingleColor ()
               << endl;
#endif

    if (pixmap && border.exists () && !border.isSingleColor ())
    {
        if (*pixmap)
        {
        #if DEBUG_KP_TOOL_AUTO_CROP && 1
            kDebug () << "\talready have *pixmap - delete it" << endl;
        #endif
            delete *pixmap;
        }

        *pixmap = new QPixmap (
            kpPixmapFX::getPixmapAt (*doc->pixmap (d->actOnSelection),
                                     border.rect ()));
    }
}


// private
void kpTransformAutoCropCommand::getUndoPixmaps ()
{
    getUndoPixmap (d->leftBorder, &d->leftPixmap);
    getUndoPixmap (d->rightBorder, &d->rightPixmap);
    getUndoPixmap (d->topBorder, &d->topPixmap);
    getUndoPixmap (d->botBorder, &d->botPixmap);
}

// private
void kpTransformAutoCropCommand::deleteUndoPixmaps ()
{
#if DEBUG_KP_TOOL_AUTO_CROP && 1
    kDebug () << "kpTransformAutoCropCommand::deleteUndoPixmaps()" << endl;
#endif

    delete d->leftPixmap; d->leftPixmap = 0;
    delete d->rightPixmap; d->rightPixmap = 0;
    delete d->topPixmap; d->topPixmap = 0;
    delete d->botPixmap; d->botPixmap = 0;
}


// public virtual [base kpCommand]
void kpTransformAutoCropCommand::execute ()
{
    if (!d->contentsRect.isValid ())
        d->contentsRect = contentsRect ();


    getUndoPixmaps ();


    kpDocument *doc = document ();
    Q_ASSERT (doc);


    QPixmap pixmapWithoutBorder =
        kpTool::neededPixmap (*doc->pixmap (d->actOnSelection),
                              d->contentsRect);


    if (!d->actOnSelection)
        doc->setPixmap (pixmapWithoutBorder);
    else
    {
        d->oldSelection = *doc->selection ();
        d->oldSelection.setPixmap (QPixmap ());

        // d->contentsRect is relative to the top of the sel
        // while sel is relative to the top of the doc
        QRect rect = d->contentsRect;
        rect.translate (d->oldSelection.x (), d->oldSelection.y ());

        kpSelection sel (kpSelection::Rectangle,
                         rect,
                         pixmapWithoutBorder,
                         d->oldSelection.transparency ());

        doc->setSelection (sel);

        if (mainWindow ()->tool ())
            mainWindow ()->tool ()->somethingBelowTheCursorChanged ();
    }
}

// public virtual [base kpCommand]
void kpTransformAutoCropCommand::unexecute ()
{
#if DEBUG_KP_TOOL_AUTO_CROP && 1
    kDebug () << "kpTransformAutoCropCommand::unexecute()" << endl;
#endif

    kpDocument *doc = document ();
    Q_ASSERT (doc);

    QPixmap pixmap (d->oldWidth, d->oldHeight);
    QBitmap maskBitmap;

    // restore the position of the center image
    kpPixmapFX::setPixmapAt (&pixmap, d->contentsRect,
                             *doc->pixmap (d->actOnSelection));

    // draw the borders

    QPainter painter (&pixmap);
    QPainter maskPainter;

    const kpTransformAutoCropBorder *borders [] =
    {
        &d->leftBorder, &d->rightBorder,
        &d->topBorder, &d->botBorder,
        0
    };

    const QPixmap *pixmaps [] =
    {
        d->leftPixmap, d->rightPixmap,
        d->topPixmap, d->botPixmap,
        0
    };

    const QPixmap **p = pixmaps;
    for (const kpTransformAutoCropBorder **b = borders; *b; b++, p++)
    {
        if (!(*b)->exists ())
            continue;

        if ((*b)->isSingleColor ())
        {
            kpColor col = (*b)->referenceColor ();
        #if DEBUG_KP_TOOL_AUTO_CROP && 1
            kDebug () << "\tdrawing border " << (*b)->rect ()
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
            kDebug () << "\trestoring border pixmap " << (*b)->rect () << endl;
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


    if (!d->actOnSelection)
        doc->setPixmap (pixmap);
    else
    {
        kpSelection sel = d->oldSelection;
        sel.setPixmap (pixmap);

        doc->setSelection (sel);

        if (mainWindow ()->tool ())
            mainWindow ()->tool ()->somethingBelowTheCursorChanged ();
    }


    deleteUndoPixmaps ();
}


// private
QRect kpTransformAutoCropCommand::contentsRect () const
{
    const QPixmap *pixmap = document ()->pixmap (d->actOnSelection);

    QPoint topLeft (d->leftBorder.exists () ?
                        d->leftBorder.rect ().right () + 1 :
                        0,
                    d->topBorder.exists () ?
                        d->topBorder.rect ().bottom () + 1 :
                        0);
    QPoint botRight (d->rightBorder.exists () ?
                         d->rightBorder.rect ().left () - 1 :
                         pixmap->width () - 1,
                     d->botBorder.exists () ?
                         d->botBorder.rect ().top () - 1 :
                         pixmap->height () - 1);

    return QRect (topLeft, botRight);
}


static void ShowNothingToAutocropMessage (kpMainWindow *mainWindow, bool actOnSelection)
{
    kpSetOverrideCursorSaver cursorSaver (Qt::ArrowCursor);

    if (actOnSelection)
    {
        KMessageBox::information (mainWindow,
            i18n ("KolourPaint cannot remove the selection's internal border as it"
                  " could not be located."),
            i18n ("Cannot Remove Internal Border"),
            "NothingToAutoCrop");
    }
    else
    {
        KMessageBox::information (mainWindow,
            i18n ("KolourPaint cannot automatically crop the image as its"
                  " border could not be located."),
            i18n ("Cannot Autocrop"),
            "NothingToAutoCrop");
    }
}

bool kpTransformAutoCrop (kpMainWindow *mainWindow)
{
#if DEBUG_KP_TOOL_AUTO_CROP
    kDebug () << "kpTransformAutoCrop() CALLED!" << endl;
#endif

    Q_ASSERT (mainWindow);
    kpDocument *doc = mainWindow->document ();
    Q_ASSERT (doc);

    // OPT: if already pulled selection pixmap, no need to do it again here
    QPixmap pixmap = doc->selection () ? doc->getSelectedPixmap () : *doc->pixmap ();
    Q_ASSERT (!pixmap.isNull ());

    kpViewManager *vm = mainWindow->viewManager ();
    Q_ASSERT (vm);

    int processedColorSimilarity = mainWindow->colorToolBar ()->processedColorSimilarity ();
    kpTransformAutoCropBorder leftBorder (&pixmap, processedColorSimilarity),
                         rightBorder (&pixmap, processedColorSimilarity),
                         topBorder (&pixmap, processedColorSimilarity),
                         botBorder (&pixmap, processedColorSimilarity);


    kpSetOverrideCursorSaver cursorSaver (Qt::WaitCursor);

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
        kDebug () << "\tcan't find border; leftBorder.rect=" << leftBorder.rect ()
                   << " rightBorder.rect=" << rightBorder.rect ()
                   << " topBorder.rect=" << topBorder.rect ()
                   << " botBorder.rect=" << botBorder.rect ()
                   << endl;
    #endif
        ::ShowNothingToAutocropMessage (mainWindow, (bool) doc->selection ());
        return false;
    }

#if DEBUG_KP_TOOL_AUTO_CROP
    kDebug () << "\tnumRegions=" << numRegions << endl;
    kDebug () << "\t\tleft=" << leftBorder.rect ()
               << " refCol=" << (leftBorder.exists () ? (int *) leftBorder.referenceColor ().toQRgb () : 0)
               << " avgCol=" << (leftBorder.exists () ? (int *) leftBorder.averageColor ().toQRgb () : 0)
               << endl;
    kDebug () << "\t\tright=" << rightBorder.rect ()
               << " refCol=" << (rightBorder.exists () ? (int *) rightBorder.referenceColor ().toQRgb () : 0)
               << " avgCol=" << (rightBorder.exists () ? (int *) rightBorder.averageColor ().toQRgb () : 0)
               << endl;
    kDebug () << "\t\ttop=" << topBorder.rect ()
               << " refCol=" << (topBorder.exists () ? (int *) topBorder.referenceColor ().toQRgb () : 0)
               << " avgCol=" << (topBorder.exists () ? (int *) topBorder.averageColor ().toQRgb () : 0)
               << endl;
    kDebug () << "\t\tbot=" << botBorder.rect ()
               << " refCol=" << (botBorder.exists () ? (int *) botBorder.referenceColor ().toQRgb () : 0)
               << " avgCol=" << (botBorder.exists () ? (int *) botBorder.averageColor ().toQRgb () : 0)
               << endl;
#endif


    // In case e.g. the user pastes a solid, coloured-in rectangle,
    // we favor killing the bottom and right regions
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
            kDebug () << "\tignoring left border" << endl;
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
            kDebug () << "\tignoring top border" << endl;
        #endif
            topBorder.invalidate ();
        }
    }


    mainWindow->addImageOrSelectionCommand (
        new kpTransformAutoCropCommand (
            (bool) doc->selection (),
            leftBorder, rightBorder,
            topBorder, botBorder,
            mainWindow));


    return true;
}
