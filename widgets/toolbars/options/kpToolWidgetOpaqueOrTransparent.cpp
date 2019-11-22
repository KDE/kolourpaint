
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

#define DEBUG_KP_TOOL_WIDGET_OPAQUE_OR_TRANSPARENT 0


#include "widgets/toolbars/options/kpToolWidgetOpaqueOrTransparent.h"

#include "kpLogCategories.h"
#include <KLocalizedString>


//---------------------------------------------------------------------

kpToolWidgetOpaqueOrTransparent::kpToolWidgetOpaqueOrTransparent (QWidget *parent, const QString &name)
    : kpToolWidgetBase (parent, name)
{
    addOption (QStringLiteral(":/icons/option_opaque"), i18n ("Opaque")/*tooltip*/);
    startNewOptionRow ();
    addOption (QStringLiteral(":/icons/option_transparent"), i18n ("Transparent")/*tooltip*/);

    finishConstruction (0, 0);
}

//---------------------------------------------------------------------

kpToolWidgetOpaqueOrTransparent::~kpToolWidgetOpaqueOrTransparent () = default;

//---------------------------------------------------------------------


// public
bool kpToolWidgetOpaqueOrTransparent::isOpaque () const
{
    return (selected () == 0);
}

// public
bool kpToolWidgetOpaqueOrTransparent::isTransparent () const
{
    return (!isOpaque ());
}

// public
void kpToolWidgetOpaqueOrTransparent::setOpaque (bool yes)
{
#if DEBUG_KP_TOOL_WIDGET_OPAQUE_OR_TRANSPARENT && 1
    qCDebug(kpLogWidgets) << "kpToolWidgetOpaqueOrTransparent::setOpaque(" << yes << ")";
#endif
    setSelected (yes ? 0 : 1, 0, false/*don't save*/);
}

// public
void kpToolWidgetOpaqueOrTransparent::setTransparent (bool yes)
{
#if DEBUG_KP_TOOL_WIDGET_OPAQUE_OR_TRANSPARENT && 1
    qCDebug(kpLogWidgets) << "kpToolWidgetOpaqueOrTransparent::setTransparent(" << yes << ")";
#endif
    setSelected (yes ? 1 : 0, 0, false/*don't save*/);
}


// protected slot virtual [base kpToolWidgetBase]
bool kpToolWidgetOpaqueOrTransparent::setSelected (int row, int col, bool saveAsDefault)
{
#if DEBUG_KP_TOOL_WIDGET_OPAQUE_OR_TRANSPARENT && 1
    qCDebug(kpLogWidgets) << "kpToolWidgetOpaqueOrTransparent::setSelected("
               << row << "," << col << ")";
#endif
    const bool ret = kpToolWidgetBase::setSelected (row, col, saveAsDefault);
    if (ret) {
        emit isOpaqueChanged (isOpaque ());
    }
    return ret;
}


