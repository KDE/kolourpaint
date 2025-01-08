
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef kpToolSelectionEnvironment_H
#define kpToolSelectionEnvironment_H

#include "environments/tools/kpToolEnvironment.h"

class QMenu;

class kpImageSelectionTransparency;
class kpTextStyle;

// Facade for kpToolSelection clients.
class kpToolSelectionEnvironment : public kpToolEnvironment
{
    Q_OBJECT

public:
    // Note: Our interface must never publicly leak <mainWindow> or any other
    //       classes we are trying to hide as that would defeat the point of
    //       the facade.
    explicit kpToolSelectionEnvironment(kpMainWindow *mainWindow);
    ~kpToolSelectionEnvironment() override;

    kpImageSelectionTransparency imageSelectionTransparency() const;
    int settingImageSelectionTransparency() const;

    void deselectSelection() const;

    QMenu *selectionToolRMBMenu() const;

    void enableTextToolBarActions(bool enable = true) const;

    kpTextStyle textStyle() const;
    int settingTextStyle() const;

private:
    struct kpToolSelectionEnvironmentPrivate *const d;
};

#endif // kpToolSelectionEnvironment_H
