
/*
   Copyright (c) 2003-2007 Clarence Dang <dang@kde.org>
   Copyright (c) 2010 Tasuku Suzuki <stasuku@gmail.com>
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


#define DEBUG_KP_SELECTION 0


#include "kpTextSelection.h"
#include "kpTextSelectionPrivate.h"

#include "kpDefs.h"
#include "kpTextStyle.h"

#include "kpLogCategories.h"

#include <QFontMetrics>
#include <QList>


// public
kpTextSelection::kpTextSelection (const QRect &rect,
        const kpTextStyle &textStyle)
    : kpAbstractSelection (rect),
      d (new kpTextSelectionPrivate ())
{
    d->textStyle = textStyle;
}

// public
kpTextSelection::kpTextSelection (const QRect &rect,
        const QList <QString> &textLines,
        const kpTextStyle &textStyle)
    : kpAbstractSelection (rect),
      d (new kpTextSelectionPrivate ())
{
    d->textLines = textLines;
    d->textStyle = textStyle;
}

// public
kpTextSelection::kpTextSelection (const kpTextSelection &rhs)
    : kpAbstractSelection (),
      d (new kpTextSelectionPrivate ())
{
    *this = rhs;
}

// public
kpTextSelection &kpTextSelection::operator= (const kpTextSelection &rhs)
{
    kpAbstractSelection::operator= (rhs);

    d->textLines = rhs.d->textLines;
    d->textStyle = rhs.d->textStyle;
    d->preeditText = rhs.d->preeditText;

    return *this;
}

// public virtual [base kpAbstractSelection]
kpTextSelection *kpTextSelection::clone () const
{
    kpTextSelection *sel = new kpTextSelection ();
    *sel = *this;
    return sel;
}

// public
kpTextSelection *kpTextSelection::resized (int newWidth, int newHeight) const
{
    return new kpTextSelection (QRect (x (), y (), newWidth, newHeight),
        d->textLines,
        d->textStyle);
}

// public
kpTextSelection::~kpTextSelection ()
{
    delete d;
}


// public virtual [kpAbstractSelection]
int kpTextSelection::serialID () const
{
    Q_ASSERT (!"Marshalling not supported");
    return -1;
}

// public virtual [base kpAbstractSelection]
bool kpTextSelection::readFromStream (QDataStream &stream)
{
    (void) stream;

    Q_ASSERT (!"Marshalling not supported");
    return false;
}

// public virtual [base kpAbstractSelection]
void kpTextSelection::writeToStream (QDataStream &stream) const
{
    (void) stream;

    Q_ASSERT (!"Marshalling not supported");
}


// public virtual [kpAbstractSelection]
QString kpTextSelection::name () const
{
    return i18n ("Text");
}


// public virtual [base kpAbstractSelection]
kpCommandSize::SizeType kpTextSelection::size () const
{
    return kpAbstractSelection::size () +
        kpCommandSize::StringSize (text ());
}


// public virtual [kpAbstractSelection]
bool kpTextSelection::isRectangular () const
{
    return true;
}


// public static
int kpTextSelection::MinimumWidthForTextStyle (const kpTextStyle &)
{
    return (kpTextSelection::TextBorderSize () * 2 + 5);
}

// public static
int kpTextSelection::MinimumHeightForTextStyle (const kpTextStyle &)
{
    return (kpTextSelection::TextBorderSize () * 2 + 5);
}

// public static
QSize kpTextSelection::MinimumSizeForTextStyle (const kpTextStyle &textStyle)
{
    return  {kpTextSelection::MinimumWidthForTextStyle (textStyle),
                kpTextSelection::MinimumHeightForTextStyle (textStyle)};
}


// public virtual [kpAbstractSelection]
int kpTextSelection::minimumWidth () const
{
    return kpTextSelection::MinimumWidthForTextStyle (textStyle ());
}

// public virtual [kpAbstractSelection]
int kpTextSelection::minimumHeight () const
{
    return kpTextSelection::MinimumHeightForTextStyle (textStyle ());
}


// public static
int kpTextSelection::PreferredMinimumWidthForTextStyle (const kpTextStyle &textStyle)
{
    const int about15CharsWidth =
        textStyle.fontMetrics().horizontalAdvance(QLatin1String("1234567890abcde"));

    const int preferredMinWidth =
        qMax (150,
              kpTextSelection::TextBorderSize () * 2 + about15CharsWidth);

    return qMax (kpTextSelection::MinimumWidthForTextStyle (textStyle),
                 qMin (250, preferredMinWidth));
}

// public static
int kpTextSelection::PreferredMinimumHeightForTextStyle (const kpTextStyle &textStyle)
{
    const int preferredMinHeight =
        kpTextSelection::TextBorderSize () * 2 + textStyle.fontMetrics ().height ();

    return qMax (kpTextSelection::MinimumHeightForTextStyle (textStyle),
                 qMin (150, preferredMinHeight));
}

// public static
QSize kpTextSelection::PreferredMinimumSizeForTextStyle (const kpTextStyle &textStyle)
{
    return  {kpTextSelection::PreferredMinimumWidthForTextStyle (textStyle),
                kpTextSelection::PreferredMinimumHeightForTextStyle (textStyle)};
}


// public static
int kpTextSelection::TextBorderSize ()
{
    return 1;
}

// public
QRect kpTextSelection::textAreaRect () const
{
    return  {x () + kpTextSelection::TextBorderSize (),
                y () + kpTextSelection::TextBorderSize (),
                width () - kpTextSelection::TextBorderSize () * 2,
                height () - kpTextSelection::TextBorderSize () * 2};
}


// public virtual [kpAbstractSelection]
QPolygon kpTextSelection::calculatePoints () const
{
    return kpAbstractSelection::CalculatePointsForRectangle (boundingRect ());
}


// public virtual [kpAbstractSelection]
bool kpTextSelection::contains (const QPoint &point) const
{
    return boundingRect ().contains (point);
}


// public
bool kpTextSelection::pointIsInTextBorderArea (const QPoint &point) const
{
    return (boundingRect ().contains (point) && !pointIsInTextArea (point));
}

// public
bool kpTextSelection::pointIsInTextArea (const QPoint &point) const
{
    return textAreaRect ().contains (point);
}


// public virtual [kpAbstractSelection]
bool kpTextSelection::hasContent () const
{
    return !d->textLines.isEmpty ();
}

// public virtual [kpAbstractSelection]
void kpTextSelection::deleteContent ()
{
    if (!hasContent ()) {
        return;
    }

    setTextLines (QList <QString> ());
}


// public
QList <QString> kpTextSelection::textLines () const
{
    return d->textLines;
}

// public
void kpTextSelection::setTextLines (const QList <QString> &textLines_)
{
    d->textLines = textLines_;

    emit changed (boundingRect ());
}

//--------------------------------------------------------------------------------

// public static
QString kpTextSelection::textForTextLines(const QList<QString> &textLines)
{
    if (textLines.isEmpty ())
      return QString();

    QString bigString = textLines[0];

    for (int i = 1; i < textLines.count(); i++)
    {
        bigString += QLatin1String("\n");
        bigString += textLines[i];
    }

    return bigString;
}

//--------------------------------------------------------------------------------

// public
QString kpTextSelection::text () const
{
    return kpTextSelection::textForTextLines (d->textLines);
}


// public
kpTextStyle kpTextSelection::textStyle () const
{
    return d->textStyle;
}

// public
void kpTextSelection::setTextStyle (const kpTextStyle &textStyle)
{
    d->textStyle = textStyle;

    emit changed (boundingRect ());
}

kpPreeditText kpTextSelection::preeditText () const
{
    return d->preeditText;
}

void kpTextSelection::setPreeditText (const kpPreeditText &preeditText)
{
    d->preeditText = preeditText;
    emit changed (boundingRect ());
}

