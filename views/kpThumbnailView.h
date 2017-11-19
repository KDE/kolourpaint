
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


#ifndef KP_THUMBNAIL_VIEW_H
#define KP_THUMBNAIL_VIEW_H


#include "views/kpView.h"


/**
 * @short Abstract base class for all thumbnail views.
 *
 * @author Clarence Dang <dang@kde.org>
 */
class kpThumbnailView : public kpView
{
Q_OBJECT

public:
    /**
     * Constructs a thumbnail view.
     *
     * You must call adjustEnvironment() at the end of your constructor.
     */
    kpThumbnailView (kpDocument *document,
                     kpToolToolBar *toolToolBar,
                     kpViewManager *viewManager,
                     kpView *buddyView,
                     kpViewScrollableContainer *scrollableContainer,
                     QWidget *parent);

    /**
     * Destructs this thumbnail view.
     */
    ~kpThumbnailView () override;


    /**
     * @returns the caption to display in an enclosing thumbnail window.
     */
    virtual QString caption () const = 0;


protected:
    /**
     * Sets the mask to cover the rectangle with top-left, origin() and
     * dimensions equal to or slightly less than (in case of rounding
     * error) the size of the document in view coordinates.  This ensures
     * that all pixels are initialised with either document pixels or the
     * standard widget background.
     */
    void setMaskToCoverDocument ();


    /**
     * Calls adjustToEnvironment() in response to a resize event.
     *
     * Extends @ref kpView.
     */
    void resizeEvent (QResizeEvent *e) override;
};


#endif  // KP_THUMBNAIL_VIEW_H
