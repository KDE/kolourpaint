
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


#define DEBUG_KP_VIEW 0
#define DEBUG_KP_VIEW_RENDERER ((DEBUG_KP_VIEW && 1) || 0)


#include <kpView.h>
#include <kpViewPrivate.h>

#include <kpAbstractSelection.h>
#include <kpTextSelection.h>
#include <kpTool.h>


// public
QRect kpView::selectionViewRect () const
{
    return selection () ?
               transformDocToView (selection ()->boundingRect ()) :
               QRect ();

}


// public
QPoint kpView::mouseViewPointRelativeToSelection (const QPoint &viewPoint) const
{
    if (!selection ())
        return KP_INVALID_POINT;

    return mouseViewPoint (viewPoint) - transformDocToView (selection ()->topLeft ());
}

// public
bool kpView::mouseOnSelection (const QPoint &viewPoint) const
{
    const QRect selViewRect = selectionViewRect ();
    if (!selViewRect.isValid ())
        return false;

    return selViewRect.contains (mouseViewPoint (viewPoint));
}


// public
int kpView::textSelectionMoveBorderAtomicSize () const
{
    if (!textSelection ())
        return 0;

    return qMax (4, zoomLevelX () / 100);
}

// public
bool kpView::mouseOnSelectionToMove (const QPoint &viewPoint) const
{
    if (!mouseOnSelection (viewPoint))
        return false;

    if (!textSelection ())
        return true;

    if (mouseOnSelectionResizeHandle (viewPoint))
        return false;


    const QPoint viewPointRelSel = mouseViewPointRelativeToSelection (viewPoint);

    // Middle point should always be selectable
    const QPoint selCenterDocPoint = selection ()->boundingRect ().center ();
    if (tool () &&
        tool ()->calculateCurrentPoint () == selCenterDocPoint)
    {
        return false;
    }


    const int atomicSize = textSelectionMoveBorderAtomicSize ();
    const QRect selViewRect = selectionViewRect ();

    return (viewPointRelSel.x () < atomicSize ||
            viewPointRelSel.x () >= selViewRect.width () - atomicSize ||
            viewPointRelSel.y () < atomicSize ||
            viewPointRelSel.y () >= selViewRect.height () - atomicSize);
}


// protected
bool kpView::selectionLargeEnoughToHaveResizeHandlesIfAtomicSize (int atomicSize) const
{
    if (!selection ())
        return false;

    const QRect selViewRect = selectionViewRect ();

    return (selViewRect.width () >= atomicSize * 5 ||
            selViewRect.height () >= atomicSize * 5);
}

// public
int kpView::selectionResizeHandleAtomicSize () const
{
    int atomicSize = qMin (7, qMax (4, zoomLevelX () / 100));
    while (atomicSize > 0 &&
           !selectionLargeEnoughToHaveResizeHandlesIfAtomicSize (atomicSize))
    {
        atomicSize--;
    }

    return atomicSize;
}

// public
bool kpView::selectionLargeEnoughToHaveResizeHandles () const
{
    return (selectionResizeHandleAtomicSize () > 0);
}

// public
QRegion kpView::selectionResizeHandlesViewRegion (bool forRenderer) const
{
    QRegion ret;

    const int atomicLength = selectionResizeHandleAtomicSize ();
    if (atomicLength <= 0)
        return QRegion ();


    // HACK: At low zoom (e.g. 100%), resize handles will probably be too
    //       big and overlap text / cursor / too much of selection.
    //
    //       So limit the _visual_ size of handles at low zoom.  The
    //       handles' grab area remains the same for usability; so yes,
    //       there are a few pixels that don't look grabable but they are.
    //
    //       The real solution is to be able to partially render the
    //       handles outside of the selection view rect.  If not possible,
    //       at least for text boxes, render text on top of handles.
    int normalAtomicLength = atomicLength;
    int vertEdgeAtomicLength = atomicLength;
    if (forRenderer && selection ())
    {
        if (zoomLevelX () <= 150)
        {
            if (normalAtomicLength > 1)
                normalAtomicLength--;

            if (vertEdgeAtomicLength > 1)
                vertEdgeAtomicLength--;
        }

        // 1 line of text?
        if (textSelection () && textSelection ()->textLines ().size () == 1)
        {
            if (zoomLevelX () <= 150)
                vertEdgeAtomicLength = qMin (vertEdgeAtomicLength, qMax (2, zoomLevelX () / 100));
            else if (zoomLevelX () <= 250)
                vertEdgeAtomicLength = qMin (vertEdgeAtomicLength, qMax (3, zoomLevelX () / 100));
        }
    }


    const QRect selViewRect = selectionViewRect ();

#define ADD_BOX_RELATIVE_TO_SELECTION(type,x,y)    \
    ret += QRect ((x), (y), type##AtomicLength, type##AtomicLength)

    ADD_BOX_RELATIVE_TO_SELECTION (normal,
                                   selViewRect.width () - normalAtomicLength,
                                   selViewRect.height () - normalAtomicLength);
    ADD_BOX_RELATIVE_TO_SELECTION (normal,
                                   selViewRect.width () - normalAtomicLength,
                                   0);
    ADD_BOX_RELATIVE_TO_SELECTION (normal,
                                   0,
                                   selViewRect.height () - normalAtomicLength);
    ADD_BOX_RELATIVE_TO_SELECTION (normal,
                                   0,
                                   0);

    ADD_BOX_RELATIVE_TO_SELECTION (vertEdge,
                                   selViewRect.width () - vertEdgeAtomicLength,
                                   (selViewRect.height () - vertEdgeAtomicLength) / 2);
    ADD_BOX_RELATIVE_TO_SELECTION (normal,
                                   (selViewRect.width () - normalAtomicLength) / 2,
                                   selViewRect.height () - normalAtomicLength);
    ADD_BOX_RELATIVE_TO_SELECTION (normal,
                                   (selViewRect.width () - normalAtomicLength) / 2,
                                   0);
    ADD_BOX_RELATIVE_TO_SELECTION (vertEdge,
                                   0,
                                   (selViewRect.height () - vertEdgeAtomicLength) / 2);

#undef ADD_BOX_RELATIVE_TO_SELECTION

    ret.translate (selViewRect.x (), selViewRect.y ());
    ret = ret.intersect (selViewRect);

    return ret;
}

// public
// REFACTOR: use QFlags as the return type for better type safety.
int kpView::mouseOnSelectionResizeHandle (const QPoint &viewPoint) const
{
#if DEBUG_KP_VIEW
    kDebug () << "kpView::mouseOnSelectionResizeHandle(viewPoint="
               << viewPoint << ")" << endl;
#endif

    if (!mouseOnSelection (viewPoint))
    {
    #if DEBUG_KP_VIEW
        kDebug () << "\tmouse not on sel";
    #endif
        return 0;
    }


    const QRect selViewRect = selectionViewRect ();
#if DEBUG_KP_VIEW
    kDebug () << "\tselViewRect=" << selViewRect;
#endif


    const int atomicLength = selectionResizeHandleAtomicSize ();
#if DEBUG_KP_VIEW
    kDebug () << "\tatomicLength=" << atomicLength;
#endif

    if (atomicLength <= 0)
    {
    #if DEBUG_KP_VIEW
        kDebug () << "\tsel not large enough to have resize handles";
    #endif
        // Want to make it possible to move a small selection
        return 0;
    }


    const QPoint viewPointRelSel = mouseViewPointRelativeToSelection (viewPoint);
#if DEBUG_KP_VIEW
    kDebug () << "\tviewPointRelSel=" << viewPointRelSel;
#endif


#define LOCAL_POINT_IN_BOX_AT(x,y)  \
    QRect ((x), (y), atomicLength, atomicLength).contains (viewPointRelSel)

    // Favour the bottom & right and the corners.
    if (LOCAL_POINT_IN_BOX_AT (selViewRect.width () - atomicLength,
                               selViewRect.height () - atomicLength))
    {
        return kpView::Bottom | kpView::Right;
    }
    else if (LOCAL_POINT_IN_BOX_AT (selViewRect.width () - atomicLength, 0))
    {
        return kpView::Top | kpView::Right;
    }
    else if (LOCAL_POINT_IN_BOX_AT (0, selViewRect.height () - atomicLength))
    {
        return kpView::Bottom | kpView::Left;
    }
    else if (LOCAL_POINT_IN_BOX_AT (0, 0))
    {
        return kpView::Top | kpView::Left;
    }
    else if (LOCAL_POINT_IN_BOX_AT (selViewRect.width () - atomicLength,
                                    (selViewRect.height () - atomicLength) / 2))
    {
        return kpView::Right;
    }
    else if (LOCAL_POINT_IN_BOX_AT ((selViewRect.width () - atomicLength) / 2,
                                    selViewRect.height () - atomicLength))
    {
        return kpView::Bottom;
    }
    else if (LOCAL_POINT_IN_BOX_AT ((selViewRect.width () - atomicLength) / 2, 0))
    {
        return kpView::Top;
    }
    else if (LOCAL_POINT_IN_BOX_AT (0, (selViewRect.height () - atomicLength) / 2))
    {
        return kpView::Left;
    }
    else
    {
    #if DEBUG_KP_VIEW
        kDebug () << "\tnot on sel resize handle";
    #endif
        return 0;
    }
#undef LOCAL_POINT_IN_BOX_AT
}

// public
bool kpView::mouseOnSelectionToSelectText (const QPoint &viewPoint) const
{
#if DEBUG_KP_VIEW
    kDebug () << "kpView::mouseOnSelectionToSelectText(viewPoint="
               << viewPoint << ")" << endl;
#endif

    if (!mouseOnSelection (viewPoint))
    {
    #if DEBUG_KP_VIEW
        kDebug () << "\tmouse non on sel";
    #endif
        return false;
    }

    if (!textSelection ())
    {
    #if DEBUG_KP_VIEW
        kDebug () << "\tsel not text";
    #endif
        return false;
    }

#if DEBUG_KP_VIEW
    kDebug () << "\tmouse on sel: to move=" << mouseOnSelectionToMove ()
               << " to resize=" << mouseOnSelectionResizeHandle ()
               << endl;
#endif

    return (!mouseOnSelectionToMove (viewPoint) &&
            !mouseOnSelectionResizeHandle (viewPoint));
}
