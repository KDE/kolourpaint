
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#include "kpDocumentMetaInfoCommand.h"

#include "document/kpDocument.h"
#include "imagelib/kpDocumentMetaInfo.h"
#include "kpDefs.h"

struct kpDocumentMetaInfoCommandPrivate {
    kpDocumentMetaInfo metaInfo, oldMetaInfo;
};

kpDocumentMetaInfoCommand::kpDocumentMetaInfoCommand(const QString &name,
                                                     const kpDocumentMetaInfo &metaInfo,
                                                     const kpDocumentMetaInfo &oldMetaInfo,
                                                     kpCommandEnvironment *environ)

    : kpNamedCommand(name, environ)
    , d(new kpDocumentMetaInfoCommandPrivate())
{
    d->metaInfo = metaInfo;
    d->oldMetaInfo = oldMetaInfo;
}

kpDocumentMetaInfoCommand::~kpDocumentMetaInfoCommand()
{
    delete d;
}

// public virtual [base kpCommand]
kpCommandSize::SizeType kpDocumentMetaInfoCommand::size() const
{
    return d->metaInfo.size() + d->oldMetaInfo.size();
}

// public virtual [base kpCommand]
void kpDocumentMetaInfoCommand::execute()
{
    kpDocument *doc = document();
    Q_ASSERT(doc);

    doc->setMetaInfo(d->metaInfo);
    doc->setModified();
}

// public virtual [base kpCommand]
void kpDocumentMetaInfoCommand::unexecute()
{
    kpDocument *doc = document();
    Q_ASSERT(doc);

    // REFACTOR: Document in kpDocument.h that kpDocument::setMetaInfo() does not mutate modified state
    doc->setMetaInfo(d->oldMetaInfo);
    doc->setModified();
}
