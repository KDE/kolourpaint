
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


#ifndef kpColorSimilarityToolBarItem_H
#define kpColorSimilarityToolBarItem_H


#include "widgets/colorSimilarity/kpColorSimilarityHolder.h"

#include <QToolButton>


class QTimer;


class kpColorSimilarityToolBarItem : public QToolButton,
        public kpColorSimilarityHolder
{
Q_OBJECT

public:
    // (reads the color similarity config setting)
    kpColorSimilarityToolBarItem (QWidget *parent);

    int processedColorSimilarity () const;

private:
    // <writeConfig> specifies whether to write the color similarity config
    // setting.
    void setColorSimilarityInternal (double similarity, bool writeConfig);
public:
    void setColorSimilarity (double similarity) override;

signals:
    void colorSimilarityChanged (double similarity, int processedSimilarity);

public:
    // (only valid in slots connected to colorSimilarityChanged());
    double oldColorSimilarity () const;


public slots:
    // Open configuration dialog for color similarity.
    void openDialog ();

private slots:
    void slotFlashTimerTimeout ();

public:
    // Animates the cube, so that the user is aware of its existence.
    // Call this whenever a tool or command uses color similarity.
    void flash ();

public:
    // Whether to ignore calls to flash().
    // You can nest blocks of suppressFlash()/unsuppressFlash().
    bool isSuppressingFlash () const;
    void suppressFlash ();
    void unsupressFlash ();


private:
    void updateToolTip ();
    void updateIcon ();

    void resizeEvent (QResizeEvent *e) override;


private:
    double m_oldColorSimilarity;
    int m_processedColorSimilarity;

    QTimer *m_flashTimer;
    int m_flashHighlight;
    int m_suppressingFlashCounter;
};


#endif  // kpColorSimilarityToolBarItem_H
