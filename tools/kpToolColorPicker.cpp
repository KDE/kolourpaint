
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


#define DEBUG_KP_TOOL_COLOR_PICKER 0


#include "kpToolColorPicker.h"
#include "kpLogCategories.h"
#include "widgets/toolbars/kpColorToolBar.h"
#include "commands/kpCommandHistory.h"
#include "kpDefs.h"
#include "document/kpDocument.h"
#include "pixmapfx/kpPixmapFX.h"
#include "commands/tools/kpToolColorPickerCommand.h"
#include "environments/tools/kpToolEnvironment.h"

#include <KLocalizedString>

kpToolColorPicker::kpToolColorPicker (kpToolEnvironment *environ, QObject *parent)
    : kpTool (i18n ("Color Picker"), i18n ("Lets you select a color from the image"),
              Qt::Key_C,
              environ, parent, QStringLiteral("tool_color_picker"))
{
}

kpToolColorPicker::~kpToolColorPicker () = default;


// private
kpColor kpToolColorPicker::colorAtPixel (const QPoint &p)
{
#if DEBUG_KP_TOOL_COLOR_PICKER && 0
    qCDebug(kpLogTools) << "kpToolColorPicker::colorAtPixel" << p;
#endif

    return kpPixmapFX::getColorAtPixel (document ()->image (), p);
}


// private
QString kpToolColorPicker::haventBegunDrawUserMessage () const
{
    return i18n ("Click to select a color.");
}


// public virtual [base kpTool]
void kpToolColorPicker::begin ()
{
    setUserMessage (haventBegunDrawUserMessage ());
}

// public virtual [base kpTool]
void kpToolColorPicker::beginDraw ()
{
    m_oldColor = color (mouseButton ());

    setUserMessage (cancelUserMessage ());
}

// public virtual [base kpTool]
void kpToolColorPicker::draw (const QPoint &thisPoint, const QPoint &, const QRect &)
{
    const kpColor color = colorAtPixel (thisPoint);

    if (color.isValid ())
    {
        environ ()->setColor (mouseButton (), color);
        setUserShapePoints (thisPoint);
    }
    else
    {
        environ ()->setColor (mouseButton (), m_oldColor);
        setUserShapePoints ();
    }
}

// public virtual [base kpTool]
void kpToolColorPicker::cancelShape ()
{
    environ ()->setColor (mouseButton (), m_oldColor);

    setUserMessage (i18n ("Let go of all the mouse buttons."));
}

// public virtual [base kpTool]
void kpToolColorPicker::releasedAllButtons ()
{
    setUserMessage (haventBegunDrawUserMessage ());

}

// public virtual [base kpTool]
void kpToolColorPicker::endDraw (const QPoint &thisPoint, const QRect &)
{
    const kpColor color = colorAtPixel (thisPoint);

    if (color.isValid ())
    {
        auto *cmd = new kpToolColorPickerCommand (  mouseButton (), color, m_oldColor,
                                                    environ ()->commandEnvironment ());

        environ ()->commandHistory ()->addCommand (cmd, false/*no exec*/);
        setUserMessage (haventBegunDrawUserMessage ());
    }
    else
    {
        cancelShape ();
    }
}


