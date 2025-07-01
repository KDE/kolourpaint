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


#include "widgets/toolbars/kpToolToolBar.h"

#include <QBoxLayout>
#include <QGridLayout>
#include <QButtonGroup>
#include <QKeyEvent>
#include <QToolButton>
#include <QStyleOption>
#include <QStylePainter>
#include <QWidgetAction>
#include <QHBoxLayout>

#include <SARibbonBar/SARibbonPannel.h>
#include <SARibbonBar/SARibbonButtonGroupWidget.h>

#include <KToggleAction>

#include "kpLogCategories.h"

#include "kpDefs.h"
#include "tools/kpTool.h"
#include "widgets/toolbars/options/kpToolWidgetBrush.h"
#include "widgets/toolbars/options/kpToolWidgetEraserSize.h"
#include "widgets/toolbars/options/kpToolWidgetFillStyle.h"
#include "widgets/toolbars/options/kpToolWidgetLineWidth.h"
#include "widgets/toolbars/options/kpToolWidgetOpaqueOrTransparent.h"
#include "widgets/toolbars/options/kpToolWidgetSpraycanSize.h"

#define NUM_TOOLS_ROWS 3

//---------------------------------------------------------------------

kpToolToolBar::kpToolToolBar(const QString &name, int colsOrRows, QMainWindow *parent)
    : KToolBar(name, parent, Qt::LeftToolBarArea),
      m_vertCols (colsOrRows),
      m_baseWidget (nullptr),
      m_baseLayout (nullptr),
      m_toolLayout (nullptr),
      m_previousTool (nullptr), m_currentTool (nullptr)
{
    auto pn = new SARibbonPannel(QLatin1String("Tool Options"), this);
    addWidget(pn);

    for (int i = 0; i < NUM_TOOLS_ROWS; i++)
    {
        auto grp = new SARibbonButtonGroupWidget(this);
        m_toolsRows.append(grp);

        if (NUM_TOOLS_ROWS <= 2)
            pn->addMediumWidget(grp);
        else
            pn->addSmallWidget(grp);
    }

    m_baseWidget = (SARibbonPannel *) new QWidget(this);//new SARibbonPannel(QLatin1String("Tool Options"), this);

    auto hbox = new QWidget(m_baseWidget);
    auto hboxLayout = new QHBoxLayout(hbox);
    pn->addLargeWidget(hbox);

    m_toolWidgets.append (m_toolWidgetBrush =
        new kpToolWidgetBrush (hbox, QStringLiteral("Tool Widget Brush")));
    m_toolWidgets.append (m_toolWidgetEraserSize =
        new kpToolWidgetEraserSize (hbox, QStringLiteral("Tool Widget Eraser Size")));
    m_toolWidgets.append (m_toolWidgetFillStyle =
        new kpToolWidgetFillStyle (hbox, QStringLiteral("Tool Widget Fill Style")));
    m_toolWidgets.append (m_toolWidgetLineWidth =
        new kpToolWidgetLineWidth (hbox, QStringLiteral("Tool Widget Line Width")));
    m_toolWidgets.append (m_toolWidgetOpaqueOrTransparent =
        new kpToolWidgetOpaqueOrTransparent (hbox, QStringLiteral("Tool Widget Opaque/Transparent")));
    m_toolWidgets.append (m_toolWidgetSpraycanSize =
        new kpToolWidgetSpraycanSize (hbox, QStringLiteral("Tool Widget Spraycan Size")));

    for (auto *w : m_toolWidgets)
    {
      hboxLayout->addWidget(w);

      connect (w, &kpToolWidgetBase::optionSelected,
               this, &kpToolToolBar::toolWidgetOptionSelected);
    }

    adjustToOrientation(orientation());
    connect (this, &kpToolToolBar::orientationChanged,
             this, &kpToolToolBar::adjustToOrientation);

    hideAllToolWidgets ();

    addWidget(m_baseWidget);

    connect (this, &kpToolToolBar::iconSizeChanged,
             this, &kpToolToolBar::slotIconSizeChanged);

    connect (this, &kpToolToolBar::toolButtonStyleChanged,
             this, &kpToolToolBar::slotToolButtonStyleChanged);
}

//---------------------------------------------------------------------

kpToolToolBar::~kpToolToolBar()
{
    while ( !m_toolButtons.isEmpty() ) {
        delete m_toolButtons.takeFirst();
    }
}

//---------------------------------------------------------------------

// public
void kpToolToolBar::registerTool (kpTool *tool)
{
    int numTools = 0;
    for (auto *row : m_toolsRows)
    {
        if (row->actions().contains(tool->action()))   // Quit if the action already exists in any of the rows.
            return;

        numTools += row->actions().count();
    }

    m_toolsRows[numTools % m_toolsRows.count()]->addAction(tool->action());

    connect (tool, &kpTool::actionActivated, [&, tool]() {
        selectTool (tool, true/*reselect if same tool*/);
    });

    adjustSizeConstraint();
}

//---------------------------------------------------------------------
// public

void kpToolToolBar::unregisterTool(kpTool *tool)
{
    qDebug() << "kpToolToolBar::unregisterTool() not implemented. Code shouldn't be calling this.";
}

//---------------------------------------------------------------------
// public

kpTool *kpToolToolBar::tool () const
{
    return m_currentTool;
}

//---------------------------------------------------------------------

// public
void kpToolToolBar::selectTool (kpTool *tool, bool reselectIfSameTool)
{
#if DEBUG_KP_TOOL_TOOL_BAR
    qCDebug(kpLogWidgets) << "kpToolToolBar::selectTool (tool=" << tool
               << ") currentTool=" << m_currentTool
               << endl;
#endif

    if (!reselectIfSameTool && tool == m_currentTool) {
        return;
    }

    if (tool)
    {
      tool->action()->setChecked(true);
      slotToolButtonClicked(tool);
    }
    else
    {
        m_currentTool->action()->setChecked(false);
        slotToolButtonClicked (tool);
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
    for (auto *w : m_toolWidgets) {
      w->hide();
    }
}

//---------------------------------------------------------------------

// public
kpToolWidgetBase *kpToolToolBar::shownToolWidget (int which) const
{
    int uptoVisibleWidget = 0;

    for(auto *w : m_toolWidgets)
    {
        if ( !w->isHidden() )
        {
            if (which == uptoVisibleWidget) {
                return w;
            }

            uptoVisibleWidget++;
        }
    }

    return nullptr;
}

//---------------------------------------------------------------------

// private slot
void kpToolToolBar::slotToolButtonClicked (kpTool *tool)
{
#if DEBUG_KP_TOOL_TOOL_BAR
    qCDebug(kpLogWidgets) << "kpToolToolBar::slotToolButtonClicked() button=" << b;

    qCDebug(kpLogWidgets) << "\ttool=" << tool
               << " currentTool=" << m_currentTool
               << endl;
#endif

    if (tool == m_currentTool)
    {
        if (m_currentTool) {
            m_currentTool->reselect ();
        }

        return;
    }

    if (m_currentTool) {
        m_currentTool->endInternal ();
    }

    m_previousTool = m_currentTool;
    m_currentTool = tool;

    if (m_currentTool)
    {
        KToggleAction *action = m_currentTool->action ();
        if (action)
        {
            action->setChecked (true);
        }

        m_currentTool->beginInternal ();
    }

    Q_EMIT sigToolSelected (m_currentTool);
    m_baseLayout->activate();
    adjustSizeConstraint();
}

//---------------------------------------------------------------------

// public
void kpToolToolBar::adjustToOrientation(Qt::Orientation o)
{
#if DEBUG_KP_TOOL_TOOL_BAR
    qCDebug(kpLogWidgets) << "kpToolToolBar::adjustToOrientation("
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
    m_baseLayout->setContentsMargins(0, 0, 0, 0);

    m_toolLayout = new QGridLayout();
    m_toolLayout->setContentsMargins(0, 0, 0, 0);

    // (ownership is transferred to m_baseLayout)
    m_baseLayout->addItem (m_toolLayout);

    auto num = 0;

    for (auto *b : m_toolButtons)
    {
      addButton(b, o, num);
      num++;
    }

    adjustSizeConstraint();
}


bool kpToolToolBar::event(QEvent *ev)
{
    if (ev->type() == QEvent::LayoutRequest) {
        adjustSizeConstraint();
    }
    return KToolBar::event(ev);
}

void kpToolToolBar::paintEvent(class QPaintEvent *)
{
    QStyleOption opt;
    opt.initFrom(this);
    if (opt.rect.height() <= contentsMargins().top() + contentsMargins().bottom() || opt.rect.width() <= contentsMargins().left() + contentsMargins().right()) {
        return;
    }

    QStylePainter painter(this);

    opt.rect.setX(opt.rect.width() - style()->pixelMetric(QStyle::PM_SplitterWidth));
    opt.rect.setWidth(style()->pixelMetric(QStyle::PM_SplitterWidth));
    opt.state = QStyle::State_Horizontal;

    painter.drawPrimitive(QStyle::PE_IndicatorToolBarSeparator, opt);
}

//---------------------------------------------------------------------
// this makes the size handled correctly during dragging/undocking the toolbar

void kpToolToolBar::adjustSizeConstraint()
{
    // remove constraints
    setFixedSize(QSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX));

    setFixedWidth(600);
}

//---------------------------------------------------------------------

// private
void kpToolToolBar::addButton(QAbstractButton *button, Qt::Orientation o, int num)
{
    if (o == Qt::Vertical) {
        m_toolLayout->addWidget (button, num / m_vertCols, num % m_vertCols);
    }
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
    for (auto *b : m_toolButtons) {
        b->setIconSize(size);
    }

    m_baseLayout->activate();
    adjustSizeConstraint();
}

//---------------------------------------------------------------------

void kpToolToolBar::slotToolButtonStyleChanged(Qt::ToolButtonStyle style)
{
    for (auto *b : m_toolButtons) {
        b->setToolButtonStyle(style);
    }

    m_baseLayout->activate();
    adjustSizeConstraint();
}

//---------------------------------------------------------------------

#include "moc_kpToolToolBar.cpp"
