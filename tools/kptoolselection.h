
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


#ifndef __kp_tool_selection_h__
#define __kp_tool_selection_h__


#include <qpixmap.h>
#include <qpoint.h>
#include <qpointarray.h>
#include <qrect.h>

#include <kcommand.h>

#include <kpcolor.h>
#include <kpselectiontransparency.h>
#include <kptool.h>

class QPoint;
class QRect;
class kpMainWindow;
class kpSelection;
class kpToolWidgetOpaqueOrTransparent;

class kpToolSelection : public kpTool
{
Q_OBJECT

public:
    kpToolSelection (kpMainWindow *mainWindow);
    virtual ~kpToolSelection ();

    enum Mode {Rectangle, Ellipse, FreeForm, Text};
    void setMode (Mode mode) { m_mode = mode; }

private:
    void pushOntoDocument ();

public:
    QString haventBegunDrawUserMessage () const;

    virtual void begin ();
    virtual void end ();
    virtual void reselect ();

    virtual bool careAboutModifierState () const { return true; }

    virtual void beginDraw ();
    virtual void hover (const QPoint &point);
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
        Unknown, Create, Move, SelectText
    };
    DragType m_dragType;
    bool m_dragHasBegun;

    class kpToolSelectionPullFromDocumentCommand *m_currentPullFromDocumentCommand;
    class kpToolSelectionMoveCommand *m_currentMoveCommand;
    bool m_currentMoveCommandIsSmear;
    kpToolWidgetOpaqueOrTransparent *m_toolWidgetOpaqueOrTransparent;

    class kpToolSelectionCreateCommand *m_currentCreateTextCommand;
    bool m_cancelledShapeButStillHoldingButtons;
};

class kpToolSelectionCreateCommand : public KCommand
{
public:
    // (if fromSelection doesn't have a pixmap, it will only recreate the region)
    kpToolSelectionCreateCommand (const QString &name, const kpSelection &fromSelection,
                                  kpMainWindow *mainWindow);
    virtual QString name () const;
    virtual ~kpToolSelectionCreateCommand ();

private:
    kpDocument *document () const;

public:
    void setFromSelection (const kpSelection &fromSelection);

    virtual void execute ();
    virtual void unexecute ();

private:
    QString m_name;
    kpSelection *m_fromSelection;
    kpMainWindow *m_mainWindow;

    int m_textRow, m_textCol;
};

class kpToolSelectionPullFromDocumentCommand : public KCommand
{
public:
    kpToolSelectionPullFromDocumentCommand (const QString &name, kpMainWindow *mainWindow);
    virtual QString name () const;
    virtual ~kpToolSelectionPullFromDocumentCommand ();

private:
    kpDocument *document () const;

public:
    virtual void execute ();
    virtual void unexecute ();

private:
    QString m_name;
    kpMainWindow *m_mainWindow;
    kpColor m_backgroundColor;
    kpSelection *m_originalSelectionRegion;
};

class kpToolSelectionTransparencyCommand : public KCommand
{
public:
    kpToolSelectionTransparencyCommand (const QString &name,
        const kpSelectionTransparency &st,
        const kpSelectionTransparency &oldST,
        kpMainWindow *mainWindow);
    virtual QString name () const;
    virtual ~kpToolSelectionTransparencyCommand ();

private:
    kpDocument *document () const;

public:
    virtual void execute ();
    virtual void unexecute ();

private:
    QString m_name;
    kpSelectionTransparency m_st, m_oldST;
    kpMainWindow *m_mainWindow;
};

class kpToolSelectionMoveCommand : public KCommand
{
public:
    kpToolSelectionMoveCommand (const QString &name, kpMainWindow *mainWindow);
    virtual QString name () const;
    virtual ~kpToolSelectionMoveCommand ();

private:
    kpDocument *document () const;

public:
    kpSelection originalSelection () const;

    virtual void execute ();
    virtual void unexecute ();

    void moveTo (const QPoint &point, bool moveLater = false);
    void moveTo (int x, int y, bool moveLater = false);
    void copyOntoDocument ();
    void finalize ();

private:
    QString m_name;
    kpMainWindow *m_mainWindow;

    QPoint m_startPoint, m_endPoint;

    QPixmap m_oldDocumentPixmap;

    // area of document affected (not the bounding rect of the sel)
    QRect m_documentBoundingRect;

    QPointArray m_copyOntoDocumentPoints;
};

class kpToolSelectionDestroyCommand : public KCommand
{
public:
    kpToolSelectionDestroyCommand (const QString &name, bool pushOntoDocument,
                                   kpMainWindow *mainWindow);
    virtual QString name () const;
    virtual ~kpToolSelectionDestroyCommand ();

private:
    kpDocument *document () const;

public:
    virtual void execute ();
    virtual void unexecute ();

private:
    QString m_name;
    bool m_pushOntoDocument;
    QPixmap m_oldDocPixmap;
    kpSelection *m_oldSelection;
    kpMainWindow *m_mainWindow;

    int m_textRow, m_textCol;
};

#endif  // __kp_tool_selection_h__
