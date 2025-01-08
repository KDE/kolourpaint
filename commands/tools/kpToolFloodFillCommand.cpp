
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#define DEBUG_KP_TOOL_FLOOD_FILL 0

#include "kpToolFloodFillCommand.h"

#include "document/kpDocument.h"
#include "imagelib/kpColor.h"
#include "imagelib/kpImage.h"
#include "kpDefs.h"
#include "kpLogCategories.h"

#include <QApplication>

#include <KLocalizedString>

//---------------------------------------------------------------------

struct kpToolFloodFillCommandPrivate {
    kpImage oldImage;
    bool fillEntireImage{false};
};

//---------------------------------------------------------------------

kpToolFloodFillCommand::kpToolFloodFillCommand(int x, int y, const kpColor &color, int processedColorSimilarity, kpCommandEnvironment *environ)

    : kpCommand(environ)
    , kpFloodFill(document()->imagePointer(), x, y, color, processedColorSimilarity)
    , d(new kpToolFloodFillCommandPrivate())
{
    d->fillEntireImage = false;
}

//---------------------------------------------------------------------

kpToolFloodFillCommand::~kpToolFloodFillCommand()
{
    delete d;
}

//---------------------------------------------------------------------

// public virtual [base kpCommand]
QString kpToolFloodFillCommand::name() const
{
    return i18n("Flood Fill");
}

//---------------------------------------------------------------------

// public virtual [base kpCommand]
kpCommandSize::SizeType kpToolFloodFillCommand::size() const
{
    return kpFloodFill::size() + ImageSize(d->oldImage);
}

//---------------------------------------------------------------------

// public
void kpToolFloodFillCommand::setFillEntireImage(bool yes)
{
    d->fillEntireImage = yes;
}

//---------------------------------------------------------------------

// protected virtual [base kpCommand]
void kpToolFloodFillCommand::execute()
{
#if DEBUG_KP_TOOL_FLOOD_FILL && 1
    qCDebug(kpLogCommands) << "kpToolFloodFillCommand::execute() fillEntireImage=" << d->fillEntireImage;
#endif

    kpDocument *doc = document();
    Q_ASSERT(doc);

    if (d->fillEntireImage) {
        doc->fill(kpFloodFill::color());
    } else {
        QRect rect = kpFloodFill::boundingRect();
        if (rect.isValid()) {
            QApplication::setOverrideCursor(Qt::WaitCursor);
            {
                d->oldImage = doc->getImageAt(rect);

                kpFloodFill::fill();
                doc->slotContentsChanged(rect);
            }
            QApplication::restoreOverrideCursor();
        } else {
#if DEBUG_KP_TOOL_FLOOD_FILL && 1
            qCDebug(kpLogCommands) << "\tinvalid boundingRect - must be NOP case";
#endif
        }
    }
}

//---------------------------------------------------------------------

// protected virtual [base kpCommand]
void kpToolFloodFillCommand::unexecute()
{
#if DEBUG_KP_TOOL_FLOOD_FILL && 1
    qCDebug(kpLogCommands) << "kpToolFloodFillCommand::unexecute() fillEntireImage=" << d->fillEntireImage;
#endif

    kpDocument *doc = document();
    Q_ASSERT(doc);

    if (d->fillEntireImage) {
        doc->fill(kpFloodFill::colorToChange());
    } else {
        QRect rect = kpFloodFill::boundingRect();
        if (rect.isValid()) {
            doc->setImageAt(d->oldImage, rect.topLeft());

            d->oldImage = kpImage();

            doc->slotContentsChanged(rect);
        }
    }
}

//---------------------------------------------------------------------
