
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


#define DEBUG_KP_UNZOOMED_THUMBNAIL_VIEW 0


#include <kpunzoomedthumbnailview.h>

#include <kdebug.h>
#include <klocale.h>

#include <kpdocument.h>
#include <kpviewmanager.h>
#include <kpviewscrollablecontainer.h>


struct kpUnzoomedThumbnailViewPrivate
{
};


kpUnzoomedThumbnailView::kpUnzoomedThumbnailView (
        kpDocument *document,
        kpToolToolBar *toolToolBar,
        kpViewManager *viewManager,
        kpView *buddyView,
        kpViewScrollableContainer *scrollableContainer,
        QWidget *parent, const char *name)

    : kpThumbnailView (document, toolToolBar, viewManager,
                       buddyView,
                       scrollableContainer,
                       parent, name),
      d (new kpUnzoomedThumbnailViewPrivate ())
{
    if (buddyViewScrollableContainer ())
    {
        connect (buddyViewScrollableContainer (),
                SIGNAL (contentsMovingSoon (int, int)),
                this,
                SLOT (adjustToEnvironment ()));
    }

    // Call to virtual function - this is why the class is sealed
    adjustToEnvironment ();
}


kpUnzoomedThumbnailView::~kpUnzoomedThumbnailView ()
{
    delete d;
}


// public virtual [base kpThumbnailView]
QString kpUnzoomedThumbnailView::caption () const
{
    return i18n ("Unzoomed Mode - Thumbnail");
}


// public slot virtual [base kpView]
void kpUnzoomedThumbnailView::adjustToEnvironment ()
{
    if (!buddyView () || !buddyViewScrollableContainer () || !document ())
        return;

    const int scrollViewContentsX =
        buddyViewScrollableContainer ()->contentsXSoon ();
    const int scrollViewContentsY =
        buddyViewScrollableContainer ()->contentsYSoon ();

#if DEBUG_KP_UNZOOMED_THUMBNAIL_VIEW
    kdDebug () << "kpUnzoomedThumbnailView(" << name ()
               << ")::adjustToEnvironment("
               << scrollViewContentsX
               << ","
               << scrollViewContentsY
               << ") width=" << width ()
               << " height=" << height ()
               << endl;
#endif


#if 1
    int x;
    if (document ()->width () > width ())
    {
        x = (int) buddyView ()->transformViewToDocX (scrollViewContentsX);
        const int rightMostAllowedX = QMAX (0, document ()->width () - width ());
    #if DEBUG_KP_UNZOOMED_THUMBNAIL_VIEW
        kdDebug () << "\tdocX=" << x
                << " docWidth=" << document ()->width ()
                << " rightMostAllowedX=" << rightMostAllowedX
                << endl;
    #endif
        if (x > rightMostAllowedX)
            x = rightMostAllowedX;
    }
    // Thumbnail width <= doc width
    else
    {
        // Centre X (rather than flush left to be consistent with
        //           kpZoomedThumbnailView)
        x = -(width () - document ()->width ()) / 2;
    }


    int y;
    if (document ()->height () > height ())
    {
        y = (int) buddyView ()->transformViewToDocY (scrollViewContentsY);
        const int bottomMostAllowedY = QMAX (0, document ()->height () - height ());
    #if DEBUG_KP_UNZOOMED_THUMBNAIL_VIEW
        kdDebug () << "\tdocY=" << y
                    << " docHeight=" << document ()->height ()
                    << " bottomMostAllowedY=" << bottomMostAllowedY
                    << endl;
    #endif
        if (y > bottomMostAllowedY)
            y = bottomMostAllowedY;
    }
    // Thumbnail height <= doc height
    else
    {
        // Centre Y (rather than flush top to be consistent with
        //           kpZoomedThumbnailView)
        y = -(height () - document ()->height ()) / 2;
    }
// Prefer to keep visible area centred in thumbnail instead of flushed left.
// Gives more editing context to the left and top.
// But feels awkward for left-to-right users.  So disabled for now.
// Not totally tested.
#else
    if (!buddyViewScrollableContainer ())
        return;

    QRect docRect = buddyView ()->transformViewToDoc (
        QRect (buddyViewScrollableContainer ()->contentsXSoon (),
               buddyViewScrollableContainer ()->contentsYSoon (),
               QMIN (buddyView ()->width (), buddyViewScrollableContainer ()->visibleWidth ()),
               QMIN (buddyView ()->height (), buddyViewScrollableContainer ()->visibleHeight ())));

    x = docRect.x () - (width () - docRect.width ()) / 2;
    kdDebug () << "\tnew suggest x=" << x << endl;
    const int rightMostAllowedX = QMAX (0, document ()->width () - width ());
    if (x < 0)
        x = 0;
    if (x > rightMostAllowedX)
        x = rightMostAllowedX;

    y = docRect.y () - (height () - docRect.height ()) / 2;
    kdDebug () << "\tnew suggest y=" << y << endl;
    const int bottomMostAllowedY = QMAX (0, document ()->height () - height ());
    if (y < 0)
        y = 0;
    if (y > bottomMostAllowedY)
        y = bottomMostAllowedY;
#endif


    if (viewManager ())
    {
        viewManager ()->setFastUpdates ();
        viewManager ()->setQueueUpdates ();
    }

    {
        // OPT: scrollView impl would be much, much faster
        setOrigin (QPoint (-x, -y));
        setMaskToCoverDocument ();

        // Above might be a NOP even if e.g. doc size changed so force
        // update
        if (viewManager ())
            viewManager ()->updateView (this);
    }

    if (viewManager ())
    {
        viewManager ()->restoreQueueUpdates ();
        viewManager ()->restoreFastUpdates ();
    }
}


#include <kpunzoomedthumbnailview.moc>
