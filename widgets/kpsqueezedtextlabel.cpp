
/*
   Copyright (c) 2003,2004,2005 Clarence Dang <dang@kde.org>
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

#define DEBUG_KP_SQUEEZED_TEXT_LABEL 0


#include <kpsqueezedtextlabel.h>

#include <qfont.h>
#include <qfontmetrics.h>
#include <qstring.h>

#include <kdebug.h>
#include <klocale.h>


kpSqueezedTextLabel::kpSqueezedTextLabel (QWidget *parent, const char *name)
    : QLabel (parent, name),
      m_showEllipsis (true)
{
}

kpSqueezedTextLabel::kpSqueezedTextLabel (const QString &text, QWidget *parent, const char *name)
    : QLabel (parent, name),
      m_showEllipsis (true)
{
    setText (text);
}


// public virtual
QSize kpSqueezedTextLabel::minimumSizeHint () const
{
#if DEBUG_KP_SQUEEZED_TEXT_LABEL && 1
    kdDebug () << "kpSqueezedTextLabel::minimumSizeHint() qLabel prefers"
               << QLabel::minimumSizeHint () << endl;
#endif
    return QSize (-1/*no minimum width*/, QLabel::minimumHeight ());
}


// public
QString kpSqueezedTextLabel::fullText () const
{
    return m_fullText;
}


// public
bool kpSqueezedTextLabel::showEllipsis () const
{
    return m_showEllipsis;
}

// public
void kpSqueezedTextLabel::setShowEllipsis (bool yes)
{
    if (m_showEllipsis == yes)
        return;

    m_showEllipsis = yes;
    
    squeezeText ();
}


// public slots virtual [base QLabel]
void kpSqueezedTextLabel::setText (const QString &text)
{
    m_fullText = text;
    squeezeText ();
}


// protected virtual [base QWidget]
void kpSqueezedTextLabel::resizeEvent (QResizeEvent *e)
{
#if DEBUG_KP_SQUEEZED_TEXT_LABEL && 1
    kdDebug () << "kpSqueezedTextLabeL::resizeEvent() size=" << e->size ()
               << " oldSize=" << e->oldSize ()
               << endl;
#endif
    squeezeText ();
}


// protected
QString kpSqueezedTextLabel::ellipsisText () const
{
    return m_showEllipsis ? i18n ("...") : QString::null;
}
    
// protected
void kpSqueezedTextLabel::squeezeText ()
{
#if DEBUG_KP_SQUEEZED_TEXT_LABEL && 1
    kdDebug () << "kpSqueezedTextLabeL::squeezeText" << endl;
#endif

    QFontMetrics fontMetrics (font ());
    int fullTextWidth = fontMetrics.width (m_fullText);
#if DEBUG_KP_SQUEEZED_TEXT_LABEL && 1
    kdDebug () << "\tfullText=" << m_fullText
               << " fullTextWidth=" << fullTextWidth
               << " labelWidth=" << width ()
               << endl;
#endif

    if (fullTextWidth <= width ())
    {
    #if DEBUG_KP_SQUEEZED_TEXT_LABEL && 1
        kdDebug () << "\tfullText will fit - display" << endl;
    #endif
        QLabel::setText (m_fullText);
    }
    else
    {
    #if DEBUG_KP_SQUEEZED_TEXT_LABEL && 1
        kdDebug () << "\tfullText won't fit :( - squeeze" << endl;
        kdDebug () << "\t\twidth of \"...\"="
                   << fontMetrics.width (ellipsisText ())
                   << endl;

    #endif
        if (fontMetrics.width (ellipsisText ()) > width ())
        {
        #if DEBUG_KP_SQUEEZED_TEXT_LABEL && 1
            kdDebug () << "\t\t\tcan't even fit \"...\" - forget it" << endl;
        #endif
            QLabel::setText (QString::null);
            return;
        }

        // Binary search our way to fit squeezed text
        int numLettersToUseLo = 0;
        int numLettersToUseHi = m_fullText.length ();
        int numLettersToUse = 0;

        while (numLettersToUseLo <= numLettersToUseHi)
        {
            int numLettersToUseMid = (numLettersToUseLo + numLettersToUseHi) / 2;
            int squeezedWidth = fontMetrics.width (m_fullText.left (numLettersToUseMid) + ellipsisText ());
        #if DEBUG_KP_SQUEEZED_TEXT_LABEL && 1
            kdDebug () << "\tbsearch: lo=" << numLettersToUseLo
                       << " hi=" << numLettersToUseHi
                       << " mid=" << numLettersToUseMid
                       << " acceptable=" << numLettersToUse
                       << " squeezedWidth=" << squeezedWidth
                       << endl;
        #endif

            if (squeezedWidth == width ())
            {
            #if DEBUG_KP_SQUEEZED_TEXT_LABEL && 1
                kdDebug () << "\t\tperfect match!" << endl;
            #endif
                numLettersToUse = numLettersToUseMid;
                break;
            }
            else if (squeezedWidth < width ())
            {
            #if DEBUG_KP_SQUEEZED_TEXT_LABEL && 1
                kdDebug () << "\t\tsmall enough - numLettersToUse="
                           << numLettersToUse << endl;
            #endif
                if (numLettersToUseMid > numLettersToUse)
                {
                    numLettersToUse = numLettersToUseMid;
                #if DEBUG_KP_SQUEEZED_TEXT_LABEL && 1
                    kdDebug () << "\t\t\tset numLettersToUse="
                               << numLettersToUse
                               << endl;
                #endif
                }

                numLettersToUseLo = numLettersToUseMid + 1;
            }
            else
            {
            #if DEBUG_KP_SQUEEZED_TEXT_LABEL && 1
                kdDebug () << "\t\ttoo big" << endl;
            #endif
                numLettersToUseHi = numLettersToUseMid - 1;
            }
        }

        QLabel::setText (m_fullText.left (numLettersToUse) + ellipsisText ());
    }
}

#include <kpsqueezedtextlabel.moc>
