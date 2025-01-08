
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef kpColorSimilarityToolBarItem_H
#define kpColorSimilarityToolBarItem_H

#include "widgets/colorSimilarity/kpColorSimilarityHolder.h"

#include <QToolButton>

class QTimer;

class kpColorSimilarityToolBarItem : public QToolButton, public kpColorSimilarityHolder
{
    Q_OBJECT

public:
    // (reads the color similarity config setting)
    explicit kpColorSimilarityToolBarItem(QWidget *parent);

    int processedColorSimilarity() const;

private:
    // <writeConfig> specifies whether to write the color similarity config
    // setting.
    void setColorSimilarityInternal(double similarity, bool writeConfig);

public:
    void setColorSimilarity(double similarity) override;

Q_SIGNALS:
    void colorSimilarityChanged(double similarity, int processedSimilarity);

public:
    // (only valid in slots connected to colorSimilarityChanged());
    double oldColorSimilarity() const;

public Q_SLOTS:
    // Open configuration dialog for color similarity.
    void openDialog();

private Q_SLOTS:
    void slotFlashTimerTimeout();

public:
    // Animates the cube, so that the user is aware of its existence.
    // Call this whenever a tool or command uses color similarity.
    void flash();

public:
    // Whether to ignore calls to flash().
    // You can nest blocks of suppressFlash()/unsuppressFlash().
    bool isSuppressingFlash() const;
    void suppressFlash();
    void unsupressFlash();

private:
    void updateToolTip();
    void updateIcon();

    void resizeEvent(QResizeEvent *e) override;

private:
    double m_oldColorSimilarity;
    int m_processedColorSimilarity;

    QTimer *m_flashTimer;
    int m_flashHighlight;
    int m_suppressingFlashCounter;
};

#endif // kpColorSimilarityToolBarItem_H
