
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


#ifndef __kp_tool_selection_h__
#define __kp_tool_selection_h__


#include <qpixmap.h>
#include <qpoint.h>
#include <qpointarray.h>
#include <qrect.h>

#include <kpcolor.h>
#include <kpcommandhistory.h>
#include <kpselection.h>
#include <kpselectiontransparency.h>
#include <kptool.h>


class QPoint;
class QRect;
class QTimer;

class kpMainWindow;
class kpSelection;

class kpToolSelectionCreateCommand;
class kpToolSelectionMoveCommand;
class kpToolSelectionPullFromDocumentCommand;
class kpToolSelectionResizeScaleCommand;
class kpToolWidgetOpaqueOrTransparent;


class kpToolSelection : public kpTool
{
Q_OBJECT

public:
    enum Mode {Rectangle, Ellipse, FreeForm, Text};

    kpToolSelection (Mode mode,
                     const QString &text, const QString &description,
                     int key,
                     kpMainWindow *mainWindow, const char *name);
    virtual ~kpToolSelection ();

    void setMode (Mode mode) { m_mode = mode; }

private:
    void pushOntoDocument ();

protected:
    bool onSelectionToMove () const;
    int onSelectionResizeHandle () const;
    bool onSelectionToSelectText () const;

public:
    QString haventBegunDrawUserMessage () const;

    virtual void begin ();
    virtual void end ();
    virtual void reselect ();

    virtual bool careAboutModifierState () const { return true; }
    bool controlOrShiftPressed () const { return (m_controlPressed || m_shiftPressed); }

    virtual void beginDraw ();
protected:
    const QCursor &cursor () const;
public:
    virtual void hover (const QPoint &point);
protected:
    void popupRMBMenu ();
    void setSelectionBorderForMove ();
protected slots:
    void slotRMBMoveUpdateGUI ();
    void delayedDraw ();
public:
    virtual void draw (const QPoint &thisPoint, const QPoint &lastPoint,
                       const QRect &normalizedRect);
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
    enum DragType
    {
        Unknown, Create, Move, SelectText, ResizeScale
    };
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

class kpToolSelectionCreateCommand : public kpNamedCommand
{
public:
    // (if fromSelection doesn't have a pixmap, it will only recreate the region)
    kpToolSelectionCreateCommand (const QString &name, const kpSelection &fromSelection,
                                  kpMainWindow *mainWindow);
    virtual ~kpToolSelectionCreateCommand ();

    virtual int size () const;

    static bool nextUndoCommandIsCreateBorder (kpCommandHistory *commandHistory);

    const kpSelection *fromSelection () const;
    void setFromSelection (const kpSelection &fromSelection);

    virtual void execute ();
    virtual void unexecute ();

private:
    kpSelection *m_fromSelection;

    int m_textRow, m_textCol;
};

class kpToolSelectionPullFromDocumentCommand : public kpNamedCommand
{
public:
    kpToolSelectionPullFromDocumentCommand (const QString &name, kpMainWindow *mainWindow);
    virtual ~kpToolSelectionPullFromDocumentCommand ();

    virtual int size () const;

    virtual void execute ();
    virtual void unexecute ();

private:
    kpColor m_backgroundColor;
    kpSelection *m_originalSelectionRegion;
};

class kpToolSelectionTransparencyCommand : public kpNamedCommand
{
public:
    kpToolSelectionTransparencyCommand (const QString &name,
        const kpSelectionTransparency &st,
        const kpSelectionTransparency &oldST,
        kpMainWindow *mainWindow);
    virtual ~kpToolSelectionTransparencyCommand ();

    virtual int size () const;

    virtual void execute ();
    virtual void unexecute ();

private:
    kpSelectionTransparency m_st, m_oldST;
};

class kpToolSelectionMoveCommand : public kpNamedCommand
{
public:
    kpToolSelectionMoveCommand (const QString &name, kpMainWindow *mainWindow);
    virtual ~kpToolSelectionMoveCommand ();

    kpSelection originalSelection () const;

    virtual int size () const;

    virtual void execute ();
    virtual void unexecute ();

    void moveTo (const QPoint &point, bool moveLater = false);
    void moveTo (int x, int y, bool moveLater = false);
    void copyOntoDocument ();
    void finalize ();

private:
    QPoint m_startPoint, m_endPoint;

    QPixmap m_oldDocumentPixmap;

    // area of document affected (not the bounding rect of the sel)
    QRect m_documentBoundingRect;

    QPointArray m_copyOntoDocumentPoints;
};

// You could subclass kpToolResizeScaleCommand and/or
// kpToolSelectionMoveCommand instead if want a disaster.
// This is different to kpToolResizeScaleCommand in that:
//
// 1. This only works for selections.
// 2. This is designed for the size and position to change several times
//    before execute().
//
class kpToolSelectionResizeScaleCommand : public QObject,
                                          public kpNamedCommand
{
Q_OBJECT

public:
    kpToolSelectionResizeScaleCommand (kpMainWindow *mainWindow);
    virtual ~kpToolSelectionResizeScaleCommand ();

    virtual int size () const;

public:
    kpSelection originalSelection () const;

    QPoint topLeft () const;
    void moveTo (const QPoint &point);

    int width () const;
    int height () const;
    void resize (int width, int height, bool delayed = false);

    // (equivalent to resize() followed by moveTo() but faster)
    void resizeAndMoveTo (int width, int height, const QPoint &point,
                          bool delayed = false);

protected:
    void killSmoothScaleTimer ();

    // If <delayed>, does a fast, low-quality scale and then calls itself
    // with <delayed> unset for a smooth scale, a short time later.
    // If acting on a text box, <delayed> is ignored.
    void resizeScaleAndMove (bool delayed);

protected slots:
    void resizeScaleAndMove (/*delayed = false*/);

public:
    void finalize ();

public:
    virtual void execute ();
    virtual void unexecute ();

protected:
    kpSelection m_originalSelection;

    QPoint m_newTopLeft;
    int m_newWidth, m_newHeight;

    QTimer *m_smoothScaleTimer;
};

class kpToolSelectionDestroyCommand : public kpNamedCommand
{
public:
    kpToolSelectionDestroyCommand (const QString &name, bool pushOntoDocument,
                                   kpMainWindow *mainWindow);
    virtual ~kpToolSelectionDestroyCommand ();

    virtual int size () const;

    virtual void execute ();
    virtual void unexecute ();

private:
    bool m_pushOntoDocument;
    QPixmap m_oldDocPixmap;
    kpSelection *m_oldSelection;

    int m_textRow, m_textCol;
};

#endif  // __kp_tool_selection_h__
