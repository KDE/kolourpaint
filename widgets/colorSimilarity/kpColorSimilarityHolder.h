
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef kpColorSimilarityHolder_H
#define kpColorSimilarityHolder_H

class QString;

class kpColorSimilarityHolder
{
public:
    kpColorSimilarityHolder();
    virtual ~kpColorSimilarityHolder();

    static const double ColorCubeDiagonalDistance;
    static const double MaxColorSimilarity;

    static QString WhatsThisWithClickInstructions();
    static QString WhatsThis();

    double colorSimilarity() const;

    // This automatically restricts the given <similarity> to the range
    // 0 .. MaxColorSimilarity inclusive.
    //
    // Override this if you need to act on mutations.
    // Remember to call this base implementation though.
    //
    // WARNING: The base constructor does not call this as virtual method
    //          calls in constructors do not invoke overrides anyway.
    virtual void setColorSimilarity(double similarity);

private:
    double m_colorSimilarity;
};

#endif // kpColorSimilarityHolder_H
