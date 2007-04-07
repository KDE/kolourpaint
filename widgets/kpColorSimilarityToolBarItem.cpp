
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


#define DEBUG_KP_COLOR_SIMILARITY_TOOL_BAR_ITEM 0


#include <kpColorSimilarityToolBarItem.h>

#include <kpColor.h>
#include <kpColorSimilarityDialog.h>
#include <kpMainWindow.h>


kpColorSimilarityToolBarItem::kpColorSimilarityToolBarItem (kpMainWindow *mainWindow,
                                                            QWidget *parent)
    : kpColorSimilarityCube (kpColorSimilarityCube::Depressed |
                             kpColorSimilarityCube::DoubleClickInstructions,
                             mainWindow, parent),
      m_mainWindow (mainWindow),
      m_processedColorSimilarity (kpColor::Exact)
{
    setColorSimilarity (mainWindow->configColorSimilarity ());
}

kpColorSimilarityToolBarItem::~kpColorSimilarityToolBarItem ()
{
}


// public
int kpColorSimilarityToolBarItem::processedColorSimilarity () const
{
    return m_processedColorSimilarity;
}


// public slot
void kpColorSimilarityToolBarItem::setColorSimilarity (double similarity)
{
    m_oldColorSimilarity = colorSimilarity ();

    kpColorSimilarityCube::setColorSimilarity (similarity);
    if (similarity > 0)
        this->setToolTip( i18n ("Color similarity: %1%", qRound (similarity * 100)));
    else
        this->setToolTip( i18n ("Color similarity: Exact"));

    m_processedColorSimilarity = kpColor::processSimilarity (colorSimilarity ());

    m_mainWindow->configSetColorSimilarity (colorSimilarity ());

    emit colorSimilarityChanged (colorSimilarity (), m_processedColorSimilarity);
}

// public
double kpColorSimilarityToolBarItem::oldColorSimilarity () const
{
    return m_oldColorSimilarity;
}


// private virtual [base QWidget]
void kpColorSimilarityToolBarItem::mouseDoubleClickEvent (QMouseEvent * /*e*/)
{
    kpColorSimilarityDialog dialog (m_mainWindow, this);
    dialog.setColorSimilarity (colorSimilarity ());
    if (dialog.exec ())
    {
        setColorSimilarity (dialog.colorSimilarity ());
    }
}


#include <kpColorSimilarityToolBarItem.moc>
