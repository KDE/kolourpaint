
/*
   Copyright (c) 2003-2007 Clarence Dang <dang@kde.org>
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


#ifndef KP_TOOL_TOOL_BAR_H
#define KP_TOOL_TOOL_BAR_H


#include <qlist.h>
#include <QDockWidget>


class QAbstractButton;
class QBoxLayout;
class QButtonGroup;
class QGridLayout;
class QWidget;

class kpTool;

class kpToolWidgetBase;
class kpToolWidgetBrush;
class kpToolWidgetEraserSize;
class kpToolWidgetFillStyle;
class kpToolWidgetLineWidth;
class kpToolWidgetOpaqueOrTransparent;
class kpToolWidgetSpraycanSize;


// COMPAT: Horizontal orientation and undocking were broken by the Qt4 port
//         so kpMainWindow::init() keeps this tool bar in a constant position for
//         the time being.  To help make this workaround possible, we use QDockWidget,
//         instead of KToolBar, to prevent XMLGUI from managing the tool
//         bar position.  This also allows us to use QMainWindow::setCorner(),
//         so we should probably keep it as a QDockWidget in the long-term
//         and once we fix orientation and undocking, we should put XMLGUI
//         support back in, somehow (create a "KDockWidget" class?).
class kpToolToolBar : public QDockWidget
{
Q_OBJECT

public:
    kpToolToolBar (const QString &label, int colsOrRows, QWidget *parent);
    virtual ~kpToolToolBar ();

private:
    int defaultIconSize ();
public:
    void registerTool (kpTool *tool);
    void unregisterTool (kpTool *tool);
    void unregisterAllTools ();

    kpTool *tool () const;
    void selectTool (const kpTool *tool, bool reselectIfSameTool = false);

    kpTool *previousTool () const;
    void selectPreviousTool ();

    void hideAllToolWidgets ();
    // could this be cleaner (the tools have to access them individually somehow)?
    kpToolWidgetBrush *toolWidgetBrush () const { return m_toolWidgetBrush; }
    kpToolWidgetEraserSize *toolWidgetEraserSize () const { return m_toolWidgetEraserSize; }
    kpToolWidgetFillStyle *toolWidgetFillStyle () const { return m_toolWidgetFillStyle; }
    kpToolWidgetLineWidth *toolWidgetLineWidth () const { return m_toolWidgetLineWidth; }
    kpToolWidgetOpaqueOrTransparent *toolWidgetOpaqueOrTransparent () const { return m_toolWidgetOpaqueOrTransparent; }
    kpToolWidgetSpraycanSize *toolWidgetSpraycanSize () const { return m_toolWidgetSpraycanSize; }

public:
    int numShownToolWidgets () const;
    kpToolWidgetBase *shownToolWidget (int which) const;

signals:
    void sigToolSelected (kpTool *tool);  // tool may be 0
    void toolWidgetOptionSelected ();

private slots:
    void slotToolButtonClicked ();

    void slotToolActionActivated ();
    void slotToolActionToolTipChanged ();

private:
    void adjustToOrientation (Qt::Orientation o);

private:
    void addButton (QAbstractButton *button, Qt::Orientation o, int num);

    int m_vertCols;

    QButtonGroup *m_buttonGroup;
    QWidget *m_baseWidget;
    QBoxLayout *m_baseLayout;
    QGridLayout *m_toolLayout;

    kpToolWidgetBrush *m_toolWidgetBrush;
    kpToolWidgetEraserSize *m_toolWidgetEraserSize;
    kpToolWidgetFillStyle *m_toolWidgetFillStyle;
    kpToolWidgetLineWidth *m_toolWidgetLineWidth;
    kpToolWidgetOpaqueOrTransparent *m_toolWidgetOpaqueOrTransparent;
    kpToolWidgetSpraycanSize *m_toolWidgetSpraycanSize;

    QList <kpToolWidgetBase *> m_toolWidgets;

private:
    struct kpButtonToolPair
    {
        kpButtonToolPair (QAbstractButton *button, kpTool *tool)
            : m_button (button), m_tool (tool)
        {
        }

        kpButtonToolPair ()
            : m_button (0), m_tool (0)
        {
        }

        QAbstractButton *m_button;
        kpTool *m_tool;
    };

    QList <kpButtonToolPair> m_buttonToolPairs;

    kpTool *m_previousTool, *m_currentTool;

    int m_defaultIconSize;

private:
    // There is no need to maintain binary compatibility at this stage.
    // The d-pointer is just so that you can experiment without recompiling
    // the kitchen sink.
    class kpToolToolBarPrivate *d;
};


#endif  // KP_TOOL_TOOL_BAR_H
