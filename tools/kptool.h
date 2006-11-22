
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


#ifndef KP_TOOL_H
#define KP_TOOL_H


#include <qobject.h>
#include <qpoint.h>
#include <qpointer.h>
#include <qrect.h>
#include <qsize.h>
#include <qstring.h>

#include <kpdefs.h>


class QFocusEvent;
class QIcon;
class QInputMethodEvent;
class QKeyEvent;
class QMouseEvent;
class QPixmap;
class QWheelEvent;

class KShortcut;

class kpColor;
class kpColorToolBar;
class kpCommandHistory;
class kpDocument;
class kpView;
class kpViewManager;
class kpMainWindow;
class kpToolAction;
class kpToolToolBar;


struct kpToolPrivate;

// Base class for all tools.
// TODO: rearrange method order to make sense and reflect kpTool_*.cpp split.
class kpTool : public QObject
{
Q_OBJECT

public:
    // <text> = user-visible name of the tool e.g. "Color Picker"
    // <description> = user-visible description used for tooltips
    //                 e.g. "Lets you select a color from the image"
    // <key> = optional shortcut key for switching to the tool, or 0 otherwise
    //         e.g. Qt::Key_C
    // <mainWindow> = pointer to the parent kpMainWindow
    // <name> = internal QObject name (not user-visible) e.g. "tool_color_picker"
    //          used for fetching the icon(), the name of the action() and
    //          debug printing.
    kpTool (const QString &text, const QString &description,
            int key,
            kpMainWindow *mainWindow, const QString &name);
    virtual ~kpTool ();

protected:
    void createAction ();

signals:
    void actionToolTipChanged (const QString &string);

protected slots:
    void slotActionToolTipChanged (const QString &string);

public:
    QString text () const;
    void setText (const QString &text);

    static QString toolTipForTextAndShortcut (const QString &text,
        const KShortcut &shortcut);
    QString toolTip () const;

    QString description () const;
    void setDescription (const QString &description);

    int key () const;
    void setKey (int key);

    // Given a single <key>, returns a shortcut with <key>
    // (disabled when the user is editing text) and as an alternate,
    // <some modifiers>+<key>.
    static KShortcut shortcutForKey (int key);
    KShortcut shortcut () const;


    static QRect neededRect (const QRect &rect, int lineWidth);
    static QPixmap neededPixmap (const QPixmap &pixmap, const QRect &boundingRect);

    bool hasCurrentPoint () const;
    // Returns the position of the cursor relative to the topleft point of
    // the current view (viewUnderStartPoint() or viewUnderCursor() otherwise).
    //
    // If neither viewUnderStartPoint() nor viewUnderCursor()
    // (i.e. !hasCurrentPoint()), then it returns KP_INVALID_POINT.
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

private:
    // Same as above except that you claim you know better than currentPoint()
    void somethingBelowTheCursorChanged (const QPoint &currentPoint_,
                                         const QPoint &currentViewPoint_);

public:
    // Called when the tool is selected.
    virtual void begin ();

    // Called when the tool is deselected.
    virtual void end ();

    // Returns true after begin() has been called but returns false after end()
    // after end() has been called.
    bool hasBegun () const;

    bool hasBegunDraw () const;

    virtual bool hasBegunShape () const;

    // Called when user double-left-clicks on a tool in the Tool Box.
    virtual void globalDraw ();

    // Called when the user clicks on a tool in the Tool Box even though it's
    // already the current tool (used by the selection tools to deselect).
    virtual void reselect ();

signals:
    // emitted after beginDraw() has been called
    void beganDraw (const QPoint &point);

    // Emitted just before draw() is called in mouseMoveEvent().  Slots
    // connected to this signal should return in <scrolled> whether the
    // mouse pos may have changed.  Used by drag scrolling.
    void movedAndAboutToDraw (const QPoint &currentPoint, const QPoint &lastPoint,
                              int zoomLevel,
                              bool *scrolled);

    // emitted after endDraw() has been called
    void endedDraw (const QPoint &point);

    // emitted after cancelShape() has been called
    void cancelledShape (const QPoint &point);


public:
    QIcon iconSet (int forceSize = 0) const;
    QString iconName () const;
    kpToolAction *action ();

signals:
    // User clicked on the tool's action - i.e. select this tool
    void actionActivated ();

protected slots:
    void slotActionActivated ();


protected:
    virtual bool returnToPreviousToolAfterEndDraw () const { return false; }
    virtual bool careAboutModifierState () const { return false; }
    virtual bool careAboutColorsSwapped () const { return false; }

    virtual void beginDraw ();

    // mouse move without button pressed
    // (only m_currentPoint & m_currentViewPoint is defined)
    virtual void hover (const QPoint &point);

    // this is useful for "instant" tools like the Pen & Eraser
    virtual void draw (const QPoint &thisPoint, const QPoint &lastPoint,
                        const QRect &normalizedRect);

    // (m_mouseButton will not change from beginDraw())
    virtual void cancelShape ();
    virtual void releasedAllButtons ();

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

    // TODO: does anyone actually use these?
    kpColor foregroundColor () const;
    kpColor backgroundColor () const;

    double colorSimilarity () const;
    int processedColorSimilarity () const;

protected slots:
    void slotColorsSwappedInternal (const kpColor &newForegroundColor,
                                    const kpColor &newBackgroundColor);
    void slotForegroundColorChangedInternal (const kpColor &color);
    void slotBackgroundColorChangedInternal (const kpColor &color);
    void slotColorSimilarityChangedInternal (double similarity, int processedSimilarity);

protected slots:  // TODO: there is no reason why these should be slots
    virtual void slotColorsSwapped (const kpColor & /*newForegroundColor*/, const kpColor & /*newBackgroundColor*/) {}
    virtual void slotForegroundColorChanged (const kpColor & /*color*/) {}
    virtual void slotBackgroundColorChanged (const kpColor & /*color*/) {}
    virtual void slotColorSimilarityChanged (double /*similarity*/, int /*processedSimilarity*/) {};

protected:
    // (only valid in slots connected to the respective signals above)
    kpColor oldForegroundColor () const;
    kpColor oldBackgroundColor () const;
    double oldColorSimilarity () const;

protected:
    // returns true if m_currentPoint <= 1 pixel away from m_lastPoint
    // or if there was no lastPoint
    bool currentPointNextToLast () const;  // (includes diagonal adjacency)
    bool currentPointCardinallyNextToLast () const;  // (only cardinally adjacent i.e. horiz & vert; no diag)

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

    // Note: _All_ events are forwarded from a kpView.
    //       The existence of a kpView implies the existence of a kpDocument.

    // If you're reimplementing any of these, you probably don't know what
    // you're doing - reimplement begin(),beginDraw(),draw(),cancelShape(),
    // endDraw() etc. instead.
    virtual void mousePressEvent (QMouseEvent *e);
    virtual void mouseMoveEvent (QMouseEvent *e);
    virtual void mouseReleaseEvent (QMouseEvent *e);
    virtual void wheelEvent (QWheelEvent *e);

    // WARNING: Do not call this "event()" as our QObject parent has a
    //          virtual function called that, that will pass us
    //          QObject events.  We only care about events forwarded by
    //          kpView.
    // TODO: rename mousePressEvent() -> viewMousePressEvent() etc.
    //       to remind us that events are coming from the view - the tool
    //       is not a visible object.
    virtual bool viewEvent (QEvent *e);

    void seeIfAndHandleModifierKey (QKeyEvent *e);
    
    void arrowKeyPressDirection (const QKeyEvent *e, int *dx, int *dy);
    void seeIfAndHandleArrowKeyPress (QKeyEvent *e);
    
    bool isDrawKey (int key);
    void seeIfAndHandleBeginDrawKeyPress (QKeyEvent *e);
    void seeIfAndHandleEndDrawKeyPress (QKeyEvent *e);

    virtual void keyPressEvent (QKeyEvent *e);
    virtual void keyReleaseEvent (QKeyEvent *e);

    virtual void inputMethodEvent (QInputMethodEvent *) {}
    
private:
    void keyUpdateModifierState (QKeyEvent *e);
    void notifyModifierStateChanged ();
protected:
    virtual void setShiftPressed (bool pressed);
    virtual void setControlPressed (bool pressed);
    virtual void setAltPressed (bool pressed);
    virtual void focusInEvent (QFocusEvent *e);
    virtual void focusOutEvent (QFocusEvent *e);
    virtual void enterEvent (QEvent *e);
    virtual void leaveEvent (QEvent *e);

    // 0 = left, 1 = right, -1 = other (none, left+right, mid)
    static int mouseButton (Qt::MouseButtons mouseButtons);

public:
    static int calculateLength (int start, int end);

    //
    // User Notifications (Status Bar)
    //

public:
    // Returns "(Left|Right) click to cancel." where Left or Right is chosen
    // depending on which one is the _opposite_ of <mouseButton>
    static QString cancelUserMessage (int mouseButton);
    QString cancelUserMessage () const;

    QString userMessage () const;
    void setUserMessage (const QString &userMessage = QString::null);

    QPoint userShapeStartPoint () const;
    QPoint userShapeEndPoint () const;
    void setUserShapePoints (const QPoint &startPoint = KP_INVALID_POINT,
                             const QPoint &endPoint = KP_INVALID_POINT,
                             bool setSize = true);

    QSize userShapeSize () const;
    int userShapeWidth () const;
    int userShapeHeight () const;
    void setUserShapeSize (const QSize &size = KP_INVALID_SIZE);
    void setUserShapeSize (int width, int height);

signals:
    void userMessageChanged (const QString &userMessage);
    void userShapePointsChanged (const QPoint &startPoint = KP_INVALID_POINT,
                                 const QPoint &endPoint = KP_INVALID_POINT);
    void userShapeSizeChanged (const QSize &size);
    void userShapeSizeChanged (int width, int height);


public:
    // Call this before the user tries to cause the document or selection
    // to resize from <oldWidth>x<oldHeight> to <newWidth>x<newHeight>.
    // If at least one dimension increases, the new dimensions will take a
    // large amount of memory (which causes thrashing, instability) and
    // the old dimensions did not take a large amount of memory, ask the
    // user if s/he really wants to perform the operation.
    //
    // Returns true if the operation should proceed, false otherwise.
    //
    // In order to make the translators' lives possible, this function cannot
    // generate the <text>,<caption> nor <continueButtonText> (without
    // concantenating sentences and words with tense).  However, it is
    // recommended that you give them the following values:
    //
    // e.g.:
    // text = i18n ("<qt><p>(Rotating|Skewing) the (image|selection) to"
    //              " %1x%2 may take a substantial amount of memory."
    //              " This can reduce system"
    //              " responsiveness and cause other application resource"
    //              " problems.</p>").arg (newWidth, newHeight)
    //
    //              "<p>Are you sure want to (rotate|skew) the"
    //              " (image|selection)?</p></qt>");
    // caption = i18n ("Rotate (Image|Selection)?");
    // continueButtonText = i18n ("Rotat&e (Image|Selection)");
    static bool warnIfBigImageSize (int oldWidth, int oldHeight,
                                    int newWidth, int newHeight,
                                    const QString &text,
                                    const QString &caption,
                                    const QString &continueButtonText,
                                    QWidget *parent);


// TODO: Don't expose variables to subclasses - otherwise they could mutate them
//       unexpectedly.  Provide const accessor methods instead.
protected:
    int m_mouseButton;  /* 0 = left, 1 = right */
    bool m_shiftPressed, m_controlPressed, m_altPressed;  // m_altPressed is unreliable
    QPoint m_startPoint,
           m_currentPoint, m_currentViewPoint,
           m_lastPoint;
           
    kpView *m_viewUnderStartPoint;

private:
    kpToolPrivate *d;
};


#endif  // KP_TOOL_H
