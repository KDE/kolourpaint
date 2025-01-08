
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#define DEBUG_KP_TOOL_TEXT 0

#include "commands/kpCommandHistory.h"
#include "commands/tools/selection/kpToolSelectionCreateCommand.h"
#include "commands/tools/selection/text/kpToolTextBackspaceCommand.h"
#include "commands/tools/selection/text/kpToolTextChangeStyleCommand.h"
#include "commands/tools/selection/text/kpToolTextDeleteCommand.h"
#include "commands/tools/selection/text/kpToolTextEnterCommand.h"
#include "commands/tools/selection/text/kpToolTextGiveContentCommand.h"
#include "commands/tools/selection/text/kpToolTextInsertCommand.h"
#include "document/kpDocument.h"
#include "environments/tools/selection/kpToolSelectionEnvironment.h"
#include "kpLogCategories.h"
#include "kpToolTextPrivate.h"
#include "layers/selections/text/kpTextSelection.h"
#include "tools/selection/text/kpToolText.h"
#include "views/kpView.h"
#include "views/manager/kpViewManager.h"
#include "widgets/toolbars/options/kpToolWidgetOpaqueOrTransparent.h"

#include <KLocalizedString>

// protected
bool kpToolText::shouldChangeTextStyle() const
{
    if (environ()->settingTextStyle()) {
#if DEBUG_KP_TOOL_TEXT
        qCDebug(kpLogTools) << "\trecursion - abort setting text style: " << environ()->settingTextStyle();
#endif
        return false;
    }

    if (!document()->textSelection()) {
#if DEBUG_KP_TOOL_TEXT
        qCDebug(kpLogTools) << "\tno text selection - abort setting text style";
#endif
        return false;
    }

    return true;
}

// protected
void kpToolText::changeTextStyle(const QString &name, const kpTextStyle &newTextStyle, const kpTextStyle &oldTextStyle)
{
#if DEBUG_KP_TOOL_TEXT
    qCDebug(kpLogTools) << "kpToolText::changeTextStyle(" << name << ")";
#endif

    if (hasBegunShape()) {
        endShape(currentPoint(), normalizedRect());
    }

    commandHistory()->addCommand(new kpToolTextChangeStyleCommand(name, newTextStyle, oldTextStyle, environ()->commandEnvironment()));
}

// protected slot virtual [base kpAbstractSelectionTool]
void kpToolText::slotIsOpaqueChanged(bool isOpaque)
{
#if DEBUG_KP_TOOL_TEXT
    qCDebug(kpLogTools) << "kpToolText::slotIsOpaqueChanged()";
#endif

    if (!shouldChangeTextStyle()) {
        return;
    }

    kpTextStyle newTextStyle = environ()->textStyle();

    kpTextStyle oldTextStyle = newTextStyle;
    oldTextStyle.setBackgroundOpaque(!isOpaque);

    changeTextStyle(newTextStyle.isBackgroundOpaque() ? i18n("Text: Opaque Background") : i18n("Text: Transparent Background"), newTextStyle, oldTextStyle);
}

// protected slot virtual [base kpTool]
void kpToolText::slotColorsSwapped(const kpColor &newForegroundColor, const kpColor &newBackgroundColor)
{
#if DEBUG_KP_TOOL_TEXT
    qCDebug(kpLogTools) << "kpToolText::slotColorsSwapped()";
#endif

    if (!shouldChangeTextStyle()) {
        return;
    }

    kpTextStyle newTextStyle = environ()->textStyle();

    kpTextStyle oldTextStyle = newTextStyle;
    oldTextStyle.setForegroundColor(newBackgroundColor);
    oldTextStyle.setBackgroundColor(newForegroundColor);

    changeTextStyle(i18n("Text: Swap Colors"), newTextStyle, oldTextStyle);
}

// protected slot virtual [base kpTool]
void kpToolText::slotForegroundColorChanged(const kpColor & /*color*/)
{
#if DEBUG_KP_TOOL_TEXT
    qCDebug(kpLogTools) << "kpToolText::slotForegroundColorChanged()";
#endif

    if (!shouldChangeTextStyle()) {
        return;
    }

    kpTextStyle newTextStyle = environ()->textStyle();

    kpTextStyle oldTextStyle = newTextStyle;
    oldTextStyle.setForegroundColor(oldForegroundColor());

    changeTextStyle(i18n("Text: Foreground Color"), newTextStyle, oldTextStyle);
}

// protected slot virtual [base kpAbstractSelectionTool]
void kpToolText::slotBackgroundColorChanged(const kpColor & /*color*/)
{
#if DEBUG_KP_TOOL_TEXT
    qCDebug(kpLogTools) << "kpToolText::slotBackgroundColorChanged()";
#endif

    if (!shouldChangeTextStyle()) {
        return;
    }

    kpTextStyle newTextStyle = environ()->textStyle();

    kpTextStyle oldTextStyle = newTextStyle;
    oldTextStyle.setBackgroundColor(oldBackgroundColor());

    changeTextStyle(i18n("Text: Background Color"), newTextStyle, oldTextStyle);
}

// protected slot virtual [base kpAbstractSelectionTool]
void kpToolText::slotColorSimilarityChanged(double, int)
{
    // --- don't pass on event to kpAbstractSelectionTool which would have set the
    //     SelectionTransparency - not relevant to the Text Tool ---
}

// public slot
void kpToolText::slotFontFamilyChanged(const QString &fontFamily, const QString &oldFontFamily)
{
#if DEBUG_KP_TOOL_TEXT
    qCDebug(kpLogTools) << "kpToolText::slotFontFamilyChanged() new=" << fontFamily << " old=" << oldFontFamily;
#else
    (void)fontFamily;
#endif

    if (!shouldChangeTextStyle()) {
        return;
    }

    kpTextStyle newTextStyle = environ()->textStyle();

    // Figure out old text style.
    kpTextStyle oldTextStyle = newTextStyle;
    oldTextStyle.setFontFamily(oldFontFamily);

    changeTextStyle(i18n("Text: Font"), newTextStyle, oldTextStyle);
}

// public slot
void kpToolText::slotFontSizeChanged(int fontSize, int oldFontSize)
{
#if DEBUG_KP_TOOL_TEXT
    qCDebug(kpLogTools) << "kpToolText::slotFontSizeChanged() new=" << fontSize << " old=" << oldFontSize;
#else
    (void)fontSize;
#endif

    if (!shouldChangeTextStyle()) {
        return;
    }

    kpTextStyle newTextStyle = environ()->textStyle();

    // Figure out old text style.
    kpTextStyle oldTextStyle = newTextStyle;
    oldTextStyle.setFontSize(oldFontSize);

    changeTextStyle(i18n("Text: Font Size"), newTextStyle, oldTextStyle);
}

// public slot
void kpToolText::slotBoldChanged(bool isBold)
{
#if DEBUG_KP_TOOL_TEXT
    qCDebug(kpLogTools) << "kpToolText::slotBoldChanged(" << isBold << ")";
#endif

    if (!shouldChangeTextStyle()) {
        return;
    }

    kpTextStyle newTextStyle = environ()->textStyle();

    // Figure out old text style.
    kpTextStyle oldTextStyle = newTextStyle;
    oldTextStyle.setBold(!isBold);

    changeTextStyle(i18n("Text: Bold"), newTextStyle, oldTextStyle);
}

// public slot
void kpToolText::slotItalicChanged(bool isItalic)
{
#if DEBUG_KP_TOOL_TEXT
    qCDebug(kpLogTools) << "kpToolText::slotItalicChanged(" << isItalic << ")";
#endif

    if (!shouldChangeTextStyle()) {
        return;
    }

    kpTextStyle newTextStyle = environ()->textStyle();

    // Figure out old text style.
    kpTextStyle oldTextStyle = newTextStyle;
    oldTextStyle.setItalic(!isItalic);

    changeTextStyle(i18n("Text: Italic"), newTextStyle, oldTextStyle);
}

// public slot
void kpToolText::slotUnderlineChanged(bool isUnderline)
{
#if DEBUG_KP_TOOL_TEXT
    qCDebug(kpLogTools) << "kpToolText::slotUnderlineChanged(" << isUnderline << ")";
#endif

    if (!shouldChangeTextStyle()) {
        return;
    }

    kpTextStyle newTextStyle = environ()->textStyle();

    // Figure out old text style.
    kpTextStyle oldTextStyle = newTextStyle;
    oldTextStyle.setUnderline(!isUnderline);

    changeTextStyle(i18n("Text: Underline"), newTextStyle, oldTextStyle);
}

// public slot
void kpToolText::slotStrikeThruChanged(bool isStrikeThru)
{
#if DEBUG_KP_TOOL_TEXT
    qCDebug(kpLogTools) << "kpToolText::slotStrikeThruChanged(" << isStrikeThru << ")";
#endif

    if (!shouldChangeTextStyle()) {
        return;
    }

    kpTextStyle newTextStyle = environ()->textStyle();

    // Figure out old text style.
    kpTextStyle oldTextStyle = newTextStyle;
    oldTextStyle.setStrikeThru(!isStrikeThru);

    changeTextStyle(i18n("Text: Strike Through"), newTextStyle, oldTextStyle);
}
