
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


#include <kdebug.h>

#include <kpcolortoolbar.h>
#include <kpmainwindow.h>
#include <kpselectiontransparency.h>
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
#include <kptoolwidgetopaqueortransparent.h>


// private
void kpMainWindow::setupTools ()
{
    //
    // create tools
    //

    m_tools.setAutoDelete (true);

    m_tools.append (m_toolFreeFormSelection = new kpToolFreeFormSelection (this));
    m_tools.append (m_toolRectSelection = new kpToolRectSelection (this));

    m_tools.append (m_toolEllipticalSelection = new kpToolEllipticalSelection (this));
    m_tools.append (m_toolText = new kpToolText (this));

    m_tools.append (m_toolLine = new kpToolLine (this));
    m_tools.append (m_toolPen = new kpToolPen (this));

    m_tools.append (m_toolEraser = new kpToolEraser (this));
    m_tools.append (m_toolBrush = new kpToolBrush (this));

    m_tools.append (m_toolFloodFill = new kpToolFloodFill (this));
    m_tools.append (m_toolColorPicker = new kpToolColorPicker (this));

    m_tools.append (m_toolColorWasher = new kpToolColorWasher (this));
    m_tools.append (m_toolAirSpray = new kpToolAirSpray (this));

    m_tools.append (m_toolRoundedRectangle = new kpToolRoundedRectangle (this));
    m_tools.append (m_toolRectangle = new kpToolRectangle (this));

    m_tools.append (m_toolPolygon = new kpToolPolygon (this));
    m_tools.append (m_toolEllipse = new kpToolEllipse (this));

    m_tools.append (m_toolPolyline = new kpToolPolyline (this));
    m_tools.append (m_toolCurve = new kpToolCurve (this));


    //
    // create Toolbox
    //

    m_toolToolBar = new kpToolToolBar (this, 2/*columns/rows*/, "Tool Box");
    connect (m_toolToolBar, SIGNAL (sigToolSelected (kpTool *)),
             this, SLOT (slotToolSelected (kpTool *)));

    for (QPtrList <kpTool>::const_iterator it = m_tools.begin ();
         it != m_tools.end ();
         it++)
    {
        m_toolToolBar->registerTool (*it);
    }


    enableToolsDocumentActions (false);
}

// private
void kpMainWindow::enableToolsDocumentActions (bool enable)
{
    if (enable && !m_toolToolBar->isEnabled ())
    {
        kpTool *previousTool = m_toolToolBar->previousTool ();

        // select tool for enabled Tool Box

        if (previousTool)
            m_toolToolBar->selectPreviousTool ();
        else
            m_toolToolBar->selectTool (m_toolPen);  // CONFIG: last used
    }
    else if (!enable && m_toolToolBar->isEnabled ())
    {
        // don't have a disabled Tool Box with an enabled Tool
        m_toolToolBar->selectTool (0);
    }


    // not using actions for tools - so can just disable toolbar
    m_toolToolBar->setEnabled (enable);
}


// public
kpTool *kpMainWindow::tool () const
{
    return m_toolToolBar ? m_toolToolBar->tool () : 0;
}

// public
bool kpMainWindow::toolHasBegunShape () const
{
    kpTool *currentTool = tool ();
    return (currentTool && currentTool->hasBegunShape ());
}

// public
bool kpMainWindow::toolIsASelectionTool (bool includingTextTool) const
{
    kpTool *currentTool = tool ();

    return ((currentTool == m_toolFreeFormSelection) ||
            (currentTool == m_toolRectSelection) ||
            (currentTool == m_toolEllipticalSelection) ||
            (currentTool == m_toolText && includingTextTool));
}

// public
bool kpMainWindow::toolIsTextTool () const
{
    return (tool () == m_toolText);
}


// public
kpSelectionTransparency kpMainWindow::selectionTransparency () const
{
    kpToolWidgetOpaqueOrTransparent *oot = m_toolToolBar->toolWidgetOpaqueOrTransparent ();
    if (!oot)
    {
        kdError () << "kpMainWindow::selectionTransparency() without opaqueOrTransparent widget" << endl;
        return kpSelectionTransparency ();
    }

    return kpSelectionTransparency (oot->isOpaque (), backgroundColor (), m_colorToolBar->colorSimilarity ());
}

// public
void kpMainWindow::setSelectionTransparency (const kpSelectionTransparency &transparency, bool forceColorChange)
{
#if DEBUG_KP_MAIN_WINDOW && 1
    kdDebug () << "kpMainWindow::setSelectionTransparency() isOpaque=" << transparency.isOpaque ()
               << " color=" << (transparency.transparentColor ().isValid () ? (int *) transparency.transparentColor ().toQRgb () : 0)
               << " forceColorChange=" << forceColorChange
               << endl;
#endif
    
    kpToolWidgetOpaqueOrTransparent *oot = m_toolToolBar->toolWidgetOpaqueOrTransparent ();
    if (!oot)
    {
        kdError () << "kpMainWindow::setSelectionTransparency() without opaqueOrTransparent widget" << endl;
        return;
    }

    d->m_settingSelectionTransparency++;
    
    oot->setOpaque (transparency.isOpaque ());
    if (transparency.isTransparent () || forceColorChange)
    {
        m_colorToolBar->setColor (1, transparency.transparentColor ());
        m_colorToolBar->setColorSimilarity (transparency.colorSimilarity ());
    }

    d->m_settingSelectionTransparency--;
}

// public
int kpMainWindow::settingSelectionTransparency () const
{
    return d->m_settingSelectionTransparency;
}
    

// private slot
void kpMainWindow::slotToolSelected (kpTool *tool)
{
#if DEBUG_KP_MAIN_WINDOW
    kdDebug () << "kpMainWindow::slotToolSelected (" << tool << ")" << endl;
#endif

    kpTool *previousTool = m_toolToolBar ? m_toolToolBar->previousTool () : 0;

    if (previousTool)
    {
        disconnect (previousTool, SIGNAL (userMessageChanged (const QString &)),
                   this, SLOT (slotUpdateStatusBarMessage (const QString &)));
        disconnect (previousTool, SIGNAL (userShapePointsChanged (const QPoint &, const QPoint &)),
                   this, SLOT (slotUpdateStatusBarShapePoints (const QPoint &, const QPoint &)));
        disconnect (previousTool, SIGNAL (userShapeSizeChanged (const QSize &)),
                   this, SLOT (slotUpdateStatusBarShapeSize (const QSize &)));

        disconnect (m_colorToolBar, SIGNAL (foregroundColorChanged (const kpColor &)),
                    previousTool, SLOT (slotForegroundColorChanged (const kpColor &)));
        disconnect (m_colorToolBar, SIGNAL (backgroundColorChanged (const kpColor &)),
                    previousTool, SLOT (slotBackgroundColorChanged (const kpColor &)));
        disconnect (m_colorToolBar, SIGNAL (colorSimilarityChanged (double, int)),
                    previousTool, SLOT (slotColorSimilarityChanged (double, int)));
    }

    if (tool)
    {
        connect (tool, SIGNAL (userMessageChanged (const QString &)),
                 this, SLOT (slotUpdateStatusBarMessage (const QString &)));
        connect (tool, SIGNAL (userShapePointsChanged (const QPoint &, const QPoint &)),
                 this, SLOT (slotUpdateStatusBarShapePoints (const QPoint &, const QPoint &)));
        connect (tool, SIGNAL (userShapeSizeChanged (const QSize &)),
                 this, SLOT (slotUpdateStatusBarShapeSize (const QSize &)));
        slotUpdateStatusBar ();

        connect (m_colorToolBar, SIGNAL (foregroundColorChanged (const kpColor &)),
                 tool, SLOT (slotForegroundColorChanged (const kpColor &)));
        connect (m_colorToolBar, SIGNAL (backgroundColorChanged (const kpColor &)),
                 tool, SLOT (slotBackgroundColorChanged (const kpColor &)));
        connect (m_colorToolBar, SIGNAL (colorSimilarityChanged (double, int)),
                 tool, SLOT (slotColorSimilarityChanged (double, int)));
    }
}


// public slots

#define SLOT_TOOL(toolName)                       \
void kpMainWindow::slotTool##toolName ()          \
{                                                 \
    if (!m_toolToolBar)                           \
        return;                                   \
                                                  \
    if (tool () == m_tool##toolName)              \
        return;                                   \
                                                  \
    m_toolToolBar->selectTool (m_tool##toolName); \
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
