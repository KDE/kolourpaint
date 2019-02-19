
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


#include "kpEffectCommandBase.h"

#include "kpDefs.h"
#include "document/kpDocument.h"
#include "generic/kpSetOverrideCursorSaver.h"

#include <KLocalizedString>

//--------------------------------------------------------------------------------

struct kpEffectCommandBasePrivate
{
    QString name;
    bool actOnSelection{false};

    kpImage oldImage;
};

kpEffectCommandBase::kpEffectCommandBase (const QString &name,
        bool actOnSelection,
        kpCommandEnvironment *environ)
    : kpCommand (environ),
      d (new kpEffectCommandBasePrivate ())
{
    d->name = name;
    d->actOnSelection = actOnSelection;
}

kpEffectCommandBase::~kpEffectCommandBase ()
{
    delete d;
}


// public virtual [base kpCommand]
QString kpEffectCommandBase::name () const
{
    return (d->actOnSelection) ? i18n ("Selection: %1", d->name) : d->name;
}


// public virtual [base kpCommand]
kpCommandSize::SizeType kpEffectCommandBase::size () const
{
    return ImageSize (d->oldImage);
}


// public virtual [base kpCommand]
void kpEffectCommandBase::execute ()
{
    kpSetOverrideCursorSaver cursorSaver (Qt::WaitCursor);

    kpDocument *doc = document ();
    Q_ASSERT (doc);


    const kpImage oldImage = doc->image (d->actOnSelection);

    if (!isInvertible ())
    {
        d->oldImage = oldImage;
    }


    kpImage newImage = /*pure virtual*/applyEffect (oldImage);

    doc->setImage (d->actOnSelection, newImage);
}

// public virtual [base kpCommand]
void kpEffectCommandBase::unexecute ()
{
    kpSetOverrideCursorSaver cursorSaver (Qt::WaitCursor);

    kpDocument *doc = document ();
    Q_ASSERT (doc);


    kpImage newImage;

    if (!isInvertible ())
    {
        newImage = d->oldImage;
    }
    else
    {
        newImage = /*pure virtual*/applyEffect (doc->image (d->actOnSelection));
    }

    doc->setImage (d->actOnSelection, newImage);


    d->oldImage = kpImage ();
}

