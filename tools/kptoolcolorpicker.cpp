
/* This file is part of the KolourPaint project
   Copyright (c) 2003 Clarence Dang <dang@kde.org>
   All rights reserved.
   
   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:
   
   1. Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
   2. Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
   3. Neither the names of the copyright holders nor the names of
      contributors may be used to endorse or promote products derived from
      this software without specific prior written permission.
   
   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
   PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
   HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <qimage.h>
#include <qpixmap.h>
#include <qpoint.h>

#include <kdebug.h>
#include <klocale.h>

#include <kpcolortoolbar.h>
#include <kpdefs.h>
#include <kpdocument.h>
#include <kpmainwindow.h>
#include <kptoolcolorpicker.h>


/*
 * kpToolColorPicker
 */

kpToolColorPicker::kpToolColorPicker (kpMainWindow *mainWindow)
    : kpTool (i18n ("Color Picker"), i18n ("Allows you to selects a color from your image"),
              mainWindow, "tool_color_picker")
{
}

kpToolColorPicker::~kpToolColorPicker ()
{
}

QColor kpToolColorPicker::colorAtPixel (const QPoint &p)
{
    QPixmap pixmap = document ()->getPixmapAt (QRect (QPoint (p), QPoint (p)));
    QImage image = pixmap.convertToImage ();
    if (image.isNull ())
    {
        kdError () << "kpToolColorPicker::colorAtPixel() could not convert to QImage" << endl;
        return Qt::black;
    }

    return QColor (image.pixel (0, 0));
}

// virtual
void kpToolColorPicker::beginDraw ()
{
    m_oldColor = color (m_mouseButton);
}

// virtual
void kpToolColorPicker::draw (const QPoint &thisPoint, const QPoint &, const QRect &)
{
    mainWindow ()->colorToolBar ()->setColor (m_mouseButton, colorAtPixel (thisPoint));
    emit mouseMoved (thisPoint);
}

// virtual
void kpToolColorPicker::cancelDraw ()
{
#if 0
    endDraw (m_currentPoint, QRect ());
    mainWindow ()->commandHistory ()->undo ();
#else
    mainWindow ()->colorToolBar ()->setColor (m_mouseButton, m_oldColor);
#endif
}

// virtual
void kpToolColorPicker::endDraw (const QPoint &thisPoint, const QRect &)
{
    kpToolColorPickerCommand *cmd = new kpToolColorPickerCommand (
                                            mainWindow ()->colorToolBar (),
                                            m_mouseButton,
                                            colorAtPixel (thisPoint), m_oldColor);

    mainWindow ()->commandHistory ()->addCommand (cmd, false /* no exec */);
}

/*
 * kpToolColorPickerCommand
 */

kpToolColorPickerCommand::kpToolColorPickerCommand (kpColorToolBar *colorToolBar,
                                                    int mouseButton,
                                                    const QColor &newColor,
                                                    const QColor &oldColor)
    : m_colorToolBar (colorToolBar),
      m_mouseButton (mouseButton),
      m_newColor (newColor),
      m_oldColor (oldColor)
{
}

kpToolColorPickerCommand::~kpToolColorPickerCommand ()
{
}

// virtual
void kpToolColorPickerCommand::execute ()
{
    m_colorToolBar->setColor (m_mouseButton, m_newColor);
}

// virtual
void kpToolColorPickerCommand::unexecute ()
{
    m_colorToolBar->setColor (m_mouseButton, m_oldColor);
}

// virtual
QString kpToolColorPickerCommand::name () const
{
    return i18n ("Color Picker");
}

#include <kptoolcolorpicker.moc>
