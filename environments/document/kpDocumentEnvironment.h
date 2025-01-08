
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
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
