
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


#ifndef __kptool_h__
#define __kptool_h__

#include <qobject.h>
#include <qpoint.h>
#include <qrect.h>
#include <qstring.h>


class QPixmap;

class kpColor;
class kpColorToolBar;
class kpCommandHistory;
class kpDocument;
class kpView;
class kpViewManager;
class kpMainWindow;
class kpToolToolBar;


// Base class for all tools
class kpTool : public QObject
{
Q_OBJECT

public:
    kpTool (const QString &text, const QString &description, kpMainWindow *, const char *name);
    virtual ~kpTool ();

    void setText (const QString &text) { m_text = text; }
    QString text () const { return m_text; }

    void setDescription (const QString &description) { m_description = description; }
    QString description () const { return m_description; }

    void setName (const char *name) { m_name = name; }
    const char *name () const { return m_name; }

    static QRect neededRect (const QRect &rect, int lineWidth);
    static QPixmap neededPixmap (const QPixmap &pixmap, const QRect &boundingRect);

    bool hasCurrentPoint () const;
    // Returns the position of the cursor relative to the topleft point of
    // the current view (viewUnderStartPoint() or viewUnderCursor() otherwise).
    //
    // If neither viewUnderStartPoint() nor viewUnderCursor()
    // (i.e. !hasCurrentPoint()), then it returns QPoint (INT_MIN, INT_MIN).
    //
    // If <zoomToDoc> is set (default), then it returns the position in the
    // document.  This theoretically == m_currentPoint (when m_currentPoint
    // is defined) but I wouldn't bet on it.  This function is useful when
    // m_currentPoint isn't necessarily defined (outside of beginDraw(),draw()
    // and hover()).
    //
    // If <zoomToDoc> is not set, then it returns an unzoomed view coordinate.
    //
    // Keep in mind that if viewUnderStartPoint(), this can return coordinates
    // outside of the document/view.
    QPoint currentPoint (bool zoomToDoc = true) const;

public slots:
    // Call this when something below the mouse cursor may have changed
    // and/or if the view has moved relative to the cursor (as opposed to
    // the cursor moving relative to the view, which would trigger a
    // mouseMoveEvent and all would be well without such hacks)
    // e.g. when zooming or scrolling the view or when deleting a selection.
    //
    // This calls hover() or draw() to let the tool know.  The Brush Tool
    // can then update the position of the Brush Cursor.  The Selection
    // Tool can update the real cursor.  The Line Tool can update the current
    // line.  The statubar gets correct coordinates.  etc. etc.
    void somethingBelowTheCursorChanged ();

    // Same as above except that you claim you know better than currentPoint()
    void somethingBelowTheCursorChanged (const QPoint &currentPoint_);

public:
    // called when the tool is selected
    virtual void begin ();

    // called when the tool is deselected
    virtual void end ();

    // set after begin() has been called, unset after end() has been called
    bool hasBegun () const { return m_began; }

    bool hasBegunDraw () const { return m_beganDraw; }

    virtual bool hasBegunShape () const { return hasBegunDraw (); }

    // called when user double-left-clicks on Tool Icon (not view)
    virtual void globalDraw ();

signals:
    // emitted after beginDraw() has been called
    void beganDraw (const QPoint &point);

    // emitted after endDraw() has been called
    void endedDraw (const QPoint &point);

    // emit these signals as appropriate
    // (if you emit mouseDragged, don't emit mouseMoved)
    void mouseMoved (const QPoint &point);
    void mouseDragged (const QRect &docRect);  // can have negative width/height

protected:
    virtual bool returnToPreviousToolAfterEndDraw () const { return false; }
    virtual bool careAboutModifierState () const { return false; }

    virtual void beginDraw ();

    // mouse move without button pressed
    // (only m_currentPoint is defined)
    virtual void hover (const QPoint &point);

    // this is useful for "instant" tools like the Pen & Eraser
    virtual void draw (const QPoint &thisPoint, const QPoint &lastPoint,
                        const QRect &normalizedRect);

    // (m_mouseButton will not change from beginDraw())
    virtual void cancelShape ();

    virtual void endDraw (const QPoint &thisPoint, const QRect &normalizedRect);

    virtual void endShape (const QPoint &thisPoint = QPoint (),
                           const QRect &normalizedRect = QRect ())
    {
        endDraw (thisPoint, normalizedRect);
    }

    kpMainWindow *mainWindow () const;
    kpDocument *document () const;
    kpViewManager *viewManager () const;
    kpToolToolBar *toolToolBar () const;
    kpView *viewUnderStartPoint () const { return m_viewUnderStartPoint; }
    kpView *viewUnderCursor () const;
    kpCommandHistory *commandHistory () const;

    kpColor color (int which) const;

    kpColor foregroundColor () const;
    kpColor backgroundColor () const;

    double colorSimilarity () const;
    int processedColorSimilarity () const;

protected slots:
    virtual void slotForegroundColorChanged (const kpColor & /*color*/) {}
    virtual void slotBackgroundColorChanged (const kpColor & /*color*/) {}
    virtual void slotColorSimilarityChanged (double /*similarity*/, int /*processedSimilarity*/) {};

protected:
    // returns true if m_currentPoint <= 1 pixel away from m_lastPoint
    // or if there was no lastPoint
    bool currentPointNextToLast () const;  // (includes diagonal adjacency)
    bool currentPointCardinallyNextToLast () const;  // (only cardinally adjacent i.e. horiz & vert; no diag)

    int m_mouseButton;  /* 0 = left, 1 = right */
    bool m_shiftPressed, m_controlPressed, m_altPressed;  // m_altPressed is unreliable
    bool m_beganDraw;  // set before beginDraw() is called, unset after endDraw() is called
    QPoint m_startPoint, m_currentPoint, m_lastPoint;

protected:
    friend class kpCommandHistory;
    friend class kpMainWindow;
    friend class kpToolToolBar;
    void beginInternal ();
    void endInternal ();

    void beginDrawInternal ();
    void endDrawInternal (const QPoint &thisPoint, const QRect &normalizedRect,
                          bool wantEndShape = false);
    void cancelShapeInternal ();
    void endShapeInternal (const QPoint &thisPoint = QPoint (),
                           const QRect &normalizedRect = QRect ());

    friend class kpView;

    // If you're reimplementing any of these, you probably don't know what
    // you're doing - reimplement begin(),beginDraw(),draw(),cancelShape(),
    // endDraw() etc. instead.
    virtual void mousePressEvent (QMouseEvent *e);
    virtual void mouseMoveEvent (QMouseEvent *e);
    virtual void mouseReleaseEvent (QMouseEvent *e);
    virtual void keyPressEvent (QKeyEvent *e);
    virtual void keyReleaseEvent (QKeyEvent *e);
private:
    void keyUpdateModifierState (QKeyEvent *e);
protected:
    virtual void setShiftPressed (bool pressed);
    virtual void setControlPressed (bool pressed);
    virtual void setAltPressed (bool pressed);
    virtual void focusInEvent (QFocusEvent *e);
    virtual void focusOutEvent (QFocusEvent *e);
    virtual void enterEvent (QEvent *e);
    virtual void leaveEvent (QEvent *e);

    // 0 = left, 1 = right, -1 = other (none, left+right, mid)
    static int mouseButton (const Qt::ButtonState &buttonState);

    QString m_text, m_description;
    const char *m_name;

    kpMainWindow *m_mainWindow;
    bool m_began;

    kpView *m_viewUnderStartPoint;
};

#endif  // __kptool_h__
