
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef kpColorSimilarityFrame_H
#define kpColorSimilarityFrame_H

#include "kpColorSimilarityHolder.h"

#include <QWidget>

class kpColorSimilarityFrame : public QWidget, public kpColorSimilarityHolder
{
public:
    explicit kpColorSimilarityFrame(QWidget *parent);

    void setColorSimilarity(double similarity) override;

    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *e) override;
};

#endif // kpColorSimilarityFrame_H
