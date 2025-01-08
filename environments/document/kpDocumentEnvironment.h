
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

#ifndef kpDocumentEnvironment_H
#define kpDocumentEnvironment_H

#include "environments/kpEnvironmentBase.h"

class QWidget;

class kpAbstractSelection;

// Facade for kpDocument clients.
class kpDocumentEnvironment : public kpEnvironmentBase
{
    Q_OBJECT

public:
    // Note: Our interface must never publicly leak <mainWindow> or any other
    //       classes we are trying to hide as that would defeat the point of
    //       the facade.
    kpDocumentEnvironment(kpMainWindow *mainWindow);
    ~kpDocumentEnvironment() override;

    QWidget *dialogParent() const;

    void setQueueViewUpdates() const;
    void restoreQueueViewUpdates() const;

    void switchToCompatibleTool(const kpAbstractSelection &selection,
                                // REFACTOR: This is horrible API and terribly named.
                                bool *isTextChanged) const;
    void assertMatchingUIState(const kpAbstractSelection &selection) const;

private:
    struct kpDocumentEnvironmentPrivate *const d;
};

#endif // kpDocumentEnvironment_H
