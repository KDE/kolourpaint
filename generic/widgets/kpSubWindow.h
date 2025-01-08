
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef kpSubWindow_H
#define kpSubWindow_H

#include <QDialog>

//
// A tool window with the following properties, in order of importance:
//
// 1. Stays on top of its parent window, but not on top of any other
//    window (this rules out Qt::WindowStaysOnTop)
// 2. Does not auto-hide when its parent window loses focus
//    (this rules out Qt::Tool).
// 3. Does not have a taskbar entry.
// 4. TODO: Does not take keyboard focus away from the parent window,
//          when it is shown.
// 5. TODO: Is not in the Alt+Tab list
//
// We mean "tool window" in the window system sense.  It has nothing to do
// with kpTool so to avoid confusion, we do not name it "kpToolWindow".
//
class kpSubWindow : public QDialog
{
public:
    explicit kpSubWindow(QWidget *parent);
};

#endif // kpSubWindow_H
