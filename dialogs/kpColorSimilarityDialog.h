
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
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
    explicit kpColorSimilarityDialog(QWidget *parent);
    ~kpColorSimilarityDialog() override;

    double colorSimilarity() const;
    void setColorSimilarity(double similarity);

private Q_SLOTS:
    void slotColorSimilarityValueChanged();

    void slotWhatIsLabelClicked();

private:
    kpColorSimilarityFrame *m_colorSimilarityFrame;
    kpIntNumInput *m_colorSimilarityInput;
    QLabel *m_whatIsLabel;
};

#endif // KP_COLOR_SIMILARITY_DIALOG_H
