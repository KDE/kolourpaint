
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


#include "kpTransformAutoCrop.h"

#include "layers/selections/image/kpAbstractImageSelection.h"
#include "widgets/toolbars/kpColorToolBar.h"
#include "environments/commands/kpCommandEnvironment.h"
#include "commands/kpCommandHistory.h"
#include "document/kpDocument.h"
#include "mainWindow/kpMainWindow.h"
#include "imagelib/kpPainter.h"
#include "pixmapfx/kpPixmapFX.h"
#include "layers/selections/image/kpRectangularImageSelection.h"
#include "generic/kpSetOverrideCursorSaver.h"
#include "tools/kpTool.h"
#include "views/manager/kpViewManager.h"

#include "kpLogCategories.h"
#include <KMessageBox>
#include <KLocalizedString>

#include <QImage>

//---------------------------------------------------------------------

class kpTransformAutoCropBorder
{
public:
    // WARNING: Only call the <ctor> with imagePtr = 0 if you are going to use
    //          operator= to fill it in with a valid imagePtr immediately
    //          afterwards.
    kpTransformAutoCropBorder (const kpImage *imagePtr = nullptr, int processedColorSimilarity = 0);

    kpCommandSize::SizeType size () const;

    const kpImage *image () const;
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

    bool fillsEntireImage () const;
    bool exists () const;
    void invalidate ();

private:
    const kpImage *m_imagePtr;
    int m_processedColorSimilarity;

    QRect m_rect;
    kpColor m_referenceColor;
    int m_redSum, m_greenSum, m_blueSum;
    bool m_isSingleColor;
};

kpTransformAutoCropBorder::kpTransformAutoCropBorder (const kpImage *imagePtr,
                                            int processedColorSimilarity)
    : m_imagePtr (imagePtr),
      m_processedColorSimilarity (processedColorSimilarity)
{
    invalidate ();
}


// public
kpCommandSize::SizeType kpTransformAutoCropBorder::size () const
{
    return sizeof (kpTransformAutoCropBorder);
}


// public
const kpImage *kpTransformAutoCropBorder::image () const
{
    return m_imagePtr;
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

    if (m_processedColorSimilarity == 0)
        return m_referenceColor;

    int numPixels = (m_rect.width () * m_rect.height ());
    Q_ASSERT (numPixels > 0);

    return kpColor (m_redSum / numPixels, m_greenSum / numPixels, m_blueSum / numPixels);

}

//---------------------------------------------------------------------

bool kpTransformAutoCropBorder::isSingleColor () const
{
    return m_isSingleColor;
}

//---------------------------------------------------------------------

// public
bool kpTransformAutoCropBorder::calculate (int isX, int dir)
{
#if DEBUG_KP_TOOL_AUTO_CROP && 1
    qCDebug(kpLogImagelib) << "kpTransformAutoCropBorder::calculate() CALLED!";
#endif
    int maxX = m_imagePtr->width () - 1;
    int maxY = m_imagePtr->height () - 1;

    QImage qimage = *m_imagePtr;
    Q_ASSERT (!qimage.isNull ());

    // (sync both branches)
    if (isX)
    {
        int numCols = 0;
        int startX = (dir > 0) ? 0 : maxX;

        kpColor col = kpPixmapFX::getColorAtPixel (qimage, startX, 0);
        for (int x = startX;
             x >= 0 && x <= maxX;
             x += dir)
        {
            int y;
            for (y = 0; y <= maxY; y++)
            {
                if (!kpPixmapFX::getColorAtPixel (qimage, x, y).isSimilarTo (col, m_processedColorSimilarity))
                    break;
            }

            if (y <= maxY)
                break;
            else
                numCols++;
        }

        if (numCols)
        {
            m_rect =
                kpPainter::normalizedRect(QPoint(startX, 0),
                       QPoint(startX + (numCols - 1) * dir, maxY));
            m_referenceColor = col;
        }
    }
    else
    {
        int numRows = 0;
        int startY = (dir > 0) ? 0 : maxY;

        kpColor col = kpPixmapFX::getColorAtPixel (qimage, 0, startY);
        for (int y = startY;
             y >= 0 && y <= maxY;
             y += dir)
        {
            int x;
            for (x = 0; x <= maxX; x++)
            {
                if (!kpPixmapFX::getColorAtPixel (qimage, x, y).isSimilarTo (col, m_processedColorSimilarity))
                    break;
            }

            if (x <= maxX)
                break;
            else
                numRows++;
        }

        if (numRows)
        {
            m_rect = kpPainter::normalizedRect(QPoint(0, startY),
                         QPoint(maxX, startY + (numRows - 1) * dir));
            m_referenceColor = col;
        }
    }


    if (m_rect.isValid ())
    {
        m_isSingleColor = true;

        if (m_processedColorSimilarity != 0)
        {
            for (int y = m_rect.top (); y <= m_rect.bottom (); y++)
            {
                for (int x = m_rect.left (); x <= m_rect.right (); x++)
                {
                    kpColor colAtPixel = kpPixmapFX::getColorAtPixel (qimage, x, y);

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
bool kpTransformAutoCropBorder::fillsEntireImage () const
{
    return (m_rect == m_imagePtr->rect ());
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
    bool actOnSelection{};
    kpTransformAutoCropBorder leftBorder, rightBorder, topBorder, botBorder;
    kpImage *leftImage{}, *rightImage{}, *topImage{}, *botImage{};

    QRect contentsRect;
    int oldWidth{}, oldHeight{};
    kpAbstractImageSelection *oldSelectionPtr{};
};

// REFACTOR: Move to /commands/
kpTransformAutoCropCommand::kpTransformAutoCropCommand (bool actOnSelection,
        const kpTransformAutoCropBorder &leftBorder,
        const kpTransformAutoCropBorder &rightBorder,
        const kpTransformAutoCropBorder &topBorder,
        const kpTransformAutoCropBorder &botBorder,
        kpCommandEnvironment *environ)
    : kpNamedCommand(text(actOnSelection, DontShowAccel), environ),
      d (new kpTransformAutoCropCommandPrivate ())
{
    d->actOnSelection = actOnSelection;
    d->leftBorder = leftBorder;
    d->rightBorder = rightBorder;
    d->topBorder = topBorder;
    d->botBorder = botBorder;
    d->leftImage = nullptr;
    d->rightImage = nullptr;
    d->topImage = nullptr;
    d->botImage = nullptr;

    kpDocument *doc = document ();
    Q_ASSERT (doc);

    d->oldWidth = doc->width (d->actOnSelection);
    d->oldHeight = doc->height (d->actOnSelection);

    d->oldSelectionPtr = nullptr;
}

//---------------------------------------------------------------------

kpTransformAutoCropCommand::~kpTransformAutoCropCommand ()
{
    deleteUndoImages ();

    delete d->oldSelectionPtr;
    delete d;
}

//---------------------------------------------------------------------
// public static

QString kpTransformAutoCropCommand::text(bool actOnSelection, int options)
{
    if (actOnSelection)
    {
        if (options & kpTransformAutoCropCommand::ShowAccel) {
            return i18n ("Remove Internal B&order");
        }

        return i18n ("Remove Internal Border");
    }

    if (options & kpTransformAutoCropCommand::ShowAccel)
        return i18n ("Autocr&op");

    return i18n ("Autocrop");
}

//---------------------------------------------------------------------
// public virtual [base kpCommand]

kpCommandSize::SizeType kpTransformAutoCropCommand::size () const
{
    return d->leftBorder.size () +
           d->rightBorder.size () +
           d->topBorder.size () +
           d->botBorder.size () +
           ImageSize (d->leftImage) +
           ImageSize (d->rightImage) +
           ImageSize (d->topImage) +
           ImageSize (d->botImage) +
           SelectionSize (d->oldSelectionPtr);
}

//---------------------------------------------------------------------
// private

void kpTransformAutoCropCommand::getUndoImage (const kpTransformAutoCropBorder &border, kpImage **image)
{
    kpDocument *doc = document ();
    Q_ASSERT (doc);

#if DEBUG_KP_TOOL_AUTO_CROP && 1
    qCDebug(kpLogImagelib) << "kpTransformAutoCropCommand::getUndoImage()";
    qCDebug(kpLogImagelib) << "\timage=" << image
               << " border: rect=" << border.rect ()
               << " isSingleColor=" << border.isSingleColor ();
#endif

    if (image && border.exists () && !border.isSingleColor ())
    {
        if (*image)
        {
        #if DEBUG_KP_TOOL_AUTO_CROP && 1
            qCDebug(kpLogImagelib) << "\talready have *image - delete it";
        #endif
            delete *image;
        }

        *image = new kpImage (
            kpPixmapFX::getPixmapAt (doc->image (d->actOnSelection),
                                     border.rect ()));
    }
}


// private
void kpTransformAutoCropCommand::getUndoImages ()
{
    getUndoImage (d->leftBorder, &d->leftImage);
    getUndoImage (d->rightBorder, &d->rightImage);
    getUndoImage (d->topBorder, &d->topImage);
    getUndoImage (d->botBorder, &d->botImage);
}

// private
void kpTransformAutoCropCommand::deleteUndoImages ()
{
#if DEBUG_KP_TOOL_AUTO_CROP && 1
    qCDebug(kpLogImagelib) << "kpTransformAutoCropCommand::deleteUndoImages()";
#endif

    delete d->leftImage; d->leftImage = nullptr;
    delete d->rightImage; d->rightImage = nullptr;
    delete d->topImage; d->topImage = nullptr;
    delete d->botImage; d->botImage = nullptr;
}


// public virtual [base kpCommand]
void kpTransformAutoCropCommand::execute ()
{
    if (!d->contentsRect.isValid ()) {
        d->contentsRect = contentsRect ();
    }


    getUndoImages ();


    kpDocument *doc = document ();
    Q_ASSERT (doc);


    kpImage imageWithoutBorder =
        kpTool::neededPixmap (doc->image (d->actOnSelection),
                              d->contentsRect);


    if (!d->actOnSelection) {
        doc->setImage (imageWithoutBorder);
    }
    else {
        d->oldSelectionPtr = doc->imageSelection ()->clone ();
        d->oldSelectionPtr->setBaseImage (kpImage ());

        // d->contentsRect is relative to the top of the sel
        // while sel is relative to the top of the doc
        QRect rect = d->contentsRect;
        rect.translate (d->oldSelectionPtr->x (), d->oldSelectionPtr->y ());

        kpRectangularImageSelection sel (
            rect,
            imageWithoutBorder,
            d->oldSelectionPtr->transparency ());

        doc->setSelection (sel);

        environ ()->somethingBelowTheCursorChanged ();
    }
}

// public virtual [base kpCommand]
void kpTransformAutoCropCommand::unexecute ()
{
#if DEBUG_KP_TOOL_AUTO_CROP && 1
    qCDebug(kpLogImagelib) << "kpTransformAutoCropCommand::unexecute()";
#endif

    kpDocument *doc = document ();
    Q_ASSERT (doc);

    kpImage image (d->oldWidth, d->oldHeight, QImage::Format_ARGB32_Premultiplied);

    // restore the position of the center image
    kpPixmapFX::setPixmapAt (&image, d->contentsRect,
        doc->image (d->actOnSelection));

    // draw the borders

    const kpTransformAutoCropBorder *borders [] =
    {
        &d->leftBorder, &d->rightBorder,
        &d->topBorder, &d->botBorder,
        nullptr
    };

    const kpImage *images [] =
    {
        d->leftImage, d->rightImage,
        d->topImage, d->botImage,
        nullptr
    };

    const kpImage **p = images;
    for (const kpTransformAutoCropBorder **b = borders; *b; b++, p++)
    {
        if (!(*b)->exists ()) {
            continue;
        }

        if ((*b)->isSingleColor ())
        {
            kpColor col = (*b)->referenceColor ();
        #if DEBUG_KP_TOOL_AUTO_CROP && 1
            qCDebug(kpLogImagelib) << "\tdrawing border " << (*b)->rect ()
                       << " rgb=" << (int *) col.toQRgb () /* %X hack */;
        #endif

            const QRect r = (*b)->rect ();
            kpPainter::fillRect (&image,
                r.x (), r.y (), r.width (), r.height (),
                col);
        }
        else
        {
        #if DEBUG_KP_TOOL_AUTO_CROP && 1
            qCDebug(kpLogImagelib) << "\trestoring border image " << (*b)->rect ();
        #endif
            if (*p)
            {
                // REFACTOR: Add equivalent method to kpPainter and use.
                kpPixmapFX::setPixmapAt (&image, (*b)->rect (), **p);
            }
        }
    }


    if (!d->actOnSelection) {
        doc->setImage (image);
    }
    else
    {
        d->oldSelectionPtr->setBaseImage (image);

        doc->setSelection (*d->oldSelectionPtr);
        delete d->oldSelectionPtr; d->oldSelectionPtr = nullptr;

        environ ()->somethingBelowTheCursorChanged ();
    }


    deleteUndoImages ();
}


// private
QRect kpTransformAutoCropCommand::contentsRect () const
{
    const kpImage image = document ()->image (d->actOnSelection);

    QPoint topLeft (d->leftBorder.exists () ?
                        d->leftBorder.rect ().right () + 1 :
                        0,
                    d->topBorder.exists () ?
                        d->topBorder.rect ().bottom () + 1 :
                        0);
    QPoint botRight (d->rightBorder.exists () ?
                         d->rightBorder.rect ().left () - 1 :
                         image.width () - 1,
                     d->botBorder.exists () ?
                         d->botBorder.rect ().top () - 1 :
                         image.height () - 1);

    return {topLeft, botRight};
}


static void ShowNothingToAutocropMessage (kpMainWindow *mainWindow, bool actOnSelection)
{
    kpSetOverrideCursorSaver cursorSaver (Qt::ArrowCursor);

    if (actOnSelection)
    {
        KMessageBox::information (mainWindow,
            i18n ("KolourPaint cannot remove the selection's internal border as it"
                  " could not be located."),
            i18nc ("@title:window", "Cannot Remove Internal Border"),
            QStringLiteral("NothingToAutoCrop"));
    }
    else
    {
        KMessageBox::information (mainWindow,
            i18n ("KolourPaint cannot automatically crop the image as its"
                  " border could not be located."),
            i18nc ("@title:window", "Cannot Autocrop"),
            QStringLiteral("NothingToAutoCrop"));
    }
}

bool kpTransformAutoCrop (kpMainWindow *mainWindow)
{
#if DEBUG_KP_TOOL_AUTO_CROP
    qCDebug(kpLogImagelib) << "kpTransformAutoCrop() CALLED!";
#endif

    Q_ASSERT (mainWindow);
    kpDocument *doc = mainWindow->document ();
    Q_ASSERT (doc);

    // OPT: if already pulled selection image, no need to do it again here
    kpImage image = doc->selection () ? doc->getSelectedBaseImage () : doc->image ();
    Q_ASSERT (!image.isNull ());

    kpViewManager *vm = mainWindow->viewManager ();
    Q_ASSERT (vm);

    int processedColorSimilarity = mainWindow->colorToolBar ()->processedColorSimilarity ();
    kpTransformAutoCropBorder leftBorder (&image, processedColorSimilarity),
                         rightBorder (&image, processedColorSimilarity),
                         topBorder (&image, processedColorSimilarity),
                         botBorder (&image, processedColorSimilarity);


    kpSetOverrideCursorSaver cursorSaver (Qt::WaitCursor);

    mainWindow->colorToolBar ()->flashColorSimilarityToolBarItem ();

    // TODO: With Colour Similarity, a lot of weird (and wonderful) things can
    //       happen resulting in a huge number of code paths.  Needs refactoring
    //       and regression testing.
    //
    // TODO: e.g. When the top fills entire rect but bot doesn't we could
    //       invalidate top and continue autocrop.
    int numRegions = 0;
    if (!leftBorder.calculate (true/*x*/, +1/*going right*/) ||
        leftBorder.fillsEntireImage () ||
        !rightBorder.calculate (true/*x*/, -1/*going left*/) ||
        rightBorder.fillsEntireImage () ||
        !topBorder.calculate (false/*y*/, +1/*going down*/) ||
        topBorder.fillsEntireImage () ||
        !botBorder.calculate (false/*y*/, -1/*going up*/) ||
        botBorder.fillsEntireImage () ||
        ((numRegions = leftBorder.exists () +
                       rightBorder.exists () +
                       topBorder.exists () +
                       botBorder.exists ()) == 0))
    {
    #if DEBUG_KP_TOOL_AUTO_CROP
        qCDebug(kpLogImagelib) << "\tcan't find border; leftBorder.rect=" << leftBorder.rect ()
                   << " rightBorder.rect=" << rightBorder.rect ()
                   << " topBorder.rect=" << topBorder.rect ()
                   << " botBorder.rect=" << botBorder.rect ();
    #endif
        ::ShowNothingToAutocropMessage (mainWindow, static_cast<bool> (doc->selection ()));
        return false;
    }

#if DEBUG_KP_TOOL_AUTO_CROP
    qCDebug(kpLogImagelib) << "\tnumRegions=" << numRegions;
    qCDebug(kpLogImagelib) << "\t\tleft=" << leftBorder.rect ()
               << " refCol=" << (leftBorder.exists () ? (int *) leftBorder.referenceColor ().toQRgb () : nullptr)
               << " avgCol=" << (leftBorder.exists () ? (int *) leftBorder.averageColor ().toQRgb () : nullptr);
    qCDebug(kpLogImagelib) << "\t\tright=" << rightBorder.rect ()
               << " refCol=" << (rightBorder.exists () ? (int *) rightBorder.referenceColor ().toQRgb () : nullptr)
               << " avgCol=" << (rightBorder.exists () ? (int *) rightBorder.averageColor ().toQRgb () : nullptr);
    qCDebug(kpLogImagelib) << "\t\ttop=" << topBorder.rect ()
               << " refCol=" << (topBorder.exists () ? (int *) topBorder.referenceColor ().toQRgb () : nullptr)
               << " avgCol=" << (topBorder.exists () ? (int *) topBorder.averageColor ().toQRgb () : nullptr);
    qCDebug(kpLogImagelib) << "\t\tbot=" << botBorder.rect ()
               << " refCol=" << (botBorder.exists () ? (int *) botBorder.referenceColor ().toQRgb () : nullptr)
               << " avgCol=" << (botBorder.exists () ? (int *) botBorder.averageColor ().toQRgb () : nullptr);
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
            qCDebug(kpLogImagelib) << "\tignoring left border";
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
            qCDebug(kpLogImagelib) << "\tignoring top border";
        #endif
            topBorder.invalidate ();
        }
    }


    mainWindow->addImageOrSelectionCommand (
        new kpTransformAutoCropCommand (static_cast<bool> (doc->selection ()),
            leftBorder, rightBorder, topBorder, botBorder,  mainWindow->commandEnvironment ()));


    return true;
}
