
/* This file is part of the KolourPaint project
   Copyright (c) 2003 Clarence Dang <dang@kde.org>
   All rights reserved.
   
   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:
   
   1. Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
   2. Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
   3. Neither the names of the copyright holders nor the names of
      contributors may be used to endorse or promote products derived from
      this software without specific prior written permission.
   
   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
   PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
   HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#define DEBUG_KP_TOOL_TOOL_BAR 1

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
#include <kptoolwidgetlinestyle.h>
#include <kptoolwidgetlinewidth.h>

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

    m_toolWidgetBrush = new kpToolWidgetBrush (m_baseWidget);
    m_toolWidgetEraserSize = new kpToolWidgetEraserSize (m_baseWidget);
    m_toolWidgetFillStyle = new kpToolWidgetFillStyle (m_baseWidget);
    m_toolWidgetLineStyle = new kpToolWidgetLineStyle (m_baseWidget);
    m_toolWidgetLineWidth = new kpToolWidgetLineWidth (m_baseWidget);
    
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

    QToolButton *b = new QToolButton (m_baseWidget);
    b->setAutoRaise (true);
    b->setUsesBigPixmap (false);
    b->setUsesTextLabel (false);
    b->setToggleButton (true);

    b->setText (tool->text ());
    b->setIconSet (BarIconSet (tool->name ()));
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
    m_toolWidgetBrush->hide ();
    m_toolWidgetEraserSize->hide ();
    m_toolWidgetFillStyle->hide ();
    m_toolWidgetLineStyle->hide ();
    m_toolWidgetLineWidth->hide ();
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
        return;

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
    kdDebug () << "kpToolToolBar::setOrientation("
               << (o == Qt::Vertical ? "vertical" : "horizontal")
               << ") called!" << endl;

    // (QDockWindow::undock() calls us)
    bool isOutsideDock = (place () == QDockWindow::OutsideDock);

    if (!m_lastDockedOrientationSet || !isOutsideDock)
    {
        m_lastDockedOrientation = o;
        m_lastDockedOrientationSet = true;
    }
    
    if (isOutsideDock)
    {
        kdDebug () << "\toutside dock, forcing orientation to last" << endl;
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

    m_baseLayout->addWidget (m_toolWidgetFillStyle);
    m_baseLayout->addWidget (m_toolWidgetLineWidth);
    m_baseLayout->addWidget (m_toolWidgetLineStyle);
    m_baseLayout->addWidget (m_toolWidgetBrush);
    m_baseLayout->addWidget (m_toolWidgetEraserSize);

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
