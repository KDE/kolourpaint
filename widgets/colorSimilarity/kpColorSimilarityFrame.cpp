
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#define DEBUG_KP_COLOR_SIMILARITY_CUBE 0

#include "kpColorSimilarityFrame.h"

#include "kpColorSimilarityCubeRenderer.h"

#include "kpLogCategories.h"

//---------------------------------------------------------------------

kpColorSimilarityFrame::kpColorSimilarityFrame(QWidget *parent)
    : QWidget(parent)
{
    setWhatsThis(WhatsThis());
}

//---------------------------------------------------------------------

// public virtual [base kpColorSimilarityHolder]
void kpColorSimilarityFrame::setColorSimilarity(double similarity)
{
    kpColorSimilarityHolder::setColorSimilarity(similarity);

    repaint();
}

//---------------------------------------------------------------------

// protected virtual [base QWidget]
QSize kpColorSimilarityFrame::sizeHint() const
{
    return {52, 52};
}

//---------------------------------------------------------------------

// protected virtual [base QWidget]
void kpColorSimilarityFrame::paintEvent(QPaintEvent *)
{
    int cubeRectSize = qMin(width() * 6 / 8, height() * 6 / 8);
    int x = (width() - cubeRectSize) / 2;
    int y = (height() - cubeRectSize) / 2;

    kpColorSimilarityCubeRenderer::Paint(this, x, y, cubeRectSize, colorSimilarity());
}

//---------------------------------------------------------------------
