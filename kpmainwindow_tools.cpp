
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

#include <kdebug.h>

#include <kpcolortoolbar.h>
#include <kpmainwindow.h>
#include <kptool.h>
#include <kptoolairspray.h>
#include <kptoolbrush.h>
#include <kptoolcolorpicker.h>
#include <kptoolcolorwasher.h>
#include <kptoolcurve.h>
#include <kptoolellipticalselection.h>
#include <kptoolellipse.h>
#include <kptooleraser.h>
#include <kptoolfloodfill.h>
#include <kptoolfreeformselection.h>
#include <kptoolline.h>
#include <kptoolpen.h>
#include <kptoolpolygon.h>
#include <kptoolpolyline.h>
#include <kptoolrectangle.h>
#include <kptoolrectselection.h>
#include <kptoolroundedrectangle.h>
#include <kptooltext.h>
#include <kptooltoolbar.h>


// private
void kpMainWindow::setupTools ()
{
    //
    // create tools
    //
    
    m_tools.setAutoDelete (true);
    
    m_tools.append (m_toolRectSelection = new kpToolRectSelection (this));
    m_tools.append (m_toolEllipticalSelection = new kpToolEllipticalSelection (this));
    
    m_tools.append (m_toolFreeFormSelection = new kpToolFreeFormSelection (this));
    m_tools.append (m_toolColorPicker = new kpToolColorPicker (this));
    
    m_tools.append (m_toolPen = new kpToolPen (this));
    m_tools.append (m_toolLine = new kpToolLine (this));

    m_tools.append (m_toolBrush = new kpToolBrush (this));
    m_tools.append (m_toolEraser = new kpToolEraser (this));
    
    m_tools.append (m_toolAirSpray = new kpToolAirSpray (this));
    m_tools.append (m_toolColorWasher = new kpToolColorWasher (this));
    
    m_tools.append (m_toolRectangle = new kpToolRectangle (this));
    m_tools.append (m_toolRoundedRectangle = new kpToolRoundedRectangle (this));
    
    m_tools.append (m_toolEllipse = new kpToolEllipse (this));
    m_tools.append (m_toolFloodFill = new kpToolFloodFill (this));

    // TODO: insert in a meaningful place
    m_tools.append (m_toolPolygon = new kpToolPolygon (this));
    m_tools.append (m_toolPolyline = new kpToolPolyline (this));
    m_tools.append (m_toolCurve = new kpToolCurve (this));
    m_tools.append (m_toolText = new kpToolText (this));

    
    //
    // create Toolbox
    //
    
    m_toolToolBar = new kpToolToolBar (this, 2/*columns/rows*/, "Tool Box");
    connect (m_toolToolBar, SIGNAL (toolSelected (kpTool *)), SLOT (switchToTool (kpTool *)));
    
    for (kpTool *tool = m_tools.first (); m_tools.current (); tool = m_tools.next ())
        m_toolToolBar->registerTool (tool);


    //
    // select initial tool
    //
    
    m_currentTool = m_previousTool = NULL;
    m_toolToolBar->selectTool (m_toolBrush);  // CONFIG: last used
}


// public
kpTool *kpMainWindow::tool () const
{
    return m_currentTool;
}

// public
bool kpMainWindow::toolHasBegunDraw () const
{
    return (m_currentTool && m_currentTool->hasBegunDraw ());
}


// public slot
// TODO: belongs to the tooltoolbar
void kpMainWindow::switchToTool (kpTool *tool)
{
#if DEBUG_KPMAINWINDOW
    kdDebug () << "kpMainWindow::switchToTool (" << tool << ")" << endl;
#endif

    if (tool == m_currentTool)
        return;

    if (m_currentTool)
    {
        m_currentTool->endInternal ();

        disconnect (m_currentTool, SIGNAL (beganDraw (const QPoint &)),
                    this, SLOT (slotUpdateStatusBar (const QPoint &)));
        disconnect (m_currentTool, SIGNAL (endedDraw (const QPoint &)),
                    this, SLOT (slotUpdateStatusBar (const QPoint &)));
        disconnect (m_currentTool, SIGNAL (mouseMoved (const QPoint &)),
                    this, SLOT (slotUpdateStatusBar (const QPoint &)));
        disconnect (m_currentTool, SIGNAL (mouseDragged (const QRect &)),
                    this, SLOT (slotUpdateStatusBar (const QRect &)));

        disconnect (m_colorToolBar, SIGNAL (foregroundColorChanged (const QColor &)),
                    m_currentTool, SLOT (slotForegroundColorChanged (const QColor &)));
        disconnect (m_colorToolBar, SIGNAL (backgroundColorChanged (const QColor &)),
                    m_currentTool, SLOT (slotBackgroundColorChanged (const QColor &)));
    }

    m_previousTool = m_currentTool;

    if (tool)
    {
        m_currentTool = tool;
        m_currentTool->beginInternal ();

        connect (m_currentTool, SIGNAL (beganDraw (const QPoint &)),
                 SLOT (slotUpdateStatusBar (const QPoint &)));
        connect (m_currentTool, SIGNAL (endedDraw (const QPoint &)),
                 SLOT (slotUpdateStatusBar (const QPoint &)));
        connect (m_currentTool, SIGNAL (mouseMoved (const QPoint &)),
                 SLOT (slotUpdateStatusBar (const QPoint &)));
        connect (m_currentTool, SIGNAL (mouseDragged (const QRect &)),
                 SLOT (slotUpdateStatusBar (const QRect &)));

        connect (m_colorToolBar, SIGNAL (foregroundColorChanged (const QColor &)),
                 m_currentTool, SLOT (slotForegroundColorChanged (const QColor &)));
        connect (m_colorToolBar, SIGNAL (backgroundColorChanged (const QColor &)),
                 m_currentTool, SLOT (slotBackgroundColorChanged (const QColor &)));
    }
    else
        m_currentTool = 0;
}

// public slot
void kpMainWindow::switchToPreviousTool ()
{
    if (m_previousTool)
        switchToTool (m_previousTool);
}


#define SLOT_TOOL(toolName)              \
void kpMainWindow::slotTool##toolName () \
{                                        \
KP_IGNORE_SLOT_CALL_IF_TOOL_ACTIVE;      \
    switchToTool (m_tool##toolName);     \
}

SLOT_TOOL (AirSpray)
SLOT_TOOL (Brush)
SLOT_TOOL (ColorPicker)
SLOT_TOOL (ColorWasher)
SLOT_TOOL (Curve)
SLOT_TOOL (Ellipse)
SLOT_TOOL (EllipticalSelection)
SLOT_TOOL (Eraser)
SLOT_TOOL (FloodFill)
SLOT_TOOL (FreeFormSelection)
SLOT_TOOL (Line)
SLOT_TOOL (Pen)
SLOT_TOOL (Polygon)
SLOT_TOOL (Polyline)
SLOT_TOOL (Rectangle)
SLOT_TOOL (RectSelection)
SLOT_TOOL (RoundedRectangle)
SLOT_TOOL (Text)
