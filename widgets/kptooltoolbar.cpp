
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

#define DEBUG_KPTOOLTOOLBAR 1

kpToolToolBar::kpToolToolBar (kpMainWindow *mainWindow, int colsOrRows, const char *name)
    : KToolBar ((QWidget *) mainWindow, name, false/*don't use global toolBar settings*/, true/*readConfig*/),
      m_vertCols (colsOrRows),
      m_buttonGroup (0),
      m_baseWidget (0),
      m_baseLayout (0),
      m_toolLayout (0)
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

void kpToolToolBar::registerTool (kpTool *tool)
{
    for (QMap <QButton *, kpTool *>::ConstIterator it = m_tools.constBegin (); it != m_tools.constEnd (); it++)
    {
        if ((*it) == tool)
            return;
    }
    int num = m_tools.count ();

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

    m_tools.insert (b, tool);
}

void kpToolToolBar::unregisterTool (kpTool *tool)
{
    for (QMap <QButton *, kpTool *>::Iterator it = m_tools.begin (); it != m_tools.end (); it++)
    {
        if ((*it) == tool)
        {
            delete (it.key ());  // button
            m_tools.remove (it);
            break;
        }
    }
}

void kpToolToolBar::unregisterAllTools ()
{
   for (QMap <QButton *, kpTool *>::Iterator it = m_tools.begin (); it != m_tools.end (); it++)
   {
       delete (it.key ());  // button
   }

   m_tools.clear ();
}

void kpToolToolBar::selectTool (kpTool *tool)
{
    for (QMap <QButton *, kpTool *>::Iterator it = m_tools.begin (); it != m_tools.end (); it++)
    {
        if ((*it) == tool)
        {
            m_buttonGroup->setButton (m_buttonGroup->id (it.key ()));
            slotToolSelected ();
            break;
        }
    }
}

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

    kdDebug () << "kpToolToolBar::slotToolSelection() button=" << b
               << " tool=" << m_tools [b]
               << endl;
    emit toolSelected (m_tools [b]);
}

// virtual
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
    for (QMap <QButton *, kpTool *>::Iterator it = m_tools.begin ();
         it != m_tools.end ();
         it++)
    {
        addButton (it.key (), o, num);
        num++;
    }

    m_baseLayout->addWidget (m_toolWidgetFillStyle);
    m_baseLayout->addWidget (m_toolWidgetLineWidth);
    m_baseLayout->addWidget (m_toolWidgetLineStyle);
    m_baseLayout->addWidget (m_toolWidgetBrush);
    m_baseLayout->addWidget (m_toolWidgetEraserSize);

    KToolBar::setOrientation (o);
}

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
