
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


#include "kpDocumentMetaInfoCommand.h"

#include "document/kpDocument.h"
#include "imagelib/kpDocumentMetaInfo.h"
#include "kpDefs.h"


struct kpDocumentMetaInfoCommandPrivate
{
    kpDocumentMetaInfo metaInfo, oldMetaInfo;
};

kpDocumentMetaInfoCommand::kpDocumentMetaInfoCommand (const QString &name,
        const kpDocumentMetaInfo &metaInfo,
        const kpDocumentMetaInfo &oldMetaInfo,
        kpCommandEnvironment *environ)

    : kpNamedCommand (name, environ),
      d (new kpDocumentMetaInfoCommandPrivate ())
{
    d->metaInfo = metaInfo;
    d->oldMetaInfo = oldMetaInfo;
}

kpDocumentMetaInfoCommand::~kpDocumentMetaInfoCommand ()
{
    delete d;
}


// public virtual [base kpCommand]
kpCommandSize::SizeType kpDocumentMetaInfoCommand::size () const
{
    return d->metaInfo.size () + d->oldMetaInfo.size ();
}


// public virtual [base kpCommand]
void kpDocumentMetaInfoCommand::execute ()
{
    kpDocument *doc = document ();
    Q_ASSERT (doc);

    doc->setMetaInfo (d->metaInfo);
    doc->setModified ();
}

// public virtual [base kpCommand]
void kpDocumentMetaInfoCommand::unexecute ()
{
    kpDocument *doc = document ();
    Q_ASSERT (doc);

    // REFACTOR: Document in kpDocument.h that kpDocument::setMetaInfo() does not mutate modified state
    doc->setMetaInfo (d->oldMetaInfo);
    doc->setModified ();
}

