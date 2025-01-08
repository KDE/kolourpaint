
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef kpDocumentMetaInfoCommand_H
#define kpDocumentMetaInfoCommand_H

#include "commands/kpNamedCommand.h"

class kpDocumentMetaInfo;

class kpDocumentMetaInfoCommand : public kpNamedCommand
{
public:
    kpDocumentMetaInfoCommand(const QString &name, const kpDocumentMetaInfo &metaInfo, const kpDocumentMetaInfo &oldMetaInfo, kpCommandEnvironment *environ);
    ~kpDocumentMetaInfoCommand() override;

    SizeType size() const override;

public:
    void execute() override;
    void unexecute() override;

private:
    struct kpDocumentMetaInfoCommandPrivate *const d;
};

#endif // kpDocumentMetaInfoCommand_H
