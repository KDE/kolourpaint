
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef kpCommand_H
#define kpCommand_H

#include "kpCommandSize.h"
#undef environ // macro on win32

class QString;

class kpAbstractImageSelection;
class kpAbstractSelection;
class kpCommandEnvironment;
class kpDocument;
class kpTextSelection;
class kpViewManager;

class kpCommand : public kpCommandSize
{
public:
    kpCommand(kpCommandEnvironment *environ);
    virtual ~kpCommand();

public:
    virtual QString name() const = 0;

    // Returns the estimated size in bytes.
    //
    // You only have to factor in the size of variables that change according
    // to the amount of input e.g. pixmap size, text size.  There is no need
    // to include the size of O(1) variables unless they are huge.
    //
    // If in doubt, return the largest possible amount of memory that your
    // command will take.  This is better than making the user unexpectedly
    // run out of memory.
    //
    // Implement this by measuring the size of all of your fields, using
    // kpCommandSize.
    virtual SizeType size() const = 0;

    virtual void execute() = 0;
    virtual void unexecute() = 0;

protected:
    kpCommandEnvironment *environ() const;

    // Commonly used accessors - simply forwards to environ().
    kpDocument *document() const;

    kpAbstractSelection *selection() const;
    kpAbstractImageSelection *imageSelection() const;
    kpTextSelection *textSelection() const;

    kpViewManager *viewManager() const;

private:
    kpCommandEnvironment *const m_environ;
};

#endif // kpCommand_H
