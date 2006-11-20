
/*
   Copyright (c) 2003-2006 Clarence Dang <dang@kde.org>
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


#ifndef KP_TOOL_SELECTION_H
#define KP_TOOL_SELECTION_H


#include <QPoint>

#include <kptool.h>


class QCursor;
class QKeyEvent;
class QRect;
class QTimer;

class kpColor;
class kpMainWindow;
class kpToolSelectionCreateCommand;
class kpToolSelectionMoveCommand;
class kpToolSelectionPullFromDocumentCommand;
class kpToolSelectionResizeScaleCommand;
class kpToolWidgetOpaqueOrTransparent;


class kpToolSelection : public kpTool
{
Q_OBJECT

public:
    // TODO: use inheritance
    enum Mode {Rectangle, Ellipse, FreeForm, Text};

    kpToolSelection (Mode mode,
                     const QString &text, const QString &description,
                     int key,
                     kpMainWindow *mainWindow, const QString &name);
    virtual ~kpToolSelection ();

    void setMode (Mode mode) { m_mode = mode; }

private:
    void pushOntoDocument ();

protected:
    bool onSelectionToMove () const;
    int onSelectionResizeHandle () const;
    bool onSelectionToSelectText () const;

protected:
    // Appropriate one called by haventBegunDrawUserMessage().
    // Subclasses may wish to reimplement.
    virtual QString haventBegunDrawUserMessageOnResizeHandle () const;
    virtual QString haventBegunDrawUserMessageInsideSelection () const;
    virtual QString haventBegunDrawUserMessageOutsideSelection () const;

public:
    QString haventBegunDrawUserMessage () const;

    virtual void begin ();
    virtual void end ();
    virtual void reselect ();

    virtual bool careAboutModifierState () const { return true; }
    bool controlOrShiftPressed () const { return (m_controlPressed || m_shiftPressed); }

protected:
    enum DragType
    {
        Unknown, Create, Move, SelectText, ResizeScale
    };

    // Called by beginDraw() if dragging inside selection.
    // Returns the type of drag that is occurring.  Will result in side-effects.
    //
    // Overridden in kpToolText.
    // TODO: Not completely clean interface with subclasses due to side effects.
    virtual DragType beginDrawInsideSelection ();

public:
    virtual void beginDraw ();
protected:
    // Returns the mouse cursor when the mouse is on top of a selection.
    // By default, this is a sizing cursor.  Subclasses may wish to reimplement.
    virtual QCursor cursorInsideSelection () const;

    QCursor cursor () const;
public:
    virtual void hover (const QPoint &point);
protected:
    void popupRMBMenu ();
    void setSelectionBorderForMove ();
protected slots:
    void slotRMBMoveUpdateGUI ();
    void delayedDraw ();
private:
    void create (QPoint thisPoint, QRect normalizedRect);
    void move (QPoint thisPoint, QRect normalizedRect);
    void resizeScale (QPoint thisPoint, QRect normalizedRect);
public:
    virtual void draw (const QPoint &thisPoint, const QPoint &lastPoint,
                       const QRect &normalizedRect);
protected:
    // Sets the selection border mode when no dragging is occurring.
    // Subclasses may wish to reimplement but should still call the base
    // implementation.
    virtual void setSelectionBorderForHaventBegunDraw ();

private:
    void cancelMove ();
    void cancelCreate ();
    void cancelResizeScale ();
public:
    virtual void cancelShape ();
    virtual void releasedAllButtons ();
    virtual void endDraw (const QPoint &thisPoint, const QRect &normalizedRect);

protected:
    virtual void keyPressEvent (QKeyEvent *e);

protected:
    void selectionTransparencyChanged (const QString &name);

protected slots:
    virtual void slotIsOpaqueChanged ();
    virtual void slotBackgroundColorChanged (const kpColor &color);
    virtual void slotColorSimilarityChanged (double similarity, int);

protected:
    Mode m_mode;

    QPoint m_startDragFromSelectionTopLeft;
    DragType m_dragType;
    bool m_dragHasBegun;
    bool m_hadSelectionBeforeDrag;
    int m_resizeScaleType;

    kpToolSelectionPullFromDocumentCommand *m_currentPullFromDocumentCommand;
    kpToolSelectionMoveCommand *m_currentMoveCommand;
    bool m_currentMoveCommandIsSmear;
    kpToolSelectionResizeScaleCommand *m_currentResizeScaleCommand;
    kpToolWidgetOpaqueOrTransparent *m_toolWidgetOpaqueOrTransparent;

    kpToolSelectionCreateCommand *m_currentCreateTextCommand;
    bool m_cancelledShapeButStillHoldingButtons;

    QTimer *m_createNOPTimer, *m_RMBMoveUpdateGUITimer;
};


#endif  // KP_TOOL_SELECTION_H
