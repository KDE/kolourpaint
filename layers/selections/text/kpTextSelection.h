
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


#ifndef kpTextSelection_H
#define kpTextSelection_H


#include "layers/selections/kpAbstractSelection.h"
#include "imagelib/kpImage.h"
#include "layers/selections/text/kpTextStyle.h"
#include "layers/selections/text/kpPreeditText.h"

//
// A rectangular text box containing lines of text, rendered in a given text
// style.
//
// A text selection with an empty list of text lines is just a text box
// border and contains no content.
//
// The minimal text selection that is considered to contain content consists
// of a single empty text line.
//
// The rendered elements are as follows:
//
//     ###############
//     #             #
//     #    *        *---------- Text Border
//     #    |        #
//     #####|#########
//          |
//          |
//       Text Area
//       (text lines are rendered on top of here; the parts of the lines
//        that don't fit here are not rendered)
//
// The text style determines how the text box is drawn.
//
// The foreground color determines the color of the text.  Transparent
// foreground text means that the text box is see-through for the pixels
// of the text, exposing the document pixels below.  It does not mean
// that the transparent color is drawn onto the document.  In other
// words, we are "painting" transparent pixels -- not "setting" them.
//
// If the background color is opaque, the text is drawn on top of a
// filled-in rectangle and the rectangle completely overwrites any
// document pixels below.
//
// Consistent with the behavior of a transparent foreground color, a
// transparent background color does not mean that a transparent-colored
// rectangle is drawn onto the document.  Instead, it means that the
// text box background is see-through so that text is drawn directly on
// top of the document pixels.  No rectangle is drawn in this case.
//
// A text box with transparent foreground and background colors is
// completely invisible since both the text and background are see-through.
//
// A rendered text cursor is controlled separately by kpViewManager, as
// cursors are only a view concept so do not belong in this document-based
// class.
//
// Marshalling, for copying text selections to the clipboard, is not
// currently supported.  This is because generally, users are only interested
// in the text itself, not the border nor formatting.
//
class kpTextSelection : public kpAbstractSelection
{
Q_OBJECT

//
// Initialization
//

public:
    kpTextSelection (const QRect &rect = QRect (),
        const kpTextStyle &textStyle = kpTextStyle ());
    kpTextSelection (const QRect &rect, const QList <QString> &textLines,
        const kpTextStyle &textStyle);
    kpTextSelection (const kpTextSelection &rhs);

    kpTextSelection &operator= (const kpTextSelection &rhs);

    kpTextSelection *clone () const override;

    // Returns a copy of the text selection but with new dimensions
    // <newWidth> x <newHeight>.
    kpTextSelection *resized (int newWidth, int newHeight) const;

    ~kpTextSelection () override;


//
// Marshalling
//

public:
    int serialID () const override;

    bool readFromStream (QDataStream &stream) override;

    void writeToStream (QDataStream &stream) const override;


//
// General Queries
//

public:
    QString name () const override;

    kpCommandSize::SizeType size () const override;

public:
    bool isRectangular () const override;


//
// Position & Dimensions
//

public:
    // Returns the absolute minimum size that a textbox must be if it is of
    // the given <textStyle>.
    //
    // This leaves enough room for the border on all 4 sides and also a
    // text area big enough to fit a character in an extremely small font.
    static int MinimumWidthForTextStyle (const kpTextStyle &textStyle);
    static int MinimumHeightForTextStyle (const kpTextStyle &textStyle);
    static QSize MinimumSizeForTextStyle (const kpTextStyle &textStyle);

    // REFACTOR: Enforce in kpTextSelection, not just in kpToolSelection &
    //           when pasting (in kpMainWindow).
    //
    //           Otherwise, if enforcement fails, e.g. textAreaRect() will
    //           not work.
    int minimumWidth () const override;
    int minimumHeight () const override;

public:
    // Returns the suggested minimum size that a textbox should be if it is of
    // the given <textStyle>.
    //
    // This leaves enough room for the border on all 4 sides and also for
    // a small line of the text in the given text style.
    static int PreferredMinimumWidthForTextStyle (const kpTextStyle &textStyle);
    static int PreferredMinimumHeightForTextStyle (const kpTextStyle &textStyle);
    static QSize PreferredMinimumSizeForTextStyle (const kpTextStyle &textStyle);

public:
    // Returns the size of the text border.  Constant.
    static int TextBorderSize ();

    // Returns the rectangle that text lines are drawn on top of.
    // This will be a sub-rectangle of boundingRect() and is therefore,
    // in document coordinates like everything else in this class.
    QRect textAreaRect () const;

public:
    QPolygon calculatePoints () const override;


//
// Point Testing
//

public:
    bool contains (const QPoint &point) const override;

public:
    bool pointIsInTextBorderArea (const QPoint &point) const;
    bool pointIsInTextArea (const QPoint &point) const;


//
// Content
//

public:
    // (see class header comment)
    bool hasContent () const override;

    void deleteContent () override;

public:
    QList <QString> textLines () const;
    void setTextLines (const QList <QString> &textLines);

    static QString textForTextLines (const QList <QString> &textLines);
    // Returns textLines() as one long newline-separated string.
    // If the last text line is not empty, there is no trailing newline.
    QString text () const;


//
// Text Style
//

public:
    kpTextStyle textStyle () const;
    void setTextStyle (const kpTextStyle &textStyle);


//
// Preedit Text
//

public:
    kpPreeditText preeditText () const;
    void setPreeditText (const kpPreeditText &preeditText);

//
// Cursor
//
// A text cursor position is the row and column of a character in
// textLines(), that it is to the left of.  As a result, a column value
// of 1 character past the last character of a text line is allowed.
//

public:
    // If the given point is in the text area, it returns the closest
    // row/column (in textLines()) for the point.
    //
    // If the given point is not in the text area, it returns -1.
    int closestTextRowForPoint (const QPoint &point) const;
    int closestTextColForPoint (const QPoint &point) const;

    // Given a valid row and column in textLines(), returns the top-left
    // point of where the text cursor should be rendered.
    // TODO: Code is not symmetric to closestTest{Row,Col}ForPoint()
    //       [look at the Y/row value calculations]
    //
    // If the row and column is not inside textLines(), it returns
    // KP_INVALID_POINT.
    QPoint pointForTextRowCol (int row, int col) const;


//
// Rendering
//

private:
    void drawPreeditString(QPainter &painter, int &x, int y, const kpPreeditText &preeditText) const;

public:
    void paint(QImage *destPixmap, const QRect &docRect) const override;

    void paintBorder(QImage *destPixmap, const QRect &docRect,
                             bool selectionFinished) const override;

public:
    // Returns an image that contains the painted text (without a border).
    //
    // If the text box has a see-through background, the image will be given
    // an arbitrarily neutral background (currently, the transparent color).
    // As a result, the returned image will be an approximation since text
    // boxes are normally rendered -- and antialiased with -- a different
    // background, namely the document image.  Therefore, it is invalid to
    // stamp the returned image onto the document image and expect it to look
    // like stamping this text selection onto the document image (the latter
    // is achieved via kpDocument::selectionPushOntoDocument(), antialiases
    // and is more correct).
    kpImage approximateImage () const;


private:
    struct kpTextSelectionPrivate * const d;
};


#endif  // kpTextSelection_H
