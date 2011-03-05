/*
   Copyright (c) 2003-2007 Clarence Dang <dang@kde.org>
   Copyright (c) 2011      Martin Koller <kollix@aon.at>
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


#include <kpToolToolBar.h>

#include <qboxlayout.h>
#include <qgridlayout.h>
#include <qbuttongroup.h>
#include <qdatetime.h>
#include <qevent.h>
#include <qtoolbutton.h>
#include <QMainWindow>

#include <kconfiggroup.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kicontheme.h>
#include <KIconLoader>

#include <kpDefs.h>
#include <kpTool.h>
#include <kpToolAction.h>
#include <kpToolWidgetBrush.h>
#include <kpToolWidgetEraserSize.h>
#include <kpToolWidgetFillStyle.h>
#include <kpToolWidgetLineWidth.h>
#include <kpToolWidgetOpaqueOrTransparent.h>
#include <kpToolWidgetSpraycanSize.h>

//---------------------------------------------------------------------

class kpToolButton : public QToolButton
{
public:
    kpToolButton (kpTool *tool, QWidget *parent)
        : QToolButton (parent),
          m_tool (tool)
    {
    }

protected:
    virtual void mouseDoubleClickEvent(QMouseEvent *e)
    {
        if (e->button () == Qt::LeftButton && m_tool)
            m_tool->globalDraw ();
    }

    kpTool *m_tool;
};

//---------------------------------------------------------------------

kpToolToolBar::kpToolToolBar(const QString &label, int colsOrRows, QWidget *parent)
    : KToolBar(label, parent),
      m_vertCols (colsOrRows),
      m_buttonGroup (0),
      m_baseWidget (0),
      m_baseLayout (0),
      m_toolLayout (0),
      m_previousTool (0), m_currentTool (0)
{
    setWindowTitle(label);

    m_baseWidget = new QWidget (this);

    m_toolWidgets.append (m_toolWidgetBrush =
        new kpToolWidgetBrush (m_baseWidget, "Tool Widget Brush"));
    m_toolWidgets.append (m_toolWidgetEraserSize =
        new kpToolWidgetEraserSize (m_baseWidget, "Tool Widget Eraser Size"));
    m_toolWidgets.append (m_toolWidgetFillStyle =
        new kpToolWidgetFillStyle (m_baseWidget, "Tool Widget Fill Style"));
    m_toolWidgets.append (m_toolWidgetLineWidth =
        new kpToolWidgetLineWidth (m_baseWidget, "Tool Widget Line Width"));
    m_toolWidgets.append (m_toolWidgetOpaqueOrTransparent =
        new kpToolWidgetOpaqueOrTransparent (m_baseWidget, "Tool Widget Opaque/Transparent"));
    m_toolWidgets.append (m_toolWidgetSpraycanSize =
        new kpToolWidgetSpraycanSize (m_baseWidget, "Tool Widget Spraycan Size"));

    for (QList <kpToolWidgetBase *>::const_iterator it = m_toolWidgets.constBegin ();
         it != m_toolWidgets.constEnd ();
         it++)
    {
        connect (*it, SIGNAL (optionSelected (int, int)),
                 this, SIGNAL (toolWidgetOptionSelected ()));
    }

    adjustToOrientation(orientation());
    connect(this, SIGNAL(orientationChanged(Qt::Orientation)),
            this, SLOT(adjustToOrientation(Qt::Orientation)));

    m_buttonGroup = new QButtonGroup (this);
    connect (m_buttonGroup, SIGNAL (buttonClicked (int)), SLOT (slotToolButtonClicked ()));

    hideAllToolWidgets ();

    addWidget(m_baseWidget);

    connect(this, SIGNAL(iconSizeChanged(const QSize &)),
            this, SLOT(slotIconSizeChanged(const QSize &)));
}

//---------------------------------------------------------------------

kpToolToolBar::~kpToolToolBar ()
{
    unregisterAllTools ();
}

//---------------------------------------------------------------------

// public
void kpToolToolBar::registerTool (kpTool *tool)
{
    for (QList <kpButtonToolPair>::const_iterator it = m_buttonToolPairs.constBegin ();
         it != m_buttonToolPairs.constEnd ();
         it++)
    {
        if ((*it).m_tool == tool)
            return;
    }
    int num = m_buttonToolPairs.count ();

    QToolButton *b = new kpToolButton (tool, m_baseWidget);
    b->setAutoRaise (true);
    b->setIconSize(iconSize());

    b->setToolButtonStyle(Qt::ToolButtonIconOnly);
    b->setCheckable(true);

    b->setText(tool->text ());
    b->setIcon(KIcon(tool->iconName()));
    b->setToolTip(tool->toolTip ());
    b->setWhatsThis(tool->description ());

    m_buttonGroup->addButton (b);
    addButton(b, orientation(), num);

    m_buttonToolPairs.append (kpButtonToolPair (b, tool));

    connect (tool, SIGNAL (actionActivated ()),
             this, SLOT (slotToolActionActivated ()));
    connect (tool, SIGNAL (actionToolTipChanged (const QString &)),
             this, SLOT (slotToolActionToolTipChanged ()));
}

//---------------------------------------------------------------------

// public
void kpToolToolBar::unregisterTool (kpTool *tool)
{
    for (QList <kpButtonToolPair>::iterator it = m_buttonToolPairs.begin ();
         it != m_buttonToolPairs.end ();
         it++)
    {
        if ((*it).m_tool == tool)
        {
            delete ((*it).m_button);
            m_buttonToolPairs.erase (it);

            disconnect (tool, SIGNAL (actionActivated ()),
                        this, SLOT (slotToolActionActivated ()));
            disconnect (tool, SIGNAL (actionToolTipChanged (const QString &)),
                        this, SLOT (slotToolActionToolTipChanged ()));
            break;
        }
    }
}

//---------------------------------------------------------------------

// public
void kpToolToolBar::unregisterAllTools ()
{
    for (QList <kpButtonToolPair>::iterator it = m_buttonToolPairs.begin ();
         it != m_buttonToolPairs.end ();
         it++)
    {
       delete ((*it).m_button);
    }

    m_buttonToolPairs.clear ();
}

//---------------------------------------------------------------------

// public
kpTool *kpToolToolBar::tool () const
{
    return m_currentTool;
}

//---------------------------------------------------------------------

// public
void kpToolToolBar::selectTool (const kpTool *tool, bool reselectIfSameTool)
{
#if DEBUG_KP_TOOL_TOOL_BAR
    kDebug () << "kpToolToolBar::selectTool (tool=" << tool
               << ") currentTool=" << m_currentTool
               << endl;
#endif

    if (!reselectIfSameTool && tool == m_currentTool)
        return;

    if (tool)
    {
        for (QList <kpButtonToolPair>::iterator it = m_buttonToolPairs.begin ();
            it != m_buttonToolPairs.end ();
            it++)
        {
            if ((*it).m_tool == tool)
            {
                (*it).m_button->setChecked (true);
                slotToolButtonClicked ();
                break;
            }
        }
    }
    else
    {
        QAbstractButton *b = m_buttonGroup->checkedButton();
    #if DEBUG_KP_TOOL_TOOL_BAR
        kDebug () << "\twant to select no tool - button selected=" << b;
    #endif
        if (b)
        {
            // HACK: qbuttongroup.html says the following about exclusive
            //       button groups:
            //
            //           "to untoggle a button you must click on another button
            //            in the group"
            //
            //       But we don't want any button to be selected.
            //       So don't be an exclusive button group temporarily.
            m_buttonGroup->setExclusive (false);
            b->setChecked (false);
            m_buttonGroup->setExclusive (true);

            slotToolButtonClicked ();
        }
    }
}

//---------------------------------------------------------------------

// public
kpTool *kpToolToolBar::previousTool () const
{
    return m_previousTool;
}

//---------------------------------------------------------------------

// public
void kpToolToolBar::selectPreviousTool ()
{
    selectTool (m_previousTool);
}

//---------------------------------------------------------------------

// public
void kpToolToolBar::hideAllToolWidgets ()
{
    for (QList <kpToolWidgetBase *>::const_iterator it = m_toolWidgets.constBegin ();
         it != m_toolWidgets.constEnd ();
         it++)
    {
        (*it)->hide ();
    }
}

//---------------------------------------------------------------------

// public
kpToolWidgetBase *kpToolToolBar::shownToolWidget (int which) const
{
    int uptoVisibleWidget = 0;

    for (QList <kpToolWidgetBase *>::const_iterator it = m_toolWidgets.constBegin ();
         it != m_toolWidgets.constEnd ();
         it++)
    {
        if (!(*it)->isHidden ())
        {
            if (which == uptoVisibleWidget)
                return *it;

            uptoVisibleWidget++;
        }
    }

    return 0;
}

//---------------------------------------------------------------------

// private slot
void kpToolToolBar::slotToolButtonClicked ()
{
    QAbstractButton *b = m_buttonGroup->checkedButton();

#if DEBUG_KP_TOOL_TOOL_BAR
    kDebug () << "kpToolToolBar::slotToolButtonClicked() button=" << b;
#endif

    kpTool *tool = 0;
    for (QList <kpButtonToolPair>::iterator it = m_buttonToolPairs.begin ();
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
    kDebug () << "\ttool=" << tool
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
    {
        kpToolAction *action = m_currentTool->action ();
        if (action)
        {
            action->setChecked (true);
        }

        m_currentTool->beginInternal ();
    }

    emit sigToolSelected (m_currentTool);
}

//---------------------------------------------------------------------

// private slot
void kpToolToolBar::slotToolActionActivated ()
{
    const kpTool *tool = dynamic_cast<const kpTool *>(sender());

#if DEBUG_KP_TOOL_TOOL_BAR
    kDebug () << "kpToolToolBar::slotToolActionActivated() tool="
               << (tool ? tool->objectName () : "null")
               << endl;
#endif

    if (m_currentTool)
    {
        // If the user clicks on the same KToggleAction, it unchecks it
        // - this is inconsistent with the Tool Box so always make sure it's
        // checked.
        kpToolAction *action = m_currentTool->action ();
        if (action)
        {
            action->setChecked (true);
        }
    }

    selectTool (tool, true/*reselect if same tool*/);
}

//---------------------------------------------------------------------

// private slot
void kpToolToolBar::slotToolActionToolTipChanged ()
{
    const kpTool *tool = dynamic_cast<const kpTool *>(sender());

#if DEBUG_KP_TOOL_TOOL_BAR
    kDebug () << "kpToolToolBar::slotToolActionToolTipChanged() tool="
               << (tool ? tool->objectName () : "null")
               << endl;
#endif

    if (!tool)
        return;

    for (QList <kpButtonToolPair>::const_iterator it = m_buttonToolPairs.constBegin ();
        it != m_buttonToolPairs.constEnd ();
        it++)
    {
        if (tool == (*it).m_tool)
        {
            (*it).m_button->setToolTip (tool->toolTip ());
            return;
        }
    }
}

//---------------------------------------------------------------------

// public
void kpToolToolBar::adjustToOrientation(Qt::Orientation o)
{
#if DEBUG_KP_TOOL_TOOL_BAR
    kDebug () << "kpToolToolBar::adjustToOrientation("
               << (o == Qt::Vertical ? "vertical" : "horizontal")
               << ") called!" << endl;
#endif

    delete m_baseLayout;
    if (o == Qt::Vertical)
    {
        m_baseLayout = new QBoxLayout (QBoxLayout::TopToBottom, m_baseWidget);
    }
    else // if (o == Qt::Horizontal)
    {
        m_baseLayout = new QBoxLayout (QBoxLayout::LeftToRight, m_baseWidget);
    }
    m_baseLayout->setSizeConstraint(QLayout::SetFixedSize);
    m_baseLayout->setSpacing (10);
    m_baseLayout->setMargin (5);

    m_toolLayout = new QGridLayout ();
    m_toolLayout->setMargin (0);
    m_toolLayout->setSpacing (0);
    // (ownership is transferred to m_baseLayout)
    m_baseLayout->addItem (m_toolLayout);

    int num = 0;

    for (QList <kpButtonToolPair>::iterator it = m_buttonToolPairs.begin ();
         it != m_buttonToolPairs.end ();
         it++)
    {
        addButton ((*it).m_button, o, num);
        num++;
    }

    for (QList <kpToolWidgetBase *>::const_iterator it = m_toolWidgets.constBegin ();
         it != m_toolWidgets.constEnd ();
         it++)
    {
        if (*it)
        {
            m_baseLayout->addWidget (*it,
                0/*stretch*/,
                o == Qt::Vertical ? Qt::AlignHCenter : Qt::AlignVCenter);
        }
    }

    adjustSizeConstraint();
}

//---------------------------------------------------------------------
// this makes the size handled correctly during dragging/undocking the toolbar

void kpToolToolBar::adjustSizeConstraint()
{
    // remove constraints
    setFixedSize(QSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX));

    if ( orientation() == Qt::Vertical )
      setFixedWidth(m_baseLayout->sizeHint().width());
    else
      setFixedHeight(m_baseLayout->sizeHint().height());
}

//---------------------------------------------------------------------

// private
void kpToolToolBar::addButton (QAbstractButton *button, Qt::Orientation o, int num)
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

//---------------------------------------------------------------------

void kpToolToolBar::slotIconSizeChanged(const QSize &size)
{
    for (QList <kpButtonToolPair>::iterator it = m_buttonToolPairs.begin ();
         it != m_buttonToolPairs.end ();
         it++)
    {
        (*it).m_button->setIconSize(size);
    }

    m_baseLayout->activate();
    adjustSizeConstraint();
}

//---------------------------------------------------------------------

#include <kpToolToolBar.moc>
