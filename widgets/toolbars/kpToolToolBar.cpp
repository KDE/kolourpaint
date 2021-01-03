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

#include "kpLogCategories.h"

#include "kpDefs.h"
#include "tools/kpTool.h"
#include "tools/kpToolAction.h"
#include "widgets/toolbars/options/kpToolWidgetBrush.h"
#include "widgets/toolbars/options/kpToolWidgetEraserSize.h"
#include "widgets/toolbars/options/kpToolWidgetFillStyle.h"
#include "widgets/toolbars/options/kpToolWidgetLineWidth.h"
#include "widgets/toolbars/options/kpToolWidgetOpaqueOrTransparent.h"
#include "widgets/toolbars/options/kpToolWidgetSpraycanSize.h"

//---------------------------------------------------------------------

class kpToolButton : public QToolButton
{
public:
    kpToolButton (kpTool *tool, QWidget *parent)
        : QToolButton (parent),
          m_tool (tool)
    {
    }

    kpTool *tool() const { return m_tool; }

protected:
    void mouseDoubleClickEvent(QMouseEvent *e) override
    {
        if (e->button () == Qt::LeftButton && m_tool) {
            m_tool->globalDraw ();
        }
    }

    kpTool *m_tool;
};

//---------------------------------------------------------------------

kpToolToolBar::kpToolToolBar(const QString &name, int colsOrRows, QMainWindow *parent)
    : KToolBar(name, parent, Qt::LeftToolBarArea),
      m_vertCols (colsOrRows),
      m_buttonGroup (nullptr),
      m_baseWidget (nullptr),
      m_baseLayout (nullptr),
      m_toolLayout (nullptr),
      m_previousTool (nullptr), m_currentTool (nullptr)
{
    m_baseWidget = new QWidget(this);

    m_toolWidgets.append (m_toolWidgetBrush =
        new kpToolWidgetBrush (m_baseWidget, QStringLiteral("Tool Widget Brush")));
    m_toolWidgets.append (m_toolWidgetEraserSize =
        new kpToolWidgetEraserSize (m_baseWidget, QStringLiteral("Tool Widget Eraser Size")));
    m_toolWidgets.append (m_toolWidgetFillStyle =
        new kpToolWidgetFillStyle (m_baseWidget, QStringLiteral("Tool Widget Fill Style")));
    m_toolWidgets.append (m_toolWidgetLineWidth =
        new kpToolWidgetLineWidth (m_baseWidget, QStringLiteral("Tool Widget Line Width")));
    m_toolWidgets.append (m_toolWidgetOpaqueOrTransparent =
        new kpToolWidgetOpaqueOrTransparent (m_baseWidget, QStringLiteral("Tool Widget Opaque/Transparent")));
    m_toolWidgets.append (m_toolWidgetSpraycanSize =
        new kpToolWidgetSpraycanSize (m_baseWidget, QStringLiteral("Tool Widget Spraycan Size")));

    for (auto *w : m_toolWidgets)
    {
      connect (w, &kpToolWidgetBase::optionSelected,
               this, &kpToolToolBar::toolWidgetOptionSelected);
    }

    adjustToOrientation(orientation());
    connect (this, &kpToolToolBar::orientationChanged,
             this, &kpToolToolBar::adjustToOrientation);

    m_buttonGroup = new QButtonGroup (this);
    connect (m_buttonGroup, &QButtonGroup::idClicked,
             this, &kpToolToolBar::slotToolButtonClicked);

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
    for (const auto *b : m_toolButtons)
    {
        if ( b->tool() == tool ) {  // already given
            return;
        }
    }

    auto *b = new kpToolButton(tool, m_baseWidget);

    b->setToolButtonStyle(toolButtonStyle());
    b->setIconSize(iconSize());
    b->setAutoRaise(true);

    // tell layout to make all with equal width (much better when text-below-icon)
    b->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    b->setDefaultAction(tool->action());

    m_buttonGroup->addButton(b);
    addButton(b, orientation(), m_toolButtons.count());

    m_toolButtons.append(b);

    connect (tool, &kpTool::actionActivated,
             this, &kpToolToolBar::slotToolActionActivated);

    adjustSizeConstraint();
}

//---------------------------------------------------------------------
// public

void kpToolToolBar::unregisterTool(kpTool *tool)
{
    for (int i = 0; i < m_toolButtons.count(); i++)
    {
      if ( m_toolButtons[i]->tool() == tool )
      {
        delete m_toolButtons.takeAt(i);
        disconnect (tool, &kpTool::actionActivated,
                 this, &kpToolToolBar::slotToolActionActivated);
        return;
      }
    }
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
      slotToolButtonClicked();
    }
    else
    {
        QAbstractButton *b = m_buttonGroup->checkedButton();
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
    for (auto *w : m_toolWidgets) {
      w->hide ();
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
void kpToolToolBar::slotToolButtonClicked ()
{
    QAbstractButton *b = m_buttonGroup->checkedButton();

#if DEBUG_KP_TOOL_TOOL_BAR
    qCDebug(kpLogWidgets) << "kpToolToolBar::slotToolButtonClicked() button=" << b;
#endif

    kpTool *tool = nullptr;
    for (const auto *button : m_toolButtons)
    {
      if ( button == b )
      {
        tool = button->tool();
        break;
      }
    }

#if DEBUG_KP_TOOL_TOOL_BAR
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
        kpToolAction *action = m_currentTool->action ();
        if (action)
        {
            action->setChecked (true);
        }

        m_currentTool->beginInternal ();
    }

    emit sigToolSelected (m_currentTool);
    m_baseLayout->activate();
    adjustSizeConstraint();
}

//---------------------------------------------------------------------

// private slot
void kpToolToolBar::slotToolActionActivated ()
{
    const auto *tool = dynamic_cast<const kpTool *>(sender());

#if DEBUG_KP_TOOL_TOOL_BAR
    qCDebug(kpLogWidgets) << "kpToolToolBar::slotToolActionActivated() tool="
               << (tool ? tool->objectName () : "null")
               << endl;
#endif

    selectTool (tool, true/*reselect if same tool*/);
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

    for (auto *w : m_toolWidgets)
    {
      m_baseLayout->addWidget(w,
          0/*stretch*/,
          o == Qt::Vertical ? Qt::AlignHCenter : Qt::AlignVCenter);
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
    {
      setFixedWidth(m_baseLayout->sizeHint().width() +
                    layout()->contentsMargins().left() +
                    layout()->contentsMargins().right());
    }
    else
    {
      setFixedHeight(m_baseLayout->sizeHint().height() +
                     layout()->contentsMargins().top() +
                     layout()->contentsMargins().bottom());
    }
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

