
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef KP_TOOL_WIDGET_BASE_H
#define KP_TOOL_WIDGET_BASE_H

#include <QFrame>
#include <QList>
#include <QPair>
#include <QPixmap>
#include <QRect>
#include <QWidget>

class QMouseEvent;

// TODO: This is a crazy and overcomplicated class that invents its own (buggy)
//       layout management.  It should be simplified or removed.
class kpToolWidgetBase : public QFrame
{
    Q_OBJECT

public:
    // (must provide a <name> for config to work)
    kpToolWidgetBase(QWidget *parent, const QString &name);
    ~kpToolWidgetBase() override;

public:
    void addOption(const QPixmap &pixmap, const QString &toolTip = QString());
    void startNewOptionRow();

    // Call this at the end of your constructor.
    // If the default row & col could not be read from the config,
    // <fallBackRow> & <fallBackCol> are passed to setSelected().
    void finishConstruction(int fallBackRow, int fallBackCol);

private:
    QList<int> spreadOutElements(const QList<int> &sizes, int maxSize);

public: // (only have to use these if you don't use finishConstruction())
    // (rereads from config file)
    QPair<int, int> defaultSelectedRowAndCol() const;
    int defaultSelectedRow() const;
    int defaultSelectedCol() const;

    void saveSelectedAsDefault() const;

    void relayoutOptions();

public:
    int selectedRow() const;
    int selectedCol() const;

    int selected() const;

    bool hasPreviousOption(int *row = nullptr, int *col = nullptr) const;
    bool hasNextOption(int *row = nullptr, int *col = nullptr) const;

public Q_SLOTS:
    // (returns whether <row> and <col> were in range)
    virtual bool setSelected(int row, int col, bool saveAsDefault);
    bool setSelected(int row, int col);

    bool selectPreviousOption();
    bool selectNextOption();

Q_SIGNALS:
    void optionSelected(int row, int col);

protected:
    bool event(QEvent *e) override;

    void mousePressEvent(QMouseEvent *e) override;
    void paintEvent(QPaintEvent *e) override;

    QWidget *m_baseWidget;

    QList<QList<QPixmap>> m_pixmaps;
    QList<QList<QString>> m_toolTips;

    QList<QList<QRect>> m_pixmapRects;

    int m_selectedRow, m_selectedCol;
};

#endif // KP_TOOL_WIDGET_BASE_H
