
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


#define DEBUG_KP_TOOL_COLOR_PICKER 0


#include <kptoolcolorpicker.h>

#include <qimage.h>
#include <qpixmap.h>
#include <qpoint.h>

#include <kdebug.h>
#include <klocale.h>

#include <kpcolortoolbar.h>
#include <kpcommandhistory.h>
#include <kpdefs.h>
#include <kpdocument.h>
#include <kpmainwindow.h>
#include <kppixmapfx.h>


/*
 * kpToolColorPicker
 */

kpToolColorPicker::kpToolColorPicker (kpMainWindow *mainWindow)
    : kpTool (i18n ("Color Picker"), i18n ("Lets you select a color from the image"),
              Qt::Key_C,
              mainWindow, "tool_color_picker")
{
}

kpToolColorPicker::~kpToolColorPicker ()
{
}

kpColor kpToolColorPicker::colorAtPixel (const QPoint &p)
{
#if DEBUG_KP_TOOL_COLOR_PICKER && 0
    kdDebug () << "kpToolColorPicker::colorAtPixel" << p << endl;
#endif

    return kpPixmapFX::getColorAtPixel (*document ()->pixmap (), p);
}


QString kpToolColorPicker::haventBegunDrawUserMessage () const
{
    return i18n ("Click to select a color.");
}

void kpToolColorPicker::begin ()
{
    setUserMessage (haventBegunDrawUserMessage ());
}

// virtual
void kpToolColorPicker::beginDraw ()
{
    m_oldColor = color (m_mouseButton);

    setUserMessage (cancelUserMessage ());
}

// virtual
void kpToolColorPicker::draw (const QPoint &thisPoint, const QPoint &, const QRect &)
{
    const kpColor color = colorAtPixel (thisPoint);
    
    if (color.isValid ())
    {
        mainWindow ()->colorToolBar ()->setColor (m_mouseButton, color);
        setUserShapePoints (thisPoint);
    }
    else
    {
        mainWindow ()->colorToolBar ()->setColor (m_mouseButton, m_oldColor);
        setUserShapePoints ();
    }
}

// virtual
void kpToolColorPicker::cancelShape ()
{
    mainWindow ()->colorToolBar ()->setColor (m_mouseButton, m_oldColor);

    setUserMessage (i18n ("Let go of all the mouse buttons."));
}

void kpToolColorPicker::releasedAllButtons ()
{
    setUserMessage (haventBegunDrawUserMessage ());

}

// virtual
void kpToolColorPicker::endDraw (const QPoint &thisPoint, const QRect &)
{
    const kpColor color = colorAtPixel (thisPoint);

    if (color.isValid ())
    {
        kpToolColorPickerCommand *cmd = new kpToolColorPickerCommand (
                                                m_mouseButton,
                                                color, m_oldColor,
                                                mainWindow ());

        mainWindow ()->commandHistory ()->addCommand (cmd, false /* no exec */);
        setUserMessage (haventBegunDrawUserMessage ());
    }
    else
    {
        cancelShape ();
    }
}

/*
 * kpToolColorPickerCommand
 */

kpToolColorPickerCommand::kpToolColorPickerCommand (int mouseButton,
                                                    const kpColor &newColor,
                                                    const kpColor &oldColor,
                                                    kpMainWindow *mainWindow)
    : kpCommand (mainWindow),
      m_mouseButton (mouseButton),
      m_newColor (newColor),
      m_oldColor (oldColor)
{
}

kpToolColorPickerCommand::~kpToolColorPickerCommand ()
{
}


// public virtual [base kpCommand]
QString kpToolColorPickerCommand::name () const
{
    return i18n ("Color Picker");
}


// public virtual [base kpCommand]
int kpToolColorPickerCommand::size () const
{
    return 0;
}


// public virtual [base kpCommand]
void kpToolColorPickerCommand::execute ()
{
    colorToolBar ()->setColor (m_mouseButton, m_newColor);
}

// public virtual [base kpCommand]
void kpToolColorPickerCommand::unexecute ()
{
    colorToolBar ()->setColor (m_mouseButton, m_oldColor);
}


// private
kpColorToolBar *kpToolColorPickerCommand::colorToolBar () const
{
    return m_mainWindow ? m_mainWindow->colorToolBar () : 0;
}

#include <kptoolcolorpicker.moc>
