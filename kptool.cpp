
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


#define DEBUG_KP_TOOL 0


#include <kptool.h>

#include <limits.h>

#include <qapplication.h>
#include <qclipboard.h>
#include <qcursor.h>
#include <qevent.h>
#include <qlayout.h>
#include <qpainter.h>
#include <qpixmap.h>

#include <kapplication.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>

#include <kpbug.h>
#include <kpcolor.h>
#include <kpcolortoolbar.h>
#include <kpdefs.h>
#include <kpmainwindow.h>
#include <kppixmapfx.h>
#include <kptoolaction.h>
#include <kptooltoolbar.h>
#include <kpview.h>
#include <kpviewmanager.h>
#include <kactioncollection.h>

//
// kpTool
//

struct kpToolPrivate
{
};


kpTool::kpTool (const QString &text, const QString &description,
                int key,
                kpMainWindow *mainWindow, const char *name)
    : QObject (mainWindow)
{
    init (text, description, key, mainWindow, name);
}

kpTool::~kpTool ()
{
    // before destructing, stop using the tool
    if (m_began)
        endInternal ();

    if (m_action)
    {
        if (m_mainWindow && m_mainWindow->actionCollection ())
            m_mainWindow->actionCollection ()->remove (m_action);
        else
            delete m_action;
    }

    delete d;
}


// private
void kpTool::init (const QString &text, const QString &description,
                   int key,
                   kpMainWindow *mainWindow, const char *name)
{
    d = new kpToolPrivate ();

    m_key = key;
    m_action = 0;
    m_ignoreColorSignals = 0;
    m_shiftPressed = false, m_controlPressed = false, m_altPressed = false;  // set in beginInternal()
    m_beganDraw = false;
    m_text = text, m_description = description, m_name = name;
    m_mainWindow = mainWindow;
    m_began = false;
    m_viewUnderStartPoint = 0;
    m_userShapeStartPoint = KP_INVALID_POINT;
    m_userShapeEndPoint = KP_INVALID_POINT;
    m_userShapeSize = KP_INVALID_SIZE;

    createAction ();
}


// private
void kpTool::createAction ()
{
#if DEBUG_KP_TOOL && 0
    kDebug () << "kpTool(" << name () << "::createAction()" << endl;
#endif

    if (!m_mainWindow)
    {
        kError () << "kpTool::createAction() without mw" << endl;
        return;
    }

    KActionCollection *ac = m_mainWindow->actionCollection ();
    if (!ac)
    {
        kError () << "kpTool::createAction() without ac" << endl;
        return;
    }


    if (m_action)
    {
    #if DEBUG_KP_TOOL
        kDebug () << "\tdeleting existing" << endl;
    #endif
        ac->remove (m_action);
        m_action = 0;
    }


    m_action = new kpToolAction (text (), iconName (), shortcutForKey (m_key),
                                 this, SLOT (slotActionActivated ()),
                                 m_mainWindow->actionCollection (), name ());
    //m_action->setExclusiveGroup (QLatin1String ("Tool Box Actions"));
    m_action->setWhatsThis (description ());

    connect (m_action, SIGNAL (toolTipChanged (const QString &)),
             this, SLOT (slotActionToolTipChanged (const QString &)));
}


// protected slot
void kpTool::slotActionToolTipChanged (const QString &string)
{
    emit actionToolTipChanged (string);
}


// public
QString kpTool::text () const
{
    return m_text;
}

// public
void kpTool::setText (const QString &text)
{
    m_text = text;

    if (m_action)
        m_action->setText (m_text);
    else
        createAction ();
}


static bool KeyIsText (int key)
{
    // TODO: should work like !QKeyEvent::text().isEmpty()
    return !(key & (Qt::KeyboardModifierMask ^ Qt::ShiftModifier));
}

// public static
QString kpTool::toolTipForTextAndShortcut (const QString &text,
                                           const KShortcut &shortcut)
{
    for (int i = 0; i < (int) shortcut.count (); i++)
    {
        const QKeySequence seq = shortcut.seq (i);
        if (seq.count () == 1 && ::KeyIsText (seq [0]))
        {
            return i18nc ("<Tool Name> (<Single Accel Key>)",
                          "%1 (%2)", text, seq.toString ().toUpper ());
        }
    }

    return text;
}

// public static
QString kpTool::toolTip () const
{
    return toolTipForTextAndShortcut (text (), shortcut ());
}


// public
int kpTool::key () const
{
    return m_key;
}

// public
void kpTool::setKey (int key)
{
    m_key = key;

    if (m_action)
        // TODO: this probably not wise since it nukes the user's settings
        m_action->setShortcut (shortcutForKey (m_key));
    else
        createAction ();
}

// public static
KShortcut kpTool::shortcutForKey (int key)
{
    KShortcut shortcut;

    if (key)
    {
        shortcut.append (KShortcut (key));
        // (CTRL+<key>, ALT+<key>, CTRL+ALT+<key>, CTRL+SHIFT+<key>
        //  all clash with global KDE shortcuts)
        shortcut.append (KShortcut (Qt::ALT + Qt::SHIFT + key));
    }

    return shortcut;
}

// public
KShortcut kpTool::shortcut () const
{
    return m_action ? m_action->shortcut () : KShortcut ();
}


// public
QString kpTool::description () const
{
    return m_description;
}

// public
void kpTool::setDescription (const QString &description)
{
    m_description = description;

    if (m_action)
        m_action->setWhatsThis (m_description);
    else
        createAction ();
}


// public
const char *kpTool::name () const
{
    return m_name;
}


// static
QRect kpTool::neededRect (const QRect &rect, int lineWidth)
{
    int x1, y1, x2, y2;
    rect.getCoords (&x1, &y1, &x2, &y2);

    if (lineWidth < 1)
        lineWidth = 1;

    return QRect (QPoint (x1 - lineWidth + 1, y1 - lineWidth + 1),
                  QPoint (x2 + lineWidth - 1, y2 + lineWidth - 1));
}

// static
QPixmap kpTool::neededPixmap (const QPixmap &pixmap, const QRect &boundingRect)
{
    return kpPixmapFX::getPixmapAt (pixmap, boundingRect);
}


// public
bool kpTool::hasCurrentPoint () const
{
    return (viewUnderStartPoint () || viewUnderCursor ());
}

// public
QPoint kpTool::currentPoint (bool zoomToDoc) const
{
#if DEBUG_KP_TOOL && 0
    kDebug () << "kpTool::currentPoint(zoomToDoc=" << zoomToDoc << ")" << endl;
    kDebug () << "\tviewUnderStartPoint="
               << (viewUnderStartPoint () ? viewUnderStartPoint ()->name () : "(none)")
               << " viewUnderCursor="
               << (viewUnderCursor () ? viewUnderCursor ()->name () : "(none)")
               << endl;
#endif

    kpView *v = viewUnderStartPoint ();
    if (!v)
    {
        v = viewUnderCursor ();
        if (!v)
        {
        #if DEBUG_KP_TOOL && 0
            kDebug () << "\tno view - returning sentinel" << endl;
        #endif
            return KP_INVALID_POINT;
        }
    }


    const QPoint globalPos = QCursor::pos ();
    const QPoint viewPos = v->mapFromGlobal (globalPos);
#if DEBUG_KP_TOOL && 0
    kDebug () << "\tglobalPos=" << globalPos
               << " viewPos=" << viewPos
               << endl;
#endif
    if (!zoomToDoc)
        return viewPos;


    const QPoint docPos = v->transformViewToDoc (viewPos);
#if DEBUG_KP_TOOL && 0
    kDebug () << "\tdocPos=" << docPos << endl;
#endif
    return docPos;
}


// public slot
void kpTool::somethingBelowTheCursorChanged ()
{
    somethingBelowTheCursorChanged (currentPoint (),
        currentPoint (false/*view point*/));
}

// private
// TODO: don't dup code from mouseMoveEvent()
void kpTool::somethingBelowTheCursorChanged (const QPoint &currentPoint_,
        const QPoint &currentViewPoint_)
{
#if DEBUG_KP_TOOL && 0
    kDebug () << "kpTool::somethingBelowTheCursorChanged(docPoint="
               << currentPoint_
               << " viewPoint="
               << currentViewPoint_
               << ")" << endl;
    kDebug () << "\tviewUnderStartPoint="
               << (viewUnderStartPoint () ? viewUnderStartPoint ()->name () : "(none)")
               << " viewUnderCursor="
               << (viewUnderCursor () ? viewUnderCursor ()->name () : "(none)")
               << endl;
    kDebug () << "\tbegan draw=" << m_beganDraw << endl;
#endif

    m_currentPoint = currentPoint_;
    m_currentViewPoint = currentViewPoint_;

    if (m_beganDraw)
    {
        if (m_currentPoint != KP_INVALID_POINT)
        {
            draw (m_currentPoint, m_lastPoint, kpBug::QRect_Normalized (QRect (m_startPoint, m_currentPoint)));
            m_lastPoint = m_currentPoint;
        }
    }
    else
    {
        hover (m_currentPoint);
    }
}


void kpTool::beginInternal ()
{
#if DEBUG_KP_TOOL
    kDebug () << "kpTool::beginInternal()" << endl;
#endif

    if (!m_began)
    {
        // clear leftover statusbar messages
        setUserMessage ();
        m_currentPoint = currentPoint ();
        m_currentViewPoint = currentPoint (false/*view point*/);
        setUserShapePoints (m_currentPoint);

        // TODO: Audit all the code in this file - states like "m_began" &
        //       "m_beganDraw" should be set before calling user func.
        //       Also, m_currentPoint should be more frequently initialised.

        // call user virtual func
        begin ();

        // we've starting using the tool...
        m_began = true;

        // but we haven't started drawing with it
        m_beganDraw = false;


        uint keyState = QApplication::keyboardModifiers ();

        m_shiftPressed = (keyState & Qt::ShiftModifier);
        m_controlPressed = (keyState & Qt::ControlModifier);

        // TODO: Can't do much about ALT - unless it's always KApplication::Modifier1?
        //       Ditto for everywhere else where I set SHIFT & CTRL but not alt.  COMPAT: supported by Qt
        m_altPressed = false;
    }
}

void kpTool::endInternal ()
{
    if (m_began)
    {
        // before we can stop using the tool, we must stop the current drawing operation (if any)
        if (hasBegunShape ())
            endShapeInternal (m_currentPoint, kpBug::QRect_Normalized (QRect (m_startPoint, m_currentPoint)));

        // call user virtual func
        end ();

        // clear leftover statusbar messages
        setUserMessage ();
        setUserShapePoints (currentPoint ());

        // we've stopped using the tool...
        m_began = false;

        // and so we can't be drawing with it
        m_beganDraw = false;

        if (m_mainWindow)
        {
            kpToolToolBar *tb = m_mainWindow->toolToolBar ();
            if (tb)
            {
                tb->hideAllToolWidgets ();
            }
        }

    }
}

// virtual
void kpTool::begin ()
{
#if DEBUG_KP_TOOL
    kDebug () << "kpTool::begin() base implementation" << endl;
#endif
}

// virtual
void kpTool::end ()
{
#if DEBUG_KP_TOOL
    kDebug () << "kpTool::end() base implementation" << endl;
#endif
}

void kpTool::beginDrawInternal ()
{
    if (!m_beganDraw)
    {
        beginDraw ();

        m_beganDraw = true;
        emit beganDraw (m_currentPoint);
    }
}

// virtual
void kpTool::beginDraw ()
{
}

// virtual
void kpTool::hover (const QPoint &point)
{
#if DEBUG_KP_TOOL
    kDebug () << "kpTool::hover" << point
               << " base implementation"
               << endl;
#endif

    setUserShapePoints (point);
}

// virtual
void kpTool::globalDraw ()
{
}

// virtual
void kpTool::reselect ()
{
#if DEBUG_KP_TOOL
    kDebug () << "kpTool::reselect() base implementation" << endl;
#endif
}


// public
QIcon kpTool::iconSet (int forceSize) const
{
#if DEBUG_KP_TOOL && 0
    kDebug () << "kpTool(" << name () << ")::iconSet(forceSize=" << forceSize << ")" << endl;
#endif
    // (robust in case BarIcon() default arg changes)
    if (forceSize > 0)
        return BarIconSet (name (), forceSize);
    else
        return BarIconSet (name ());
}

// public
QString kpTool::iconName () const
{
    return name ();
}

// public
kpToolAction *kpTool::action ()
{
    if (!m_action)
        createAction ();

    return m_action;
}


// protected slots
void kpTool::slotActionActivated ()
{
    emit actionActivated ();
}


// virtual
void kpTool::draw (const QPoint &, const QPoint &, const QRect &)
{
}

// also called by kpView
void kpTool::cancelShapeInternal ()
{
    if (hasBegunShape ())
    {
        m_beganDraw = false;
        cancelShape ();
        m_viewUnderStartPoint = 0;

        emit cancelledShape (viewUnderCursor () ? m_currentPoint : KP_INVALID_POINT);

        if (viewUnderCursor ())
            hover (m_currentPoint);
        else
        {
            m_currentPoint = KP_INVALID_POINT;
            m_currentViewPoint = KP_INVALID_POINT;
            hover (m_currentPoint);
        }

        if (returnToPreviousToolAfterEndDraw ())
        {
            kpToolToolBar *tb = mainWindow ()->toolToolBar ();
            
            // (don't end up with no tool selected)
            if (tb->previousTool ())
            {
                // endInternal() will be called by kpMainWindow (thanks to this line)
                // so we won't have the view anymore
                tb->selectPreviousTool ();
            }
        }
    }
}

// virtual
void kpTool::cancelShape ()
{
    kWarning () << "Tool cannot cancel operation!" << endl;
}

void kpTool::releasedAllButtons ()
{
}

void kpTool::endDrawInternal (const QPoint &thisPoint, const QRect &normalizedRect,
                              bool wantEndShape)
{
#if DEBUG_KP_TOOL && 1
    kDebug () << "kpTool::endDrawInternal() wantEndShape=" << wantEndShape << endl;
#endif

    if (wantEndShape && !hasBegunShape ())
        return;
    else if (!wantEndShape && !hasBegunDraw ())
        return;

    m_beganDraw = false;

    if (wantEndShape)
    {
    #if DEBUG_KP_TOOL && 0
        kDebug () << "\tcalling endShape()" << endl;
    #endif
        endShape (thisPoint, normalizedRect);
    }
    else
    {
    #if DEBUG_KP_TOOL && 0
        kDebug () << "\tcalling endDraw()" << endl;
    #endif
        endDraw (thisPoint, normalizedRect);
    }
    m_viewUnderStartPoint = 0;

    emit endedDraw (m_currentPoint);
    if (viewUnderCursor ())
        hover (m_currentPoint);
    else
    {
        m_currentPoint = KP_INVALID_POINT;
        m_currentViewPoint = KP_INVALID_POINT;
        hover (m_currentPoint);
    }

    if (returnToPreviousToolAfterEndDraw ())
    {
        kpToolToolBar *tb = mainWindow ()->toolToolBar ();
        
        // (don't end up with no tool selected)
        if (tb->previousTool ())
        {
            // endInternal() will be called by kpMainWindow (thanks to this line)
            // so we won't have the view anymore
            tb->selectPreviousTool ();
        }
    }
}

// private
void kpTool::endShapeInternal (const QPoint &thisPoint, const QRect &normalizedRect)
{
    endDrawInternal (thisPoint, normalizedRect, true/*end shape*/);
}

// virtual
void kpTool::endDraw (const QPoint &, const QRect &)
{
}

kpMainWindow *kpTool::mainWindow () const
{
    return m_mainWindow;
}

kpDocument *kpTool::document () const
{
    return m_mainWindow ? m_mainWindow->document () : 0;
}

kpView *kpTool::viewUnderCursor () const
{
    kpViewManager *vm = viewManager ();
    return vm ? vm->viewUnderCursor () : 0;
}

kpViewManager *kpTool::viewManager () const
{
    return m_mainWindow ? m_mainWindow->viewManager () : 0;
}

kpToolToolBar *kpTool::toolToolBar () const
{
    return m_mainWindow ? m_mainWindow->toolToolBar () : 0;
}

kpColor kpTool::color (int which) const
{
    if (m_mainWindow)
        return m_mainWindow->colorToolBar ()->color (which);
    else
    {
        kError () << "kpTool::color () without mainWindow" << endl;
        return kpColor::invalid;
    }
}

kpColor kpTool::foregroundColor () const
{
    return color (0);
}

kpColor kpTool::backgroundColor () const
{
    return color (1);
}


double kpTool::colorSimilarity () const
{
    if (m_mainWindow)
        return m_mainWindow->colorToolBar ()->colorSimilarity ();
    else
    {
        kError () << "kpTool::colorSimilarity() without mainWindow" << endl;
        return 0;
    }
}

int kpTool::processedColorSimilarity () const
{
    if (m_mainWindow)
        return m_mainWindow->colorToolBar ()->processedColorSimilarity ();
    else
    {
        kError () << "kpTool::processedColorSimilarity() without mainWindow" << endl;
        return kpColor::Exact;
    }
}


kpColor kpTool::oldForegroundColor () const
{
    if (m_mainWindow)
        return m_mainWindow->colorToolBar ()->oldForegroundColor ();
    else
    {
        kError () << "kpTool::oldForegroundColor() without mainWindow" << endl;
        return kpColor::invalid;
    }
}

kpColor kpTool::oldBackgroundColor () const
{
    if (m_mainWindow)
        return m_mainWindow->colorToolBar ()->oldBackgroundColor ();
    else
    {
        kError () << "kpTool::oldBackgroundColor() without mainWindow" << endl;
        return kpColor::invalid;
    }
}

double kpTool::oldColorSimilarity () const
{
    if (m_mainWindow)
        return m_mainWindow->colorToolBar ()->oldColorSimilarity ();
    else
    {
        kError () << "kpTool::oldColorSimilarity() without mainWindow" << endl;
        return 0;
    }
}


void kpTool::slotColorsSwappedInternal (const kpColor &newForegroundColor,
                                        const kpColor &newBackgroundColor)
{
    if (careAboutColorsSwapped ())
    {
        slotColorsSwapped (newForegroundColor, newBackgroundColor);
        m_ignoreColorSignals = 2;
    }
    else
        m_ignoreColorSignals = 0;
}

void kpTool::slotForegroundColorChangedInternal (const kpColor &color)
{
    if (m_ignoreColorSignals > 0)
    {
    #if DEBUG_KP_TOOL && 1
        kDebug () << "kpTool::slotForegroundColorChangedInternal() ignoreColorSignals=" << m_ignoreColorSignals << endl;
    #endif
        m_ignoreColorSignals--;
        return;
    }

    slotForegroundColorChanged (color);
}

void kpTool::slotBackgroundColorChangedInternal (const kpColor &color)
{
    if (m_ignoreColorSignals > 0)
    {
    #if DEBUG_KP_TOOL && 1
        kDebug () << "kpTool::slotBackgroundColorChangedInternal() ignoreColorSignals=" << m_ignoreColorSignals << endl;
    #endif
        m_ignoreColorSignals--;
        return;
    }

    slotBackgroundColorChanged (color);
}

void kpTool::slotColorSimilarityChangedInternal (double similarity, int processedSimilarity)
{
    slotColorSimilarityChanged (similarity, processedSimilarity);
}

bool kpTool::currentPointNextToLast () const
{
    if (m_lastPoint == QPoint (-1, -1))
        return true;

    int dx = qAbs (m_currentPoint.x () - m_lastPoint.x ());
    int dy = qAbs (m_currentPoint.y () - m_lastPoint.y ());

    return (dx <= 1 && dy <= 1);
}

bool kpTool::currentPointCardinallyNextToLast () const
{
    if (m_lastPoint == QPoint (-1, -1))
        return true;

    int dx = qAbs (m_currentPoint.x () - m_lastPoint.x ());
    int dy = qAbs (m_currentPoint.y () - m_lastPoint.y ());

    return (dx + dy <= 1);
}

kpCommandHistory *kpTool::commandHistory () const
{
    return m_mainWindow ? m_mainWindow->commandHistory () : 0;
}

void kpTool::mousePressEvent (QMouseEvent *e)
{
#if DEBUG_KP_TOOL && 1 || 1
    kDebug () << "kpTool::mousePressEvent pos=" << e->pos ()
               << " button=" << (int) e->button ()
               << " stateAfter: buttons=" << (int *) (int) e->buttons ()
               << " modifiers=" << (int *) (int) e->modifiers ()
               << " beganDraw=" << m_beganDraw << endl;
#endif

    if (m_mainWindow && e->button () == Qt::MidButton)
    {
        const QString text = QApplication::clipboard ()->text (QClipboard::Selection);
    #if DEBUG_KP_TOOL && 1 || 1
        kDebug () << "\tMMB pasteText='" << text << "'" << endl;
    #endif
        if (!text.isEmpty ())
        {
            if (hasBegunShape ())
            {
            #if DEBUG_KP_TOOL && 1 || 1
                kDebug () << "\t\thasBegunShape - end" << endl;
            #endif
                endShapeInternal (m_currentPoint,
                                  kpBug::QRect_Normalized (QRect (m_startPoint, m_currentPoint)));
            }

            if (viewUnderCursor ())
            {
                m_mainWindow->pasteTextAt (text,
                    viewUnderCursor ()->transformViewToDoc (e->pos ()),
                    true/*adjust topLeft so that cursor isn't
                          on top of resize handle*/);
            }

            return;
        }
    }

    int mb = mouseButton (e->buttons ());
#if DEBUG_KP_TOOL && 1 || 1
    kDebug () << "\tmb=" << mb << " m_beganDraw=" << m_beganDraw << endl;
#endif

    if (mb == -1 && !m_beganDraw) return; // ignore

    if (m_beganDraw)
    {
        if (mb == -1 || mb != m_mouseButton)
        {
        #if DEBUG_KP_TOOL && 1 || 1
            kDebug () << "\tCancelling operation as " << mb << " == -1 or != " << m_mouseButton << endl;
        #endif

            kpView *view = viewUnderStartPoint ();
            if (!view)
            {
                kError () << "kpTool::mousePressEvent() cancel without a view under the start point!" << endl;
            }

            // if we get a mousePressEvent when we're drawing, then the other
            // mouse button must have been pressed
            m_currentPoint = view ? view->transformViewToDoc (e->pos ()) : QPoint (-1, -1);
            m_currentViewPoint = view ? e->pos () : QPoint (-1, -1);
            cancelShapeInternal ();
        }

        return;
    }

    kpView *view = viewUnderCursor ();
    if (!view)
    {
        kError () << "kpTool::mousePressEvent() without a view under the cursor!" << endl;
    }

#if DEBUG_KP_TOOL && 1 || 1
    if (view)
        kDebug () << "\tview=" << view->objectName () << endl;
#endif


    // let user know what mouse button is being used for entire draw
    m_mouseButton = mouseButton (e->buttons ());
    m_shiftPressed = (e->modifiers () & Qt::ShiftModifier);
    m_controlPressed = (e->modifiers () & Qt::ControlModifier);
    m_altPressed = (e->modifiers () & Qt::AltModifier);
    m_startPoint = m_currentPoint = view ? view->transformViewToDoc (e->pos ()) : QPoint (-1, -1);
    m_currentViewPoint = view ? e->pos () : QPoint (-1, -1);
    m_viewUnderStartPoint = view;
    m_lastPoint = QPoint (-1, -1);

#if DEBUG_KP_TOOL && 1 || 1
    kDebug () << "\tBeginning draw @ " << m_currentPoint << endl;
#endif

    beginDrawInternal ();

    draw (m_currentPoint, m_lastPoint, QRect (m_currentPoint, m_currentPoint));
    m_lastPoint = m_currentPoint;
}

void kpTool::mouseMoveEvent (QMouseEvent *e)
{
#if DEBUG_KP_TOOL && 0
    kDebug () << "kpTool::mouseMoveEvent pos=" << e->pos ()
               << " stateAfter: buttons=" << (int *) (int) e->buttons ()
               << " modifiers=" << (int *) (int) e->modifiers ()
    kpView *v0 = viewUnderCursor (),
           *v1 = viewManager ()->viewUnderCursor (true/*use Qt*/),
           *v2 = viewUnderStartPoint ();
    kDebug () << "\tviewUnderCursor=" << (v0 ? v0->name () : "(none)")
               << " viewUnderCursorQt=" << (v1 ? v1->name () : "(none)")
               << " viewUnderStartPoint=" << (v2 ? v2->name () : "(none)")
               << endl;
    kDebug () << "\tfocusWidget=" << kapp->focusWidget () << endl;
#endif

    m_shiftPressed = (e->modifiers () & Qt::ShiftModifier);
    m_controlPressed = (e->modifiers () & Qt::ControlModifier);
    m_altPressed = (e->modifiers () & Qt::AltModifier);

    if (m_beganDraw)
    {
        kpView *view = viewUnderStartPoint ();
        if (!view)
        {
            kError () << "kpTool::mouseMoveEvent() without a view under the start point!" << endl;
            return;
        }

        m_currentPoint = view->transformViewToDoc (e->pos ());
        m_currentViewPoint = e->pos ();

    #if DEBUG_KP_TOOL && 0
        kDebug () << "\tDraw!" << endl;
    #endif

        bool dragScrolled = false;
        movedAndAboutToDraw (m_currentPoint, m_lastPoint, view->zoomLevelX (), &dragScrolled);

        if (dragScrolled)
        {
            m_currentPoint = currentPoint ();
            m_currentViewPoint = currentPoint (false/*view point*/);

            // Scrollview has scrolled contents and has scheduled an update
            // for the newly exposed region.  If draw() schedules an update
            // as well (instead of immediately updating), the scrollview's
            // update will be executed first and it'll only update part of
            // the screen resulting in ugly tearing of the viewManager's
            // tempPixmap.
            viewManager ()->setFastUpdates ();
        }

        draw (m_currentPoint, m_lastPoint, kpBug::QRect_Normalized (QRect (m_startPoint, m_currentPoint)));

        if (dragScrolled)
            viewManager ()->restoreFastUpdates ();

        m_lastPoint = m_currentPoint;
    }
    else
    {
        kpView *view = viewUnderCursor ();
        if (!view)  // possible if cancelShape()'ed but still holding down initial mousebtn
        {
            m_currentPoint = KP_INVALID_POINT;
            m_currentViewPoint = KP_INVALID_POINT;
            return;
        }

        m_currentPoint = view->transformViewToDoc (e->pos ());
        m_currentViewPoint = e->pos ();
        hover (m_currentPoint);
    }
}

void kpTool::mouseReleaseEvent (QMouseEvent *e)
{
#if DEBUG_KP_TOOL && 1 || 1
    kDebug () << "kpTool::mouseReleaseEvent pos=" << e->pos ()
               << " button=" << (int) e->button ()
               << " stateAfter: buttons=" << (int *) (int) e->buttons ()
               << " modifiers=" << (int *) (int) e->modifiers ()
               << " beganDraw=" << m_beganDraw << endl;
#endif

    if (m_beganDraw)  // didn't cancelShape()
    {
        kpView *view = viewUnderStartPoint ();
        if (!view)
        {
            kError () << "kpTool::mouseReleaseEvent() without a view under the start point!" << endl;
            return;
        }

        m_currentPoint = view->transformViewToDoc (e->pos ());
        m_currentViewPoint = e->pos ();
        endDrawInternal (m_currentPoint, kpBug::QRect_Normalized (QRect (m_startPoint, m_currentPoint)));
    }

    if ((e->buttons () & Qt::MouseButtonMask) == 0)
    {
        releasedAllButtons ();
    }
}

void kpTool::wheelEvent (QWheelEvent *e)
{
#if DEBUG_KP_TOOL || 1
    kDebug () << "kpTool::wheelEvent() modifiers=" << (int *) (int) e->modifiers ()
               << " hasBegunDraw=" << hasBegunDraw ()
               << " delta=" << e->delta ()
               << endl;
#endif

    e->ignore ();
    
    // If CTRL not pressed, bye.
    if ((e->modifiers () & Qt::ControlButton) == 0)
    {
    #if DEBUG_KP_TOOL || 1
        kDebug () << "\tno CTRL -> bye" << endl;
    #endif
        return;
    }
    
    // If drawing, bye; don't care if a shape in progress though.
    if (hasBegunDraw ())
    {
    #if DEBUG_KP_TOOL || 1
        kDebug () << "\thasBegunDraw() -> bye" << endl;
    #endif
        return;
    }
        
        
    // Zoom in/out depending on wheel direction.
    
    // Moved wheel away from user?
    if (e->delta () > 0)
    {
    #if DEBUG_KP_TOOL || 1
        kDebug () << "\tzoom in" << endl;
    #endif
        m_mainWindow->zoomIn (true/*center under cursor*/);
        e->accept ();
    }
    // Moved wheel towards user?
    else if (e->delta () < 0)
    {
    #if DEBUG_KP_TOOL || 1
        kDebug () << "\tzoom out" << endl;
    #endif
    #if 1
        m_mainWindow->zoomOut (true/*center under cursor - make zoom in/out
                                     stay under same doc pos*/);
    #else
        m_mainWindow->zoomOut (false/*don't center under cursor - as is
                                      confusing behaviour when zooming
                                      out*/);
    #endif
        e->accept ();
    }
}

bool kpTool::viewEvent (QEvent *e)
{
#if DEBUG_KP_TOOL
    kDebug () << "kpTool<" << name ()
              << "," << this << ">::viewEvent(type=" << e->type ()
              << ") returning false" << endl;
#else
    (void) e;
#endif

    // Don't handle.
    return false;
}


void kpTool::seeIfAndHandleModifierKey (QKeyEvent *e)
{
    switch (e->key ())
    {
    case 0:
    case Qt::Key_unknown:
    #if DEBUG_KP_TOOL && 0 || 1
        kDebug () << "kpTool::seeIfAndHandleModifierKey() picked up unknown key!" << endl;
    #endif
        // HACK: around Qt bug: if you hold a modifier before you start the
        //                      program and then release it over the view,
        //                      Qt reports it as the release of an unknown key
        //       Qt4 update: I don't think this happens anymore...
        // --- fall thru and update all modifiers ---

    case Qt::Key_Alt:
    case Qt::Key_Shift:
    case Qt::Key_Control:
    #if DEBUG_KP_TOOL && 0 || 1
        kDebug () << "kpTool::setIfAndHandleModifierKey() accepting" << endl;
    #endif
        keyUpdateModifierState (e);

        e->accept ();
        break;
    }
}


// Returns in <dx> and <dy> the direction the arrow key "e->key()" is
// pointing in or (0,0) if it's not a recognised arrow key.
void kpTool::arrowKeyPressDirection (const QKeyEvent *e, int *dx, int *dy)
{
    int dxLocal = 0, dyLocal = 0;

    switch (e->key ())
    {
    case Qt::Key_Home:     dxLocal = -1, dyLocal = -1;    break;
    case Qt::Key_Up:                     dyLocal = -1;    break;
    case Qt::Key_PageUp:   dxLocal = +1, dyLocal = -1;    break;

    case Qt::Key_Left:     dxLocal = -1;                  break;
    case Qt::Key_Right:    dxLocal = +1;                  break;

    case Qt::Key_End:      dxLocal = -1, dyLocal = +1;    break;
    case Qt::Key_Down:                   dyLocal = +1;    break;
    case Qt::Key_PageDown: dxLocal = +1, dyLocal = +1;    break;
    }

    if (dx)
        *dx = dxLocal;
    if (dy)
        *dy = dyLocal;
}

void kpTool::seeIfAndHandleArrowKeyPress (QKeyEvent *e)
{
    int dx, dy;

    arrowKeyPressDirection (e, &dx, &dy);
    if (dx == 0 && dy == 0)
        return;    

                
    kpView * const view = viewUnderCursor ();
    if (!view)
        return;

                
    const QPoint oldPoint = view->mapFromGlobal (QCursor::pos ());
#if DEBUG_KP_TOOL && 0 || 1
    kDebug () << "\toldPoint=" << oldPoint
                << " dx=" << dx << " dy=" << dy << endl;
#endif


    const int viewIncX = (dx ? qMax (1, view->zoomLevelX () / 100) * dx : 0);
    const int viewIncY = (dy ? qMax (1, view->zoomLevelY () / 100) * dy : 0);

    int newViewX = oldPoint.x () + viewIncX;
    int newViewY = oldPoint.y () + viewIncY;


#if DEBUG_KP_TOOL && 0 || 1
    kDebug () << "\tnewPoint=" << QPoint (newViewX, newViewY) << endl;
#endif

    // Make sure we really moved at least one doc point (needed due to
    // rounding error).

    if (view->transformViewToDoc (QPoint (newViewX, newViewY)) ==
        view->transformViewToDoc (oldPoint))
    {
        newViewX += viewIncX, newViewY += viewIncY;

    #if DEBUG_KP_TOOL && 0 || 1
        kDebug () << "\tneed adjust for doc - newPoint="
                    << QPoint (newViewX, newViewY) << endl;
    #endif
    }


    // TODO: visible width/height (e.g. with scrollbars)
    const int x = qMin (qMax (newViewX, 0), view->width () - 1);
    const int y = qMin (qMax (newViewY, 0), view->height () - 1);

    // QCursor::setPos conveniently causes mouseMoveEvents
    QCursor::setPos (view->mapToGlobal (QPoint (x, y)));
    e->accept ();
}


bool kpTool::isDrawKey (int key)
{
    return (key == Qt::Key_Enter ||
            key == Qt::Key_Return ||
            key == Qt::Key_Insert ||
            key == Qt::Key_Clear/*Numpad 5 Key*/ ||
            key == Qt::Key_L);
}

void kpTool::seeIfAndHandleBeginDrawKeyPress (QKeyEvent *e)
{
    if (e->isAutoRepeat ())
        return;

    if (!isDrawKey (e->key ()))
        return;
        
#if DEBUG_KP_TOOL && 0 || 1
    kDebug () << "kpTool::seeIfAndHandleBeginDrawKeyPress() accept" << endl;
#endif


    // TODO: wrong for dragging lines outside of view (for e.g.)
    kpView * const view = viewUnderCursor ();
    if (!view)
        return;
        
        
    // TODO: what about the modifiers?
    QMouseEvent me (QEvent::MouseButtonPress,
                    view->mapFromGlobal (QCursor::pos ()),
                    Qt::LeftButton,
                    Qt::LeftButton/*button state after event*/,
                    Qt::NoModifier);
    mousePressEvent (&me);
    e->accept ();
}

void kpTool::seeIfAndHandleEndDrawKeyPress (QKeyEvent *e)
{
#if DEBUG_KP_TOOL && 0 || 1
    kDebug () << "kpTool::setIfAndHandleEndDrawKeyPress() key=" << e->key ()
               << " isAutoRepeat=" << e->isAutoRepeat ()
               << " isDrawKey=" << isDrawKey (e->key ())
               << " view=" << viewUnderCursor ()
               << endl;
#endif

    if (e->isAutoRepeat ())
        return;

    if (!isDrawKey (e->key ()))
        return;
    
#if DEBUG_KP_TOOL && 0 || 1
    kDebug () << "kpTool::seeIfAndHandleEndDrawKeyPress() accept" << endl;
#endif
    
    
    kpView * const view = viewUnderCursor ();
    if (!view)
        return;
    
        
    // TODO: what about the modifiers?
    QMouseEvent me (QEvent::MouseButtonRelease,
                    view->mapFromGlobal (QCursor::pos ()),
                    Qt::LeftButton,
                    Qt::NoButton/*button state after event*/,
                    Qt::NoModifier);
    mouseReleaseEvent (&me);

    e->accept ();
}


void kpTool::keyPressEvent (QKeyEvent *e)
{
#if DEBUG_KP_TOOL && 0 || 1
    kDebug () << "kpTool::keyPressEvent() key=" << (int *) e->key ()
              << " stateAfter: modifiers=" << (int *) (int) e->modifiers ()
              << " isAutoRep=" << e->isAutoRepeat ()
              << endl;
#endif

    e->ignore ();


    seeIfAndHandleModifierKey (e);
    if (e->isAccepted ())
        return;
            
    seeIfAndHandleArrowKeyPress (e);
    if (e->isAccepted ())
        return;
        
    seeIfAndHandleBeginDrawKeyPress (e);
    if (e->isAccepted ())
        return;
    
            
    switch (e->key ())
    {
    case Qt::Key_Delete:
        m_mainWindow->slotDelete ();
        break;

    case Qt::Key_Escape:
        if (hasBegunDraw ())
        {
            cancelShapeInternal ();
            e->accept ();
        }

        break;
    }
}

void kpTool::keyReleaseEvent (QKeyEvent *e)
{
#if DEBUG_KP_TOOL && 0 || 1
    kDebug () << "kpTool::keyReleaseEvent() key=" << (int *) e->key ()
              << " stateAfter: modifiers=" << (int *) (int) e->modifiers ()
              << " isAutoRep=" << e->isAutoRepeat ()
              << endl;
#endif

    e->ignore ();

    
    seeIfAndHandleModifierKey (e);
    if (e->isAccepted ())
        return;
            
    seeIfAndHandleEndDrawKeyPress (e);
    if (e->isAccepted ())
        return;
}


// private
void kpTool::keyUpdateModifierState (QKeyEvent *e)
{
#if DEBUG_KP_TOOL && 0 || 1
    kDebug () << "kpTool::keyUpdateModifierState() e->key=" << (int *) e->key () << endl;
    kDebug () << "\tshift="
               << (e->modifiers () & Qt::ShiftModifier)
               << " control="
               << (e->modifiers () & Qt::ControlModifier)
               << " alt="
               << (e->modifiers () & Qt::AltModifier)
               << endl;
#endif
    if (e->key () & (Qt::Key_Alt | Qt::Key_Shift | Qt::Key_Control))
    {
    #if DEBUG_KP_TOOL && 0 || 1
        kDebug () << "\t\tmodifier changed - use e's claims" << endl;
    #endif
        setShiftPressed (e->modifiers () & Qt::ShiftModifier);
        setControlPressed (e->modifiers () & Qt::ControlModifier);
        setAltPressed (e->modifiers () & Qt::AltModifier);
    }
    // See seeIfAndHandleModifierKey() for why this code path exists.
    else
    {
    #if DEBUG_KP_TOOL && 0 || 1
        kDebug () << "\t\tmodifiers not changed - figure out the truth" << endl;
    #endif
        const Qt::KeyboardModifiers keyState = QApplication::keyboardModifiers ();

        setShiftPressed (keyState & Qt::ShiftModifier);
        setControlPressed (keyState & Qt::ControlModifier);
        setAltPressed (keyState & Qt::AltModifier);
    }
}


void kpTool::notifyModifierStateChanged ()
{
    if (careAboutModifierState ())
    {
        if (m_beganDraw)
            draw (m_currentPoint, m_lastPoint, kpBug::QRect_Normalized (QRect (m_startPoint, m_currentPoint)));
        else
        {
            m_currentPoint = currentPoint ();
            m_currentViewPoint = currentPoint (false/*view point*/);
            hover (m_currentPoint);
        }
    }
}

void kpTool::setShiftPressed (bool pressed)
{
    if (pressed == m_shiftPressed)
        return;

    m_shiftPressed = pressed;

    notifyModifierStateChanged ();
}

void kpTool::setControlPressed (bool pressed)
{
    if (pressed == m_controlPressed)
        return;

    m_controlPressed = pressed;

    notifyModifierStateChanged ();
}

void kpTool::setAltPressed (bool pressed)
{
    if (pressed = m_altPressed)
        return;

    m_altPressed = pressed;

    notifyModifierStateChanged ();
}

void kpTool::focusInEvent (QFocusEvent *)
{
}

void kpTool::focusOutEvent (QFocusEvent *)
{
#if DEBUG_KP_TOOL && 0
    kDebug () << "kpTool::focusOutEvent() beganDraw=" << m_beganDraw << endl;
#endif

    if (m_beganDraw)
        endDrawInternal (m_currentPoint, kpBug::QRect_Normalized (QRect (m_startPoint, m_currentPoint)));
}

void kpTool::enterEvent (QEvent *)
{
#if DEBUG_KP_TOOL && 1
    kDebug () << "kpTool::enterEvent() beganDraw=" << m_beganDraw << endl;
#endif
}

void kpTool::leaveEvent (QEvent *)
{
#if DEBUG_KP_TOOL && 1
    kDebug () << "kpTool::leaveEvent() beganDraw=" << m_beganDraw << endl;
#endif

    // if we haven't started drawing (e.g. dragging a rectangle)...
    if (!m_beganDraw)
    {
        m_currentPoint = KP_INVALID_POINT;
        m_currentViewPoint = KP_INVALID_POINT;
        hover (m_currentPoint);
    }
}

// static
// TODO: we don't handle Qt::XButton1 and Qt::XButton2 at the moment.
int kpTool::mouseButton (Qt::MouseButtons mouseButtons)
{
    // we have nothing to do with mid-buttons
    if (mouseButtons & Qt::MidButton)
        return -1;

    // both left & right together is quite meaningless...
    const Qt::MouseButtons bothButtons = (Qt::LeftButton | Qt::RightButton);
    if ((mouseButtons & bothButtons) == bothButtons)
        return -1;

    if (mouseButtons & Qt::LeftButton)
        return 0;
    else if (mouseButtons & Qt::RightButton)
        return 1;
    else
        return -1;
}


/*
 * User Notifications
 */


// public static
QString kpTool::cancelUserMessage (int mouseButton)
{
    if (mouseButton == 0)
        return i18n ("Right click to cancel.");
    else
        return i18n ("Left click to cancel.");
}

// public
QString kpTool::cancelUserMessage () const
{
    return cancelUserMessage (m_mouseButton);
}


// public
QString kpTool::userMessage () const
{
    return m_userMessage;
}

// public
void kpTool::setUserMessage (const QString &userMessage)
{
    m_userMessage = userMessage;

    if (m_userMessage.isEmpty ())
        m_userMessage = text ();
    else
        m_userMessage.prepend (i18n ("%1: ", text ()));

    emit userMessageChanged (m_userMessage);
}


// public
QPoint kpTool::userShapeStartPoint () const
{
    return m_userShapeStartPoint;
}

// public
QPoint kpTool::userShapeEndPoint () const
{
    return m_userShapeEndPoint;
}

// public static
int kpTool::calculateLength (int start, int end)
{
    if (start <= end)
    {
        return end - start + 1;
    }
    else
    {
        return end - start - 1;
    }
}

// public
void kpTool::setUserShapePoints (const QPoint &startPoint,
                                 const QPoint &endPoint,
                                 bool setSize)
{
    m_userShapeStartPoint = startPoint;
    m_userShapeEndPoint = endPoint;
    emit userShapePointsChanged (m_userShapeStartPoint, m_userShapeEndPoint);

    if (setSize)
    {
        if (startPoint != KP_INVALID_POINT &&
            endPoint != KP_INVALID_POINT)
        {
            setUserShapeSize (calculateLength (startPoint.x (), endPoint.x ()),
                              calculateLength (startPoint.y (), endPoint.y ()));
        }
        else
        {
            setUserShapeSize ();
        }
    }
}


// public
QSize kpTool::userShapeSize () const
{
    return m_userShapeSize;
}

// public
int kpTool::userShapeWidth () const
{
    return m_userShapeSize.width ();
}

// public
int kpTool::userShapeHeight () const
{
    return m_userShapeSize.height ();
}

// public
void kpTool::setUserShapeSize (const QSize &size)
{
    m_userShapeSize = size;

    emit userShapeSizeChanged (m_userShapeSize);
    emit userShapeSizeChanged (m_userShapeSize.width (),
                               m_userShapeSize.height ());
}

// public
void kpTool::setUserShapeSize (int width, int height)
{
    setUserShapeSize (QSize (width, height));
}


// public static
bool kpTool::warnIfBigImageSize (int oldWidth, int oldHeight,
                                 int newWidth, int newHeight,
                                 const QString &text,
                                 const QString &caption,
                                 const QString &continueButtonText,
                                 QWidget *parent)
{
#if DEBUG_KP_TOOL
    kDebug () << "kpTool::warnIfBigImageSize()"
               << " old: w=" << oldWidth << " h=" << oldWidth
               << " new: w=" << newWidth << " h=" << newHeight
               << " pixmapSize="
               << kpPixmapFX::pixmapSize (newWidth,
                                          newHeight,
                                          QPixmap::defaultDepth ())
               << " vs BigImageSize=" << KP_BIG_IMAGE_SIZE
               << endl;
#endif

    // Only got smaller or unchanged - don't complain
    if (!(newWidth > oldWidth || newHeight > oldHeight))
    {
        return true;
    }

    // Was already large - user was warned before, don't annoy him/her again
    if (kpPixmapFX::pixmapSize (oldWidth, oldHeight, QPixmap::defaultDepth ()) >=
        KP_BIG_IMAGE_SIZE)
    {
        return true;
    }

    if (kpPixmapFX::pixmapSize (newWidth, newHeight, QPixmap::defaultDepth ()) >=
        KP_BIG_IMAGE_SIZE)
    {
        int accept = KMessageBox::warningContinueCancel (parent,
            text,
            caption,
            continueButtonText,
            QLatin1String ("BigImageDontAskAgain"));

        return (accept == KMessageBox::Continue);
    }
    else
    {
        return true;
    }
}


#include <kptool.moc>

