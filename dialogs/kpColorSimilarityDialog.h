
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


#ifndef KP_COLOR_SIMILARITY_DIALOG_H
#define KP_COLOR_SIMILARITY_DIALOG_H


#include <QDialog>


class QLabel;

class kpIntNumInput;

class kpColorSimilarityFrame;


// LOTODO: Why doesn't this dialog automatically enforce a minimum size
//         based on layout magic, like Image -> Resize / Scale?
class kpColorSimilarityDialog : public QDialog
{
Q_OBJECT

public:
    kpColorSimilarityDialog (QWidget *parent);
    ~kpColorSimilarityDialog () override;

    double colorSimilarity () const;
    void setColorSimilarity (double similarity);

private slots:
    void slotColorSimilarityValueChanged ();

    void slotWhatIsLabelClicked ();

private:
    kpColorSimilarityFrame *m_colorSimilarityFrame;
    kpIntNumInput *m_colorSimilarityInput;
    QLabel *m_whatIsLabel;
};


#endif  // KP_COLOR_SIMILARITY_DIALOG_H
