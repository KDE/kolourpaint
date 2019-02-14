
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


#ifndef KP_TOOL_WIDGET_BASE_H
#define KP_TOOL_WIDGET_BASE_H


#include <QFrame>
#include <QPair>
#include <QPixmap>
#include <QRect>
#include <QList>
#include <QWidget>


class QMouseEvent;


// TODO: This is a crazy and overcomplicated class that invents its own (buggy)
//       layout management.  It should be simplified or removed.
class kpToolWidgetBase : public QFrame
{
Q_OBJECT

public:
    // (must provide a <name> for config to work)
    kpToolWidgetBase (QWidget *parent, const QString &name);
    ~kpToolWidgetBase () override;

public:
    void addOption (const QPixmap &pixmap, const QString &toolTip = QString());
    void startNewOptionRow ();

    // Call this at the end of your constructor.
    // If the default row & col could not be read from the config,
    // <fallBackRow> & <fallBackCol> are passed to setSelected().
    void finishConstruction (int fallBackRow, int fallBackCol);

private:
    QList <int> spreadOutElements (const QList <int> &sizes, int maxSize);

public:  // (only have to use these if you don't use finishConstruction())
    // (rereads from config file)
    QPair <int, int> defaultSelectedRowAndCol () const;
    int defaultSelectedRow () const;
    int defaultSelectedCol () const;

    void saveSelectedAsDefault () const;

    void relayoutOptions ();

public:
    int selectedRow () const;
    int selectedCol () const;

    int selected () const;

    bool hasPreviousOption (int *row = nullptr, int *col = nullptr) const;
    bool hasNextOption (int *row = nullptr, int *col = nullptr) const;

public slots:
    // (returns whether <row> and <col> were in range)
    virtual bool setSelected (int row, int col, bool saveAsDefault);
    bool setSelected (int row, int col);

    bool selectPreviousOption ();
    bool selectNextOption ();

signals:
    void optionSelected (int row, int col);

protected:
    bool event (QEvent *e) override;
 
    void mousePressEvent (QMouseEvent *e) override;
    void paintEvent (QPaintEvent *e) override;

    QWidget *m_baseWidget;

    QList < QList <QPixmap> > m_pixmaps;
    QList < QList <QString> > m_toolTips;

    QList < QList <QRect> > m_pixmapRects;

    int m_selectedRow, m_selectedCol;
};


#endif  // KP_TOOL_WIDGET_BASE_H
