
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#define DEBUG_KP_TOOL_COLOR_PICKER 0

#include "kpToolColorPicker.h"
#include "commands/kpCommandHistory.h"
#include "commands/tools/kpToolColorPickerCommand.h"
#include "document/kpDocument.h"
#include "environments/tools/kpToolEnvironment.h"
#include "kpDefs.h"
#include "kpLogCategories.h"
#include "pixmapfx/kpPixmapFX.h"
#include "widgets/toolbars/kpColorToolBar.h"

#include <KLocalizedString>

kpToolColorPicker::kpToolColorPicker(kpToolEnvironment *environ, QObject *parent)
    : kpTool(i18n("Color Picker"), i18n("Lets you select a color from the image"), Qt::Key_C, environ, parent, QStringLiteral("tool_color_picker"))
{
}

kpToolColorPicker::~kpToolColorPicker() = default;

// private
kpColor kpToolColorPicker::colorAtPixel(const QPoint &p)
{
#if DEBUG_KP_TOOL_COLOR_PICKER && 0
    qCDebug(kpLogTools) << "kpToolColorPicker::colorAtPixel" << p;
#endif

    return kpPixmapFX::getColorAtPixel(document()->image(), p);
}

// private
QString kpToolColorPicker::haventBegunDrawUserMessage() const
{
    return i18n("Click to select a color.");
}

// public virtual [base kpTool]
void kpToolColorPicker::begin()
{
    setUserMessage(haventBegunDrawUserMessage());
}

// public virtual [base kpTool]
void kpToolColorPicker::beginDraw()
{
    m_oldColor = color(mouseButton());

    setUserMessage(cancelUserMessage());
}

// public virtual [base kpTool]
void kpToolColorPicker::draw(const QPoint &thisPoint, const QPoint &, const QRect &)
{
    const kpColor color = colorAtPixel(thisPoint);

    if (color.isValid()) {
        environ()->setColor(mouseButton(), color);
        setUserShapePoints(thisPoint);
    } else {
        environ()->setColor(mouseButton(), m_oldColor);
        setUserShapePoints();
    }
}

// public virtual [base kpTool]
void kpToolColorPicker::cancelShape()
{
    environ()->setColor(mouseButton(), m_oldColor);

    setUserMessage(i18n("Let go of all the mouse buttons."));
}

// public virtual [base kpTool]
void kpToolColorPicker::releasedAllButtons()
{
    setUserMessage(haventBegunDrawUserMessage());
}

// public virtual [base kpTool]
void kpToolColorPicker::endDraw(const QPoint &thisPoint, const QRect &)
{
    const kpColor color = colorAtPixel(thisPoint);

    if (color.isValid()) {
        auto *cmd = new kpToolColorPickerCommand(mouseButton(), color, m_oldColor, environ()->commandEnvironment());

        environ()->commandHistory()->addCommand(cmd, false /*no exec*/);
        setUserMessage(haventBegunDrawUserMessage());
    } else {
        cancelShape();
    }
}

#include "moc_kpToolColorPicker.cpp"
