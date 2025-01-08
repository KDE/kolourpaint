
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#include "kpAbstractSelectionContentCommand.h"

#include "layers/selections/kpAbstractSelection.h"

struct kpAbstractSelectionContentCommandPrivate {
    const kpAbstractSelection *orgSelBorder;
};

kpAbstractSelectionContentCommand::kpAbstractSelectionContentCommand(const kpAbstractSelection &originalSelBorder,
                                                                     const QString &name,
                                                                     kpCommandEnvironment *environ)
    : kpNamedCommand(name, environ)
    , d(new kpAbstractSelectionContentCommandPrivate())
{
    Q_ASSERT(!originalSelBorder.hasContent());

    d->orgSelBorder = originalSelBorder.clone();
}

kpAbstractSelectionContentCommand::~kpAbstractSelectionContentCommand()
{
    delete d->orgSelBorder;
    delete d;
}

// public virtual [base kpCommand]
kpCommandSize::SizeType kpAbstractSelectionContentCommand::size() const
{
    return d->orgSelBorder->size();
}

// public
const kpAbstractSelection *kpAbstractSelectionContentCommand::originalSelection() const
{
    return d->orgSelBorder;
}
