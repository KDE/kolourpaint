
/*
   Copyright (c) 2003-2004 Clarence Dang <dang@kde.org>
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

#define DEBUG_KP_TOOL_TEXT 1

#include <qfont.h>
#include <qfontmetrics.h>
#include <qpainter.h>

#include <kdebug.h>
#include <klocale.h>

#include <kpcolor.h>
#include <kpdocument.h>
#include <kpmainwindow.h>
#include <kppixmapfx.h>
#include <kptemppixmap.h>
#include <kptooltext.h>
#include <kptooltoolbar.h>
#include <kptoolwidgetopaqueortransparent.h>
#include <kpviewmanager.h>


static QPixmap pixmap (const QPixmap &docPixmap,
                       const QPoint &textTopLeft, const QValueVector <QString> textLines,
                       const kpTextStyle &textStyle,
                       const kpColor &foregroundColor, const kpColor &backgroundColor,
                       int cursorRow, int cursorCol, bool cursorOn)
{
#if DEBUG_KP_TOOL_TEXT
    kdDebug () << "pixmap()" << endl;
#endif

    QString bigString = textLines [0];
    for (QValueVector <QString>::const_iterator it = textLines.begin () + 1;
         it != textLines.end ();
         it++)
    {
        bigString += QString::fromLatin1 ("\n");
        bigString += (*it);
    }
#if DEBUG_KP_TOOL_TEXT
    kdDebug () << "\tbigString='" << bigString << "'" << endl;
#endif

#if DEBUG_KP_TOOL_TEXT
    kdDebug () << "\tfont: family=" << textStyle.fontFamily ()
               << " size=" << textStyle.fontSize ()
               << endl;
#endif
    QFont font (textStyle.fontFamily (), textStyle.fontSize ());
    font.setBold (textStyle.isBold ());
    font.setItalic (textStyle.isItalic ());
    font.setUnderline (textStyle.isUnderline ());
    font.setStrikeOut (textStyle.isStrikeThru ());

    QFontMetrics fontMetrics (font);
    QRect boundingRect = fontMetrics.boundingRect (bigString);
    boundingRect = QRect (0, 0, 200, 200);//HACK
    boundingRect.moveTopLeft (textTopLeft);


#if DEBUG_KP_TOOL_TEXT
    kdDebug () << "\tboundingRect=" << boundingRect << endl;
#endif


    QPixmap destPixmap = kpPixmapFX::getPixmapAt (docPixmap, boundingRect);
    destPixmap.fill (Qt::red);
#if DEBUG_KP_TOOL_TEXT
    kdDebug () << "\tdestPixmap.rect=" << destPixmap.rect () << endl;
#endif
    QPainter p (&destPixmap);
    p.setBackgroundMode (QPainter::OpaqueMode);
    p.setBackgroundColor (Qt::blue);
    p.setPen (Qt::green);
    p.setFont (font);

#if DEBUG_KP_TOOL_TEXT
    kdDebug () << "\tdrawing at pixmap rect=" << QRect (0, 0, boundingRect.width (), boundingRect.height ()) << endl;
#endif
    QRect actualBoundingRect;
    p.drawText (QRect (0, 0, boundingRect.width (), boundingRect.height ()), 0/*flags*/, bigString,
                -1/*len*/, &actualBoundingRect);
#if DEBUG_KP_TOOL_TEXT
    kdDebug () << "\texpected fontMetrics.boundingRect" << fontMetrics.boundingRect (bigString) << endl;
    kdDebug () << "\texpected qpainter.boundingRect" << p.boundingRect (QRect (0, 0, 2000, 2000), 0, bigString) << endl;
    kdDebug () << "\tactual boundingRect=" << actualBoundingRect << endl;
#endif


    if (cursorOn)
    {
        const int x = fontMetrics.width (textLines [cursorRow].left (cursorCol));
        const int y = cursorRow * fontMetrics.height () + (cursorRow >= 1 ? cursorRow * fontMetrics.leading () : 0);
        const int h = fontMetrics.height ();
    #if DEBUG_KP_TOOL_TEXT
        kdDebug () << "\tcursorRow=" << cursorRow << " cursorCol=" << cursorCol
                   << " fontMetrics: width_of_text=" << fontMetrics.width (textLines [cursorRow].left (cursorCol))
                   << " height=" << fontMetrics.height ()
                   << " leading=" << fontMetrics.leading ()
                   << " leftBearing('w')=" << fontMetrics.leftBearing ('w')
                   << " output: x=" << x << " y=" << y << " h=" << h << endl;
    #endif

        p.setRasterOp (Qt::XorROP);
        p.setPen (Qt::white);

        p.drawLine (x, y, x, y + h - 1);
    }


    p.end ();
    return destPixmap;
}


kpToolText::kpToolText (kpMainWindow *mainWindow)
    : kpTool (i18n ("Text"), i18n ("Writes text"), mainWindow, "tool_text"),
      m_hasDecidedTextTopLeft (false),
      m_toolWidgetOpaqueOrTransparent (0)
{
}

kpToolText::~kpToolText ()
{
}


// public virtual [base kpTool]
void kpToolText::begin ()
{
    kpToolToolBar *tb = toolToolBar ();

    if (tb)
    {
        m_toolWidgetOpaqueOrTransparent = tb->toolWidgetOpaqueOrTransparent ();

        if (m_toolWidgetOpaqueOrTransparent)
        {
            connect (m_toolWidgetOpaqueOrTransparent, SIGNAL (isOpaqueChanged (bool)),
                     this, SLOT (slotIsOpaqueChanged ()));
            m_toolWidgetOpaqueOrTransparent->show ();
        }
    }
    else
    {
        m_toolWidgetOpaqueOrTransparent = 0;
    }


    m_textStyle = mainWindow ()->textStyle ();


    mainWindow ()->enableTextToolBarActions (true);


    // TODO: Qt::ibeamCursor with (0, 0) as hotspot
    viewManager ()->setCursor (Qt::crossCursor);
}

// public virtual [base kpTool]
void kpToolText::end ()
{
    if (m_toolWidgetOpaqueOrTransparent)
    {
        disconnect (m_toolWidgetOpaqueOrTransparent, SIGNAL (isOpaqueChanged (bool)),
                    this, SLOT (slotIsOpaqueChanged ()));
        m_toolWidgetOpaqueOrTransparent = 0;
    }


    m_textPixmap.resize (0, 0);

    mainWindow ()->enableTextToolBarActions (false);

    viewManager ()->unsetCursor ();
}


// public virtual [base kpTool]
void kpToolText::beginDraw ()
{
#if DEBUG_KP_TOOL_TEXT
    kdDebug () << "kpToolText::beginDraw()" << endl;
#endif
}

// public virtual [base kpTool]
void kpToolText::cancelShape ()
{
#if DEBUG_KP_TOOL_TEXT
    kdDebug () << "kpToolText::cancelShape()" << endl;
#endif
}

// public virtual [base kpTool]
void kpToolText::endDraw (const QPoint &thisPoint, const QRect &)
{
#if DEBUG_KP_TOOL_TEXT
    kdDebug () << "kpToolText::endDraw()" << endl;
#endif

    m_textTopLeft = thisPoint;
    m_hasDecidedTextTopLeft = true;
    m_textLines = QValueVector <QString> (1, QString ());

    m_cursorRow = m_cursorCol = 0;
    m_cursorOn = true;

    viewManager ()->invalidateTempPixmap ();
    //m_textPixmap.resize (0, 0);
}


// public
bool kpToolText::hasDecidedTextTopLeft () const
{
    return m_hasDecidedTextTopLeft;
}

// public
QPoint kpToolText::textTopLeft () const
{
    if (!m_hasDecidedTextTopLeft)
    {
        kdError () << "kpToolText::textTopLeft() without having decided!" << endl;
        return QPoint ();
    }

    return m_textTopLeft;
}


// protected virtual [base kpTool]
void kpToolText::keyPressEvent (QKeyEvent *e)
{
#if DEBUG_KP_TOOL_TEXT
    kdDebug () << "kpToolText::keyPressEvent(e->text='" << e->text () << "')" << endl;
#endif


    e->ignore ();


    if (!m_hasDecidedTextTopLeft)
    {
    #if DEBUG_KP_TOOL_TEXT
        kdDebug () << "\thaven't decided topLeft - passing on event to kpTool" << endl;
    #endif
        kpTool::keyPressEvent (e);
        return;
    }


    if (e->key () == Qt::Key_Enter || e->key () == Qt::Key_Return)
    {
    #if DEBUG_KP_TOOL_TEXT
        kdDebug () << "\tenter pressed" << endl;
    #endif
        const QString rightHalf = m_textLines [m_cursorRow].mid (m_cursorCol);

        m_textLines [m_cursorRow].truncate (m_cursorCol);
        m_textLines.insert (m_textLines.begin () + m_cursorRow + 1, rightHalf);

        m_cursorRow++;
        m_cursorCol = 0;

        e->accept ();
    }
    else if (e->key () == Qt::Key_Backspace)
    {
    #if DEBUG_KP_TOOL_TEXT
        kdDebug () << "\tbackspace pressed" << endl;
    #endif

        if (m_cursorCol > 0)
        {
            m_textLines [m_cursorRow] = m_textLines [m_cursorRow].left (m_cursorCol - 1) +
                                        m_textLines [m_cursorRow].mid (m_cursorCol);
            m_cursorCol--;
        }
        else
        {
            if (m_cursorRow > 0)
            {
                int newCursorRow = m_cursorRow - 1;
                int newCursorCol = m_textLines [newCursorRow].length ();

                m_textLines [newCursorRow] += m_textLines [m_cursorRow];

                m_textLines.erase (m_textLines.begin () + m_cursorRow);

                m_cursorRow = newCursorRow;
                m_cursorCol = newCursorCol;
            }
        }

        e->accept ();
    }
    else if (e->key () == Qt::Key_Delete)
    {
    #if DEBUG_KP_TOOL_TEXT
        kdDebug () << "\tdelete pressed" << endl;
    #endif

        if (m_cursorCol < (int) m_textLines [m_cursorRow].length ())
        {
            m_textLines [m_cursorRow] = m_textLines [m_cursorRow].left (m_cursorCol) +
                                        m_textLines [m_cursorRow].mid (m_cursorCol + 1);
        }
        else
        {
            if (m_cursorRow < (int) m_textLines.size () - 1)
            {
                m_textLines [m_cursorRow] += m_textLines [m_cursorRow + 1];
                m_textLines.erase (m_textLines.begin () + m_cursorRow + 1);
            }
        }

        e->accept ();
    }
    else if (e->key () == Qt::Key_Up)
    {
    #if DEBUG_KP_TOOL_TEXT
        kdDebug () << "\tup pressed" << endl;
    #endif

        if (m_cursorRow > 0)
        {
            m_cursorRow--;
            m_cursorCol = QMIN (m_cursorCol, (int) m_textLines [m_cursorRow].length ());
        }

        e->accept ();
    }
    else if (e->key () == Qt::Key_Down)
    {
    #if DEBUG_KP_TOOL_TEXT
        kdDebug () << "\tdown pressed" << endl;
    #endif

        if (m_cursorRow < (int) m_textLines.size () - 1)
        {
            m_cursorRow++;
            m_cursorCol = QMIN (m_cursorCol, (int) m_textLines [m_cursorRow].length ());
        }

        e->accept ();
    }
    else if (e->key () == Qt::Key_Left)
    {
    #if DEBUG_KP_TOOL_TEXT
        kdDebug () << "\tleft pressed" << endl;
    #endif

    #define MOVE_CURSOR_LEFT()                                      \
    {                                                               \
        m_cursorCol--;                                              \
                                                                    \
        if (m_cursorCol < 0)                                        \
        {                                                           \
            m_cursorRow--;                                          \
            if (m_cursorRow < 0)                                    \
            {                                                       \
                m_cursorRow = 0;                                    \
                m_cursorCol = 0;                                    \
            }                                                       \
            else                                                    \
                m_cursorCol = m_textLines [m_cursorRow].length ();  \
        }                                                           \
    }

        if ((e->state () & Qt::ControlButton) == 0)
        {
        #if DEBUG_KP_TOOL_TEXT
            kdDebug () << "\tmove single char" << endl;
        #endif

            MOVE_CURSOR_LEFT ();
        }
        else
        {
        #if DEBUG_KP_TOOL_TEXT
            kdDebug () << "\tmove to start of word" << endl;
        #endif

            bool stillLooking = true;

            while (!(m_cursorRow == 0 && m_cursorCol == 0) && stillLooking)
            {
                while (m_cursorCol > 0 && m_textLines [m_cursorRow].at (m_cursorCol).isSpace ())
                {
                    MOVE_CURSOR_LEFT ();
                    stillLooking = false;
                }

                while (m_cursorCol > 0 && !m_textLines [m_cursorRow].at (m_cursorCol - 1).isSpace ())
                {
                    MOVE_CURSOR_LEFT ();
                    stillLooking = false;
                }

                if (stillLooking)
                    MOVE_CURSOR_LEFT ();
            }
        }

    #undef MOVE_CURSOR_LEFT

        e->accept ();

    }
    else if (e->key () == Qt::Key_Right)
    {
    #if DEBUG_KP_TOOL_TEXT
        kdDebug () << "\tright pressed" << endl;
    #endif

    #define MOVE_CURSOR_RIGHT()                                       \
    {                                                                 \
        m_cursorCol++;                                                \
                                                                      \
        if (m_cursorCol > (int) m_textLines [m_cursorRow].length ())  \
        {                                                             \
            m_cursorRow++;                                            \
            if (m_cursorRow > (int) m_textLines.size () - 1)          \
            {                                                         \
                m_cursorRow = m_textLines.size () - 1;                \
                m_cursorCol = m_textLines [m_cursorRow].length ();    \
            }                                                         \
            else                                                      \
                m_cursorCol = 0;                                      \
        }                                                             \
    }

        if ((e->state () & Qt::ControlButton) == 0)
        {
        #if DEBUG_KP_TOOL_TEXT
            kdDebug () << "\tmove single char" << endl;
        #endif

            MOVE_CURSOR_RIGHT ();
        }
        else
        {
        #if DEBUG_KP_TOOL_TEXT
            kdDebug () << "\tmove to start of word" << endl;
        #endif

            int oldCursorRow = m_cursorRow,
                oldCursorCol = m_cursorCol;

            bool stillLooking = true;

        #define IS_PAST_LAST_CHAR() (m_cursorRow == (int) m_textLines.size () - 1 &&           \
                                     m_cursorCol == (int) m_textLines [m_cursorRow].length ())
            while (!IS_PAST_LAST_CHAR() && stillLooking)
            {
            #if DEBUG_KP_TOOL_TEXT && 1
                kdDebug () << "\tcursorRow=" << m_cursorRow
                           << " m_cursorCol=" << m_cursorCol
                           << endl;
            #endif
                while (m_cursorCol < (int) m_textLines [m_cursorRow].length () - 1 &&
                       !m_textLines [m_cursorRow].at (m_cursorCol).isSpace ())
                {
                    MOVE_CURSOR_RIGHT ();
                    stillLooking = false;
                }

                while (m_cursorCol < (int) m_textLines [m_cursorRow].length () - 1 &&
                       m_textLines [m_cursorRow].at (m_cursorCol).isSpace ())
                {
                    MOVE_CURSOR_RIGHT ();
                    stillLooking = false;
                }

                if (stillLooking)
                    MOVE_CURSOR_RIGHT ();
            }

            if (IS_PAST_LAST_CHAR ())
            {
                // We started past the last char or on the last word
                // - no need to move
                m_cursorRow = oldCursorRow;
                m_cursorCol = oldCursorCol;
            }
        #undef IS_PAST_LAST_CHAR
        }

    #undef MOVE_CURSOR_RIGHT

        e->accept ();
    }
    else if (e->key () == Qt::Key_Home)
    {
    #if DEBUG_KP_TOOL_TEXT
        kdDebug () << "\thome pressed" << endl;
    #endif

        if (e->state () & Qt::ControlButton)
            m_cursorRow = 0;

        m_cursorCol = 0;

        e->accept ();
    }
    else if (e->key () == Qt::Key_End)
    {
    #if DEBUG_KP_TOOL_TEXT
        kdDebug () << "\tend pressed" << endl;
    #endif

        if (e->state () & Qt::ControlButton)
            m_cursorRow = m_textLines.size () - 1;

        m_cursorCol = m_textLines [m_cursorRow].length ();

        e->accept ();
    }
    else
    {
    #if DEBUG_KP_TOOL_TEXT
        kdDebug () << "\ttext='" << e->text () << "'" << endl;
    #endif
        QString usableText;
        for (int i = 0; i < (int) e->text ().length (); i++)
        {
            if (e->text ().at (i).isPrint ())
                usableText += e->text ().at (i);
        }
    #if DEBUG_KP_TOOL_TEXT
        kdDebug () << "\tusableText='" << usableText << "'" << endl;
    #endif

        if (usableText.length () > 0)
        {
            const QString leftHalf = m_textLines [m_cursorRow].left (m_cursorCol);
            const QString rightHalf = m_textLines [m_cursorRow].mid (m_cursorCol);
            m_textLines [m_cursorRow] = leftHalf + usableText + rightHalf;
            m_cursorCol += usableText.length ();

            e->accept ();
        }
    }


    if (!e->isAccepted ())
    {
    #if DEBUG_KP_TOOL_TEXT
        kdDebug () << "\tkey processing did not accept (text was '"
                   << e->text ()
                   << "') - passing on event to kpTool"
                   << endl;
    #endif
        kpTool::keyPressEvent (e);
        return;
    }


    m_cursorOn = true;

    kpColor backgroundColorToUse;
    if (m_toolWidgetOpaqueOrTransparent && m_toolWidgetOpaqueOrTransparent->isOpaque ())
        backgroundColorToUse = backgroundColor ();

    QPixmap pm = pixmap (*document ()->pixmap (),
                         m_textTopLeft, m_textLines,
                         m_textStyle,
                         foregroundColor (),
                         backgroundColorToUse,
                         m_cursorRow, m_cursorCol, m_cursorOn);
    kpTempPixmap tempPixmap (false/*not brush*/,
                             kpTempPixmap::SetPixmap,
                             m_textTopLeft,
                             pm);
    viewManager ()->setTempPixmap (tempPixmap);
}


// public slot
void kpToolText::slotFontFamilyChanged (const QString &)
{
    m_textStyle = mainWindow ()->textStyle ();
}

// public slot
void kpToolText::slotFontSizeChanged (int)
{
    m_textStyle = mainWindow ()->textStyle ();
}

// public slot
void kpToolText::slotBoldChanged (bool)
{
    m_textStyle = mainWindow ()->textStyle ();
}

// public slot
void kpToolText::slotItalicChanged (bool)
{
    m_textStyle = mainWindow ()->textStyle ();
}

// public slot
void kpToolText::slotUnderlineChanged (bool)
{
    m_textStyle = mainWindow ()->textStyle ();
}

// public slot
void kpToolText::slotStrikeThruChanged (bool)
{
    m_textStyle = mainWindow ()->textStyle ();
}


// private slot
void kpToolText::slotIsOpaqueChanged ()
{
    // update the shape
}

#include <kptooltext.moc>
