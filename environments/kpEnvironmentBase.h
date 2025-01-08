
// LOREFACTOR: The files in this folder duplicate too much code.
//             Use mixin multiple inheritance to get around this.
//
// LOREFACTOR: Move as much kpMainWindow code as possible into these classes
//             to reduce the size of kpMainWindow.  But do not split concerns /
//             "aspects" in kpMainWindow, with half the code there and half here,
//             as that would be confusing.

/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef kpEnvironmentBase_H
#define kpEnvironmentBase_H

#include <QObject>

class kpAbstractImageSelection;
class kpAbstractSelection;
class kpColor;
class kpCommandEnvironment;
class kpDocument;
class kpMainWindow;
class kpTextSelection;
class kpViewManager;

// Abstract facade bridging kpMainWindow and other suppliers (e.g. kpTool,
// kpToolToolBar, kpColorToolBar) to clients.
//
// This decouples as many classes as possible from clients, for maintainability.
// If adding new methods to this class, it is preferable to expose additional
// functionality, rather than expose additional classes.
//
// This class also avoids cramming excessive functionality into kpMainWindow.
//
// If this interface gets too bloated, push down the specialized methods into
// a new class.
class kpEnvironmentBase : public QObject
{
    Q_OBJECT

    // (must derive from)
protected:
    // Note: Our interface must never publicly leak <mainWindow> or any other
    //       classes we are trying to hide as that would defeat the point of
    //       the facade.
    kpEnvironmentBase(kpMainWindow *mainWindow);
    ~kpEnvironmentBase() override;

public:
    kpDocument *document() const;

    kpAbstractSelection *selection() const;
    kpAbstractImageSelection *imageSelection() const;
    kpTextSelection *textSelection() const;

    kpViewManager *viewManager() const;

    kpCommandEnvironment *commandEnvironment() const;

    kpColor backgroundColor(bool ofSelection = false) const;

protected:
    kpMainWindow *mainWindow() const;

private:
    struct kpEnvironmentBasePrivate *const d;
};

#endif // kpEnvironmentBase_H
