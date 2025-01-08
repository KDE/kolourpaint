
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

#ifndef KP_COLOR_TOOLBAR_H
#define KP_COLOR_TOOLBAR_H

#include <QCloseEvent>
#include <QDockWidget>

#include "imagelib/kpColor.h"

class QBoxLayout;

class kpColorCells;
class kpColorPalette;
class kpColorSimilarityToolBarItem;
class kpDualColorButton;

// COMPAT: Vertical orientation and undocking were broken by the Qt4 port
//         so kpMainWindow::init() keeps this tool bar in a constant position for
//         the time being.  To help make this workaround possible, we use QDockWidget,
//         instead of KToolBar, to prevent XMLGUI from managing the tool
//         bar position.  This also allows us to use QMainWindow::setCorner().
//
//         A nice must-have side-effect is that we are now resizeable, which
//         is good for configurable (and potentially large) color collections.
//         So we should probably keep it as a QDockWidget in the long-term
//         and once we fix orientation and undocking, we should put XMLGUI
//         support back in, somehow (create a "KDockWidget" class?).
class kpColorToolBar : public QDockWidget
{
    Q_OBJECT

public:
    kpColorToolBar(const QString &label, QWidget *parent);

    kpColorCells *colorCells() const;

    kpColor color(int which) const;
    void setColor(int which, const kpColor &color);

    kpColor foregroundColor() const;
    kpColor backgroundColor() const;

    double colorSimilarity() const;
    void setColorSimilarity(double similarity);
    int processedColorSimilarity() const;

    void openColorSimilarityDialog();
    void flashColorSimilarityToolBarItem();

Q_SIGNALS:
    // If you connect to this signal, ignore the following
    // foregroundColorChanged() and backgroundColorChanged() signals
    void colorsSwapped(const kpColor &newForegroundColor, const kpColor &newBackgroundColor);

    void foregroundColorChanged(const kpColor &color);
    void backgroundColorChanged(const kpColor &color);
    void colorSimilarityChanged(double similarity, int processedSimilarity);

public:
    // (only valid in slots connected to foregroundColorChanged())
    kpColor oldForegroundColor() const;
    // (only valid in slots connected to backgroundColorChanged())
    kpColor oldBackgroundColor() const;

    // (only valid in slots connected to colorSimilarityChanged())
    double oldColorSimilarity() const;

    QList<QAction *> customContextMenuActions() const;
    void setCustomContextMenuActions(QList<QAction *> customContextMenuActions);

public Q_SLOTS:
    void setForegroundColor(const kpColor &color);
    void setBackgroundColor(const kpColor &color);

private Q_SLOTS:
    void updateNameOrUrlLabel();

protected:
    // Eat color drops (which are usually accidental drags from one of our
    // child widgets) to prevent them from being pasted as text in the
    // main window (by kpMainWindow::dropEvent()).
    void dragEnterEvent(QDragEnterEvent *e) override;
    void dragMoveEvent(QDragMoveEvent *e) override;

    void closeEvent(QCloseEvent *event) override
    {
        event->ignore();
    }
    void contextMenuEvent(QContextMenuEvent *event) override;

private:
    void adjustToOrientation(Qt::Orientation o);
    bool isLocked() const;
    void setLocked(bool lock);

    QBoxLayout *m_boxLayout;
    QList<QAction *> m_customContextMenuActions;
    kpDualColorButton *m_dualColorButton;
    kpColorPalette *m_colorPalette;
    kpColorSimilarityToolBarItem *m_colorSimilarityToolBarItem;
    bool m_locked = true;
};

#endif // KP_COLOR_TOOLBAR_H
