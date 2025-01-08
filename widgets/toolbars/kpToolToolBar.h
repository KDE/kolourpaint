
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>
   SPDX-FileCopyrightText: 2011 Martin Koller <kollix@aon.at>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef KP_TOOL_TOOL_BAR_H
#define KP_TOOL_TOOL_BAR_H

#include <QList>

#include <KToolBar>

class QAbstractButton;
class QBoxLayout;
class QButtonGroup;
class QGridLayout;
class QWidget;

class kpTool;
class kpToolButton;

class kpToolWidgetBase;
class kpToolWidgetBrush;
class kpToolWidgetEraserSize;
class kpToolWidgetFillStyle;
class kpToolWidgetLineWidth;
class kpToolWidgetOpaqueOrTransparent;
class kpToolWidgetSpraycanSize;

class kpToolToolBar : public KToolBar
{
    Q_OBJECT

public:
    kpToolToolBar(const QString &name, int colsOrRows, QMainWindow *parent);
    ~kpToolToolBar() override;

    void registerTool(kpTool *tool);
    void unregisterTool(kpTool *tool);

    kpTool *tool() const;
    void selectTool(const kpTool *tool, bool reselectIfSameTool = false);

    kpTool *previousTool() const;
    void selectPreviousTool();

    void hideAllToolWidgets();
    // could this be cleaner (the tools have to access them individually somehow)?
    kpToolWidgetBrush *toolWidgetBrush() const
    {
        return m_toolWidgetBrush;
    }
    kpToolWidgetEraserSize *toolWidgetEraserSize() const
    {
        return m_toolWidgetEraserSize;
    }
    kpToolWidgetFillStyle *toolWidgetFillStyle() const
    {
        return m_toolWidgetFillStyle;
    }
    kpToolWidgetLineWidth *toolWidgetLineWidth() const
    {
        return m_toolWidgetLineWidth;
    }
    kpToolWidgetOpaqueOrTransparent *toolWidgetOpaqueOrTransparent() const
    {
        return m_toolWidgetOpaqueOrTransparent;
    }
    kpToolWidgetSpraycanSize *toolWidgetSpraycanSize() const
    {
        return m_toolWidgetSpraycanSize;
    }

    kpToolWidgetBase *shownToolWidget(int which) const;

protected:
    bool event(QEvent *ev) override;
    void paintEvent(QPaintEvent *) override;

Q_SIGNALS:
    void sigToolSelected(kpTool *tool); // tool may be 0
    void toolWidgetOptionSelected();

private Q_SLOTS:
    void slotToolButtonClicked();

    void slotToolActionActivated();

    void adjustToOrientation(Qt::Orientation o);
    void slotIconSizeChanged(const QSize &);
    void slotToolButtonStyleChanged(Qt::ToolButtonStyle style);

private:
    void addButton(QAbstractButton *button, Qt::Orientation o, int num);
    void adjustSizeConstraint();

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

    QList<kpToolWidgetBase *> m_toolWidgets;

    QList<kpToolButton *> m_toolButtons;

    kpTool *m_previousTool, *m_currentTool;
};

#endif // KP_TOOL_TOOL_BAR_H
