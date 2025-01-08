
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#include "kpEffectCommandBase.h"

#include "document/kpDocument.h"
#include "generic/kpSetOverrideCursorSaver.h"
#include "kpDefs.h"

#include <KLocalizedString>

#include <QCursor>

//--------------------------------------------------------------------------------

struct kpEffectCommandBasePrivate {
    QString name;
    bool actOnSelection{false};

    kpImage oldImage;
};

kpEffectCommandBase::kpEffectCommandBase(const QString &name, bool actOnSelection, kpCommandEnvironment *environ)
    : kpCommand(environ)
    , d(new kpEffectCommandBasePrivate())
{
    d->name = name;
    d->actOnSelection = actOnSelection;
}

kpEffectCommandBase::~kpEffectCommandBase()
{
    delete d;
}

// public virtual [base kpCommand]
QString kpEffectCommandBase::name() const
{
    return (d->actOnSelection) ? i18n("Selection: %1", d->name) : d->name;
}

// public virtual [base kpCommand]
kpCommandSize::SizeType kpEffectCommandBase::size() const
{
    return ImageSize(d->oldImage);
}

// public virtual [base kpCommand]
void kpEffectCommandBase::execute()
{
    kpSetOverrideCursorSaver cursorSaver(Qt::WaitCursor);

    kpDocument *doc = document();
    Q_ASSERT(doc);

    const kpImage oldImage = doc->image(d->actOnSelection);

    if (!isInvertible()) {
        d->oldImage = oldImage;
    }

    kpImage newImage = /*pure virtual*/ applyEffect(oldImage);

    doc->setImage(d->actOnSelection, newImage);
}

// public virtual [base kpCommand]
void kpEffectCommandBase::unexecute()
{
    kpSetOverrideCursorSaver cursorSaver(Qt::WaitCursor);

    kpDocument *doc = document();
    Q_ASSERT(doc);

    kpImage newImage;

    if (!isInvertible()) {
        newImage = d->oldImage;
    } else {
        newImage = /*pure virtual*/ applyEffect(doc->image(d->actOnSelection));
    }

    doc->setImage(d->actOnSelection, newImage);

    d->oldImage = kpImage();
}
