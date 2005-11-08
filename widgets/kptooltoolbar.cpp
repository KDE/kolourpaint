
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


#define DEBUG_KP_TOOL_TOOL_BAR 0


#include <kptooltoolbar.h>

#include <qbuttongroup.h>
#include <qlayout.h>
#include <qdatetime.h>
#include <qtoolbutton.h>
#include <qtooltip.h>
#include <qwidget.h>
#include <qwhatsthis.h>

#include <kconfig.h>
#include <kdebug.h>
#include <kglobalsettings.h>
#include <kicontheme.h>

#include <kpdefs.h>
#include <kptool.h>
#include <kptoolaction.h>
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


kpToolToolBar::kpToolToolBar (const QString &label, kpMainWindow *mainWindow, int colsOrRows, const char *name)
    : KToolBar ((QWidget *) mainWindow, name, false/*don't use global toolBar settings*/, true/*readConfig*/),
      m_vertCols (colsOrRows),
      m_buttonGroup (0),
      m_baseWidget (0),
      m_baseLayout (0),
      m_toolLayout (0),
      m_previousTool (0), m_currentTool (0),
      m_defaultIconSize (0)
{
    setText (label);


    // With these lines enabled, mousePressEvent's weren't being generated
    // when right clicking in empty part of the toolbar (each call affects
    // the toolbar in its respective orientation).  They don't seem to be
    // needed anyway since !isResizeEnabled().

    //setHorizontallyStretchable (false);
    //setVerticallyStretchable (false);


    m_baseWidget = new QWidget (this);

#if DEBUG_KP_TOOL_TOOL_BAR
    QTime timer;
    timer.start ();
#endif

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

#if DEBUG_KP_TOOL_TOOL_BAR
    kdDebug () << "kpToolToolBar::<ctor> create tool widgets msec="
               << timer.restart () << endl;
#endif

    for (QValueVector <kpToolWidgetBase *>::const_iterator it = m_toolWidgets.begin ();
         it != m_toolWidgets.end ();
         it++)
    {
        connect (*it, SIGNAL (optionSelected (int, int)),
                 this, SIGNAL (toolWidgetOptionSelected ()));
    }

#if DEBUG_KP_TOOL_TOOL_BAR
    kdDebug () << "kpToolToolBar::<ctor> connect widgets msec="
               << timer.restart () << endl;
#endif

    m_lastDockedOrientationSet = false;
    setOrientation (orientation ());

#if DEBUG_KP_TOOL_TOOL_BAR
    kdDebug () << "kpToolToolBar::<ctor> layout tool widgets msec="
               << timer.elapsed () << endl;
#endif

    m_buttonGroup = new QButtonGroup ();  // invisible
    m_buttonGroup->setExclusive (true);

    connect (m_buttonGroup, SIGNAL (clicked (int)), SLOT (slotToolButtonClicked ()));

    hideAllToolWidgets ();
}

kpToolToolBar::~kpToolToolBar ()
{
    unregisterAllTools ();
    delete m_buttonGroup;
}


// private
int kpToolToolBar::defaultIconSize ()
{
    // Cached?
    if (m_defaultIconSize > 0)
        return m_defaultIconSize;

#if DEBUG_KP_TOOL_TOOL_BAR
    kdDebug () << "kpToolToolBar::defaultIconSize()" << endl;
#endif


    KConfigGroupSaver cfgGroupSaver (KGlobal::config (),
                                     kpSettingsGroupTools);
    KConfigBase *cfg = cfgGroupSaver.config ();

    if (cfg->hasKey (kpSettingToolBoxIconSize))
    {
        m_defaultIconSize = cfg->readNumEntry (kpSettingToolBoxIconSize);
    #if DEBUG_KP_TOOL_TOOL_BAR
        kdDebug () << "\tread: " << m_defaultIconSize << endl;
    #endif
    }
    else
    {
        m_defaultIconSize = -1;
#if DEBUG_KP_TOOL_TOOL_BAR
        kdDebug () << "\tfirst time - writing default: " << m_defaultIconSize << endl;
#endif
        cfg->writeEntry (kpSettingToolBoxIconSize, m_defaultIconSize);
        cfg->sync ();
    }


    if (m_defaultIconSize <= 0)
    {
        // Adapt according to screen geometry
        const QRect desktopSize = KGlobalSettings::desktopGeometry (this);
    #if DEBUG_KP_TOOL_TOOL_BAR
        kdDebug () << "\tadapting to screen size=" << desktopSize << endl;
    #endif

        if (desktopSize.width () >= 1024 && desktopSize.height () >= 768)
            m_defaultIconSize = KIcon::SizeSmallMedium/*22x22*/;
        else
            m_defaultIconSize = KIcon::SizeSmall/*16x16*/;
    }


#if DEBUG_KP_TOOL_TOOL_BAR
    kdDebug () << "\treturning " << m_defaultIconSize << endl;
#endif
    return m_defaultIconSize;
}

// public
void kpToolToolBar::registerTool (kpTool *tool)
{
    for (QValueVector <kpButtonToolPair>::const_iterator it = m_buttonToolPairs.begin ();
         it != m_buttonToolPairs.end ();
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
    b->setIconSet (tool->iconSet (defaultIconSize ()));
    QToolTip::add (b, tool->toolTip ());
    QWhatsThis::add (b, tool->description ());

    m_buttonGroup->insert (b);
    addButton (b, orientation (), num);

    m_buttonToolPairs.append (kpButtonToolPair (b, tool));


    connect (tool, SIGNAL (actionActivated ()),
             this, SLOT (slotToolActionActivated ()));
    connect (tool, SIGNAL (actionToolTipChanged (const QString &)),
             this, SLOT (slotToolActionToolTipChanged ()));
}

// public
void kpToolToolBar::unregisterTool (kpTool *tool)
{
    for (QValueVector <kpButtonToolPair>::iterator it = m_buttonToolPairs.begin ();
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

// public
void kpToolToolBar::unregisterAllTools ()
{
    for (QValueVector <kpButtonToolPair>::iterator it = m_buttonToolPairs.begin ();
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
void kpToolToolBar::selectTool (const kpTool *tool, bool reselectIfSameTool)
{
#if DEBUG_KP_TOOL_TOOL_BAR
    kdDebug () << "kpToolToolBar::selectTool (tool=" << tool
               << ") currentTool=" << m_currentTool
               << endl;
#endif

    if (!reselectIfSameTool && tool == m_currentTool)
        return;

    if (tool)
    {
        for (QValueVector <kpButtonToolPair>::iterator it = m_buttonToolPairs.begin ();
            it != m_buttonToolPairs.end ();
            it++)
        {
            if ((*it).m_tool == tool)
            {
                m_buttonGroup->setButton (m_buttonGroup->id ((*it).m_button));
                slotToolButtonClicked ();
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
            slotToolButtonClicked ();
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
    for (QValueVector <kpToolWidgetBase *>::const_iterator it = m_toolWidgets.begin ();
         it != m_toolWidgets.end ();
         it++)
    {
        (*it)->hide ();
    }
}

// public
int kpToolToolBar::numShownToolWidgets () const
{
#if DEBUG_KP_TOOL_TOOL_BAR
    kdDebug () << "kpToolToolBar::numShownToolWidgets()" << endl;
#endif

    int ret = 0;

    for (QValueVector <kpToolWidgetBase *>::const_iterator it = m_toolWidgets.begin ();
         it != m_toolWidgets.end ();
         it++)
    {
    #if DEBUG_KP_TOOL_TOOL_BAR
        kdDebug () << "\t" << (*it)->name ()
                   << " isShown=" << (*it)->isShown ()
                   << endl;
    #endif
        if ((*it)->isShown ())
            ret++;
    }

    return ret;
}

// public
kpToolWidgetBase *kpToolToolBar::shownToolWidget (int which) const
{
    int uptoVisibleWidget = 0;

    for (QValueVector <kpToolWidgetBase *>::const_iterator it = m_toolWidgets.begin ();
         it != m_toolWidgets.end ();
         it++)
    {
        if ((*it)->isShown ())
        {
            if (which == uptoVisibleWidget)
                return *it;

            uptoVisibleWidget++;
        }
    }

    return 0;
}


// public
bool kpToolToolBar::toolsSingleKeyTriggersEnabled () const
{
    for (QValueVector <kpButtonToolPair>::const_iterator it = m_buttonToolPairs.begin ();
         it != m_buttonToolPairs.end ();
         it++)
    {
        if (!(*it).m_tool->singleKeyTriggersEnabled ())
            return false;
    }

    return true;
}

// public
void kpToolToolBar::enableToolsSingleKeyTriggers (bool enable)
{
#if DEBUG_KP_TOOL_TOOL_BAR
    kdDebug () << "kpToolToolBar::enableToolsSingleKeyTriggers(" << enable << ")" << endl;
#endif

    for (QValueVector <kpButtonToolPair>::const_iterator it = m_buttonToolPairs.begin ();
        it != m_buttonToolPairs.end ();
        it++)
    {
        (*it).m_tool->enableSingleKeyTriggers (enable);
    }
}


// private slot
void kpToolToolBar::slotToolButtonClicked ()
{
    QButton *b = m_buttonGroup->selected ();

#if DEBUG_KP_TOOL_TOOL_BAR
    kdDebug () << "kpToolToolBar::slotToolButtonClicked() button=" << b << endl;
#endif

    kpTool *tool = 0;
    for (QValueVector <kpButtonToolPair>::iterator it = m_buttonToolPairs.begin ();
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


#define CONST_KP_TOOL_SENDER() (dynamic_cast <const kpTool *> (sender ()))

// private slot
void kpToolToolBar::slotToolActionActivated ()
{
    const kpTool *tool = CONST_KP_TOOL_SENDER ();

#if DEBUG_KP_TOOL_TOOL_BAR
    kdDebug () << "kpToolToolBar::slotToolActionActivated() tool="
               << (tool ? tool->name () : "null")
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

// private slot
void kpToolToolBar::slotToolActionToolTipChanged ()
{
    const kpTool *tool = CONST_KP_TOOL_SENDER ();

#if DEBUG_KP_TOOL_TOOL_BAR
    kdDebug () << "kpToolToolBar::slotToolActionToolTipChanged() tool="
               << (tool ? tool->name () : "null")
               << endl;
#endif

    if (!tool)
        return;

    for (QValueVector <kpButtonToolPair>::const_iterator it = m_buttonToolPairs.begin ();
        it != m_buttonToolPairs.end ();
        it++)
    {
        if (tool == (*it).m_tool)
        {
            QToolTip::add ((*it).m_button, tool->toolTip ());
            return;
        }
    }
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

    for (QValueVector <kpButtonToolPair>::iterator it = m_buttonToolPairs.begin ();
         it != m_buttonToolPairs.end ();
         it++)
    {
        addButton ((*it).m_button, o, num);
        num++;
    }

    for (QValueVector <kpToolWidgetBase *>::const_iterator it = m_toolWidgets.begin ();
         it != m_toolWidgets.end ();
         it++)
    {
        if (*it)
        {
            m_baseLayout->addWidget (*it,
                0/*stretch*/,
                o == Qt::Vertical ? Qt::AlignHCenter : Qt::AlignVCenter);
        }
    }

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
