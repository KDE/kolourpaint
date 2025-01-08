
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#include "kpEffectWidgetBase.h"

kpEffectWidgetBase::kpEffectWidgetBase(bool actOnSelection, QWidget *parent)
    : QWidget(parent)
    , m_actOnSelection(actOnSelection)
{
}

kpEffectWidgetBase::~kpEffectWidgetBase() = default;

// public
QString kpEffectWidgetBase::caption() const
{
    return {};
}

#include "moc_kpEffectWidgetBase.cpp"
