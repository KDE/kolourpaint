
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef kpColorPalette_H
#define kpColorPalette_H

#include <QWidget>

class kpColor;
class kpColorCells;

class kpColorPalette : public QWidget
{
    Q_OBJECT

public:
    explicit kpColorPalette(QWidget *parent, Qt::Orientation o = Qt::Horizontal);
    ~kpColorPalette() override;

    Qt::Orientation orientation() const;
    void setOrientation(Qt::Orientation o);

    kpColorCells *colorCells() const;

Q_SIGNALS:
    void foregroundColorChanged(const kpColor &color);
    void backgroundColorChanged(const kpColor &color);

private:
    struct kpColorPalettePrivate *const d;
};

#endif // kpColorPalette_H
