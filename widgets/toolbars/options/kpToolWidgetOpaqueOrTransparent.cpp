
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#define DEBUG_KP_TOOL_WIDGET_OPAQUE_OR_TRANSPARENT 0

#include "widgets/toolbars/options/kpToolWidgetOpaqueOrTransparent.h"

#include "kpLogCategories.h"
#include <KLocalizedString>

//---------------------------------------------------------------------

kpToolWidgetOpaqueOrTransparent::kpToolWidgetOpaqueOrTransparent(QWidget *parent, const QString &name)
    : kpToolWidgetBase(parent, name)
{
    addOption(QStringLiteral(":/icons/option_opaque"), i18n("Opaque") /*tooltip*/);
    startNewOptionRow();
    addOption(QStringLiteral(":/icons/option_transparent"), i18n("Transparent") /*tooltip*/);

    finishConstruction(0, 0);
}

//---------------------------------------------------------------------

kpToolWidgetOpaqueOrTransparent::~kpToolWidgetOpaqueOrTransparent() = default;

//---------------------------------------------------------------------

// public
bool kpToolWidgetOpaqueOrTransparent::isOpaque() const
{
    return (selected() == 0);
}

// public
bool kpToolWidgetOpaqueOrTransparent::isTransparent() const
{
    return (!isOpaque());
}

// public
void kpToolWidgetOpaqueOrTransparent::setOpaque(bool yes)
{
#if DEBUG_KP_TOOL_WIDGET_OPAQUE_OR_TRANSPARENT && 1
    qCDebug(kpLogWidgets) << "kpToolWidgetOpaqueOrTransparent::setOpaque(" << yes << ")";
#endif
    setSelected(yes ? 0 : 1, 0, false /*don't save*/);
}

// public
void kpToolWidgetOpaqueOrTransparent::setTransparent(bool yes)
{
#if DEBUG_KP_TOOL_WIDGET_OPAQUE_OR_TRANSPARENT && 1
    qCDebug(kpLogWidgets) << "kpToolWidgetOpaqueOrTransparent::setTransparent(" << yes << ")";
#endif
    setSelected(yes ? 1 : 0, 0, false /*don't save*/);
}

// protected slot virtual [base kpToolWidgetBase]
bool kpToolWidgetOpaqueOrTransparent::setSelected(int row, int col, bool saveAsDefault)
{
#if DEBUG_KP_TOOL_WIDGET_OPAQUE_OR_TRANSPARENT && 1
    qCDebug(kpLogWidgets) << "kpToolWidgetOpaqueOrTransparent::setSelected(" << row << "," << col << ")";
#endif
    const bool ret = kpToolWidgetBase::setSelected(row, col, saveAsDefault);
    if (ret) {
        Q_EMIT isOpaqueChanged(isOpaque());
    }
    return ret;
}

#include "moc_kpToolWidgetOpaqueOrTransparent.cpp"
