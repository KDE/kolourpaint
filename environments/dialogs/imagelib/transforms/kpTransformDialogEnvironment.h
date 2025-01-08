
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef kpTransformDialogEnvironment_H
#define kpTransformDialogEnvironment_H

#include "environments/kpEnvironmentBase.h"

// Facade for kpTransformDialog* clients.
class kpTransformDialogEnvironment : public kpEnvironmentBase
{
    Q_OBJECT

public:
    // Note: Our interface must never publicly leak <mainWindow> or any other
    //       classes we are trying to hide as that would defeat the point of
    //       the facade.
    kpTransformDialogEnvironment(kpMainWindow *mainWindow);
};

#endif // kpTransformDialogEnvironment_H
