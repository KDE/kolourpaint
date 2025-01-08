
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef kpCommandEnvironment_H
#define kpCommandEnvironment_H

#include "environments/kpEnvironmentBase.h"

class kpMainWindow;
class kpImageSelectionTransparency;
class kpTextStyle;

// Facade for kpCommand clients.
class kpCommandEnvironment : public kpEnvironmentBase
{
    Q_OBJECT

public:
    // Note: Our interface must never publicly leak <mainWindow> or any other
    //       classes we are trying to hide as that would defeat the point of
    //       the facade.
    kpCommandEnvironment(kpMainWindow *mainWindow);
    ~kpCommandEnvironment() override;

    void somethingBelowTheCursorChanged() const;

    // Sets the foreground and background drawing colors in the UI.
    void setColor(int which, const kpColor &color) const;

    //
    // Selections
    //

    kpImageSelectionTransparency imageSelectionTransparency() const;
    void setImageSelectionTransparency(const kpImageSelectionTransparency &transparency, bool forceColorChange = false);

    kpTextStyle textStyle() const;
    void setTextStyle(const kpTextStyle &textStyle);

private:
    struct kpCommandEnvironmentPrivate *const d;
};

#endif // kpCommandEnvironment_H
