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

#define NUM_TOOL_ROWS 3
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
#include <SARibbonBar/SARibbonCategory.h>
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

//---------------------------------------------------------------------

kpToolToolBar::kpToolToolBar(QMainWindow *parent, SARibbonCategory *ribbonPageForPannels)
    : QObject(),
      m_previousTool (nullptr), m_currentTool (nullptr)
{
    m_pnTools = new SARibbonPannel(QLatin1String("Tools"), ribbonPageForPannels);
    ribbonPageForPannels->addPannel(m_pnTools);

    for (int i = 0; i < NUM_TOOL_ROWS; i++)
    {
        auto grp = new SARibbonButtonGroupWidget(m_pnTools);
        m_toolsRows.append(grp);

        if (NUM_TOOL_ROWS <= 2)
            m_pnTools->addMediumWidget(grp);
        else
            m_pnTools->addSmallWidget(grp);
    }

    m_pnParams = new SARibbonPannel(QLatin1String(""), ribbonPageForPannels);
    ribbonPageForPannels->addPannel(m_pnParams);

    auto hbox = new QWidget(m_pnParams);
    auto hboxLayout = new QHBoxLayout(hbox);
    m_pnParams->addLargeWidget(hbox);

    m_toolWidgets.append (m_toolWidgetBrush =
        new kpToolWidgetBrush (hbox, QStringLiteral("Brush")));
    m_toolWidgets.append (m_toolWidgetEraserSize =
        new kpToolWidgetEraserSize (hbox, QStringLiteral("Eraser Size")));
    m_toolWidgets.append (m_toolWidgetFillStyle =
        new kpToolWidgetFillStyle (hbox, QStringLiteral("Fill Style")));
    m_toolWidgets.append (m_toolWidgetLineWidth =
        new kpToolWidgetLineWidth (hbox, QStringLiteral("Line Width")));
    m_toolWidgets.append (m_toolWidgetOpaqueOrTransparent =
        new kpToolWidgetOpaqueOrTransparent (hbox, QStringLiteral("Transparency")));
    m_toolWidgets.append (m_toolWidgetSpraycanSize =
        new kpToolWidgetSpraycanSize (hbox, QStringLiteral("Spraycan Size")));

    for (auto *w : m_toolWidgets)
    {
      hboxLayout->addWidget(w);

      connect (w, &kpToolWidgetBase::optionSelected,
               this, &kpToolToolBar::toolWidgetOptionSelected);
    }

    hideAllToolWidgets ();
}

//---------------------------------------------------------------------

kpToolToolBar::~kpToolToolBar()
{
}

//---------------------------------------------------------------------

// public
void kpToolToolBar::registerTool (kpTool *tool, bool dontMakeButton)
{
    int numTools = 0;
    for (auto *row : m_toolsRows)
    {
        if (row->actions().contains(tool->action()))   // Quit if the action already exists in any of the rows.
            return;

        numTools += row->actions().count();
    }
        
    if (!dontMakeButton)
        m_toolsRows[numTools % m_toolsRows.count()]->addAction(tool->action());

    connect (tool, &kpTool::actionActivated, [&, tool]() {
        selectTool (tool, true/*reselect if same tool*/);
    });
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
}

//---------------------------------------------------------------------

void kpToolToolBar::setEnabled(bool enabled)
{
    m_pnTools->setEnabled(enabled);
    m_pnParams->setEnabled(enabled);
}

bool kpToolToolBar::isEnabled()
{
    return (m_pnTools->isEnabled() && m_pnParams->isEnabled());
}

#include "moc_kpToolToolBar.cpp"
