
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#define DEBUG_KP_RESIZE_SIGNALLING_LABEL 0

#include "generic/widgets/kpResizeSignallingLabel.h"

#include "kpLogCategories.h"

kpResizeSignallingLabel::kpResizeSignallingLabel(const QString &string, QWidget *parent)
    : QLabel(string, parent)
{
}

kpResizeSignallingLabel::kpResizeSignallingLabel(QWidget *parent)
    : QLabel(parent)
{
}

kpResizeSignallingLabel::~kpResizeSignallingLabel() = default;

// protected virtual [base QLabel]
void kpResizeSignallingLabel::resizeEvent(QResizeEvent *e)
{
#if DEBUG_KP_RESIZE_SIGNALLING_LABEL
    qCDebug(kpLogMisc) << "kpResizeSignallingLabel::resizeEvent() newSize=" << e->size() << " oldSize=" << e->oldSize();
#endif
    QLabel::resizeEvent(e);

    Q_EMIT resized();
}

#include "moc_kpResizeSignallingLabel.cpp"
