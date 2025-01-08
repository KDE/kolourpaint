
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
