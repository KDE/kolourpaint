
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


#define DEBUG_KP_TOOL_TOOL_BAR 0

#include <qbuttongroup.h>
#include <qlayout.h>
#include <qtoolbutton.h>
#include <qtooltip.h>
#include <qwidget.h>
#include <qwhatsthis.h>

#include <kdebug.h>
#include <kiconloader.h>

#include <kpdefs.h>
#include <kptool.h>
#include <kptooltoolbar.h>

#include <kptoolwidgetbrush.h>
#include <kptoolwidgeterasersize.h>
#include <kptoolwidgetfillstyle.h>
#include <kptoolwidgetlinewidth.h>
#include <kptoolwidgetopaqueortransparent.h>
#include <kptoolwidgetspraycansize.h>


class kpToolButton : public QToolButton
{
public:
    kpToolButton (kpTool *tool, QWidget *parent)
        : QToolButton (parent),
          m_tool (tool)
    {
    }
    
    virtual ~kpToolButton ()
    {
    }
    
protected:
    // virtual [base QWidget]
    void mouseDoubleClickEvent (QMouseEvent *e)
    {
        if (e->button () == Qt::LeftButton && m_tool)
            m_tool->globalDraw ();
    }

    kpTool *m_tool;
};


kpToolToolBar::kpToolToolBar (kpMainWindow *mainWindow, int colsOrRows, const char *name)
    : KToolBar ((QWidget *) mainWindow, name, false/*don't use global toolBar settings*/, true/*readConfig*/),
      m_vertCols (colsOrRows),
      m_buttonGroup (0),
      m_baseWidget (0),
      m_baseLayout (0),
      m_toolLayout (0),
      m_previousTool (0), m_currentTool (0)
{
    setHorizontallyStretchable (false);
    setVerticallyStretchable (false);

    m_baseWidget = new QWidget (this);

    m_toolWidgetBrush = new kpToolWidgetBrush (m_baseWidget, "Tool Widget Brush");
    m_toolWidgetEraserSize = new kpToolWidgetEraserSize (m_baseWidget, "Tool Widget Eraser Size");
    m_toolWidgetFillStyle = new kpToolWidgetFillStyle (m_baseWidget, "Tool Widget Fill Style");
    m_toolWidgetLineWidth = new kpToolWidgetLineWidth (m_baseWidget, "Tool Widget Line Width");
    m_toolWidgetOpaqueOrTransparent = new kpToolWidgetOpaqueOrTransparent (m_baseWidget, "Tool Widget Opaque/Transparent");
    m_toolWidgetSpraycanSize = new kpToolWidgetSpraycanSize (m_baseWidget, "Tool Widget Spraycan Size");
    
    m_lastDockedOrientationSet = false;
    setOrientation (orientation ());

    m_buttonGroup = new QButtonGroup ();  // invisible
    m_buttonGroup->setExclusive (true);

    connect (m_buttonGroup, SIGNAL (clicked (int)), SLOT (slotToolSelected ()));

    hideAllToolWidgets ();
}

kpToolToolBar::~kpToolToolBar ()
{
    unregisterAllTools ();
    delete m_buttonGroup;
}


// public
void kpToolToolBar::registerTool (kpTool *tool)
{
    for (QValueVector <kpButtonToolPair>::ConstIterator it = m_buttonToolPairs.constBegin ();
         it != m_buttonToolPairs.constEnd ();
         it++)
    {
        if ((*it).m_tool == tool)
            return;
    }
    int num = m_buttonToolPairs.count ();

    QToolButton *b = new kpToolButton (tool, m_baseWidget);
    b->setAutoRaise (true);
    b->setUsesBigPixmap (false);
    b->setUsesTextLabel (false);
    b->setToggleButton (true);

    b->setText (tool->text ());
    b->setIconSet (BarIconSet (tool->name (), 16/*force size*/));
    QToolTip::add (b, tool->text ());
    QWhatsThis::add (b, tool->description ());

    m_buttonGroup->insert (b);
    addButton (b, orientation (), num);

    m_buttonToolPairs.append (kpButtonToolPair (b, tool));
}

// public
void kpToolToolBar::unregisterTool (kpTool *tool)
{
    for (QValueVector <kpButtonToolPair>::Iterator it = m_buttonToolPairs.begin ();
         it != m_buttonToolPairs.end ();
         it++)
    {
        if ((*it).m_tool == tool)
        {
            delete ((*it).m_button);
            m_buttonToolPairs.erase (it);
            break;
        }
    }
}

// public
void kpToolToolBar::unregisterAllTools ()
{
    for (QValueVector <kpButtonToolPair>::Iterator it = m_buttonToolPairs.begin ();
         it != m_buttonToolPairs.end ();
         it++)
    {
       delete ((*it).m_button);
    }

    m_buttonToolPairs.clear ();
}


// public
kpTool *kpToolToolBar::tool () const
{
    return m_currentTool;
}

// public
void kpToolToolBar::selectTool (kpTool *tool)
{
#if DEBUG_KP_TOOL_TOOL_BAR
    kdDebug () << "kpToolToolBar::selectTool (tool=" << tool
               << ") currentTool=" << m_currentTool
               << endl;
#endif

    if (tool == m_currentTool)
        return;

    if (tool)
    {
        for (QValueVector <kpButtonToolPair>::Iterator it = m_buttonToolPairs.begin ();
            it != m_buttonToolPairs.end ();
            it++)
        {
            if ((*it).m_tool == tool)
            {
                m_buttonGroup->setButton (m_buttonGroup->id ((*it).m_button));
                slotToolSelected ();
                break;
            }
        }
    }
    else
    {
        QButton *b = m_buttonGroup->selected ();
    #if DEBUG_KP_TOOL_TOOL_BAR
        kdDebug () << "\twant to select no tool - button selected=" << b << endl;
    #endif
        if (b)
        {
            b->toggle ();
            slotToolSelected ();
        }
    }
}


// public
kpTool *kpToolToolBar::previousTool () const
{
    return m_previousTool;
}

// public
void kpToolToolBar::selectPreviousTool ()
{
    selectTool (m_previousTool);
}


// public
void kpToolToolBar::hideAllToolWidgets ()
{
#define HIDE_WIDGET(w) if (w) w->hide ()
    HIDE_WIDGET (m_toolWidgetBrush);
    HIDE_WIDGET (m_toolWidgetEraserSize);
    HIDE_WIDGET (m_toolWidgetFillStyle);
    HIDE_WIDGET (m_toolWidgetLineWidth);
    HIDE_WIDGET (m_toolWidgetOpaqueOrTransparent);
    HIDE_WIDGET (m_toolWidgetSpraycanSize);
#undef HIDE_WIDGET
}


// private slot
void kpToolToolBar::slotToolSelected ()
{
    QButton *b = m_buttonGroup->selected ();

#if DEBUG_KP_TOOL_TOOL_BAR
    kdDebug () << "kpToolToolBar::slotToolSelected() button=" << b << endl;
#endif

    kpTool *tool = 0;
    for (QValueVector <kpButtonToolPair>::Iterator it = m_buttonToolPairs.begin ();
         it != m_buttonToolPairs.end ();
         it++)
    {
        if ((*it).m_button == b)
        {
            tool = (*it).m_tool;
            break;
        }
    }
    
#if DEBUG_KP_TOOL_TOOL_BAR
    kdDebug () << "\ttool=" << tool
               << " currentTool=" << m_currentTool
               << endl;
#endif

    if (tool == m_currentTool)
    {
        if (m_currentTool)
            m_currentTool->reselect ();

        return;
    }

    if (m_currentTool)
        m_currentTool->endInternal ();

    m_previousTool = m_currentTool;        
    m_currentTool = tool;
    
    if (m_currentTool)
        m_currentTool->beginInternal ();

    emit sigToolSelected (m_currentTool);
}


// public slot virtual [base QDockWindow]
void kpToolToolBar::setOrientation (Qt::Orientation o)
{
#if DEBUG_KP_TOOL_TOOL_BAR
    kdDebug () << "kpToolToolBar::setOrientation("
               << (o == Qt::Vertical ? "vertical" : "horizontal")
               << ") called!" << endl;
#endif

    // (QDockWindow::undock() calls us)
    bool isOutsideDock = (place () == QDockWindow::OutsideDock);

    if (!m_lastDockedOrientationSet || !isOutsideDock)
    {
        m_lastDockedOrientation = o;
        m_lastDockedOrientationSet = true;
    }
    
    if (isOutsideDock)
    {
    #if DEBUG_KP_TOOL_TOOL_BAR
        kdDebug () << "\toutside dock, forcing orientation to last" << endl;
    #endif
        o = m_lastDockedOrientation;
    }

    delete m_toolLayout;
    delete m_baseLayout;
    if (o == Qt::Vertical)
    {
        m_baseLayout = new QBoxLayout (m_baseWidget, QBoxLayout::TopToBottom,
                                       5/*margin*/,
                                       10/*spacing*/);
        m_toolLayout = new QGridLayout (m_baseLayout,
                                        5/*arbitrary rows since toolBar auto-expands*/,
                                        m_vertCols,
                                        0/*margin*/,
                                        0/*spacing*/);
    }
    else // if (o == Qt::Horizontal)
    {
        m_baseLayout = new QBoxLayout (m_baseWidget, QBoxLayout::LeftToRight,
                                       5/*margin*/,
                                       10/*spacing*/);
        m_toolLayout = new QGridLayout (m_baseLayout,
                                        m_vertCols/*rows in this case, since horiz*/,
                                        5/*arbitrary cols since toolBar auto-expands*/,
                                        0/*margin*/,
                                        0/*spacing*/);
    }

    int num = 0;
    
    for (QValueVector <kpButtonToolPair>::Iterator it = m_buttonToolPairs.begin ();
         it != m_buttonToolPairs.end ();
         it++)
    {
        addButton ((*it).m_button, o, num);
        num++;
    }

#define ADD_WIDGET(w)   \
{                       \
    if (w)              \
    {                   \
        m_baseLayout->addWidget (w, \
                                 0/*stretch*/,  \
                                 o == Qt::Vertical ? Qt::AlignHCenter : Qt::AlignVCenter);  \
    }                   \
}
    ADD_WIDGET (m_toolWidgetFillStyle);
    ADD_WIDGET (m_toolWidgetLineWidth);
    ADD_WIDGET (m_toolWidgetOpaqueOrTransparent);
    ADD_WIDGET (m_toolWidgetBrush);
    ADD_WIDGET (m_toolWidgetEraserSize);
    ADD_WIDGET (m_toolWidgetSpraycanSize);
#undef ADD_WIDGET

    KToolBar::setOrientation (o);
}

// private
void kpToolToolBar::addButton (QButton *button, Qt::Orientation o, int num)
{
    if (o == Qt::Vertical)
        m_toolLayout->addWidget (button, num / m_vertCols, num % m_vertCols);
    else
    {
        // maps Left (o = vertical) to Bottom (o = horizontal)
        int row = (m_vertCols - 1) - (num % m_vertCols);
        m_toolLayout->addWidget (button, row, num / m_vertCols);
    }
}

#include <kptooltoolbar.moc>
