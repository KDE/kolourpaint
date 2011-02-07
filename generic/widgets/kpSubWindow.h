
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
    kpSubWindow(QWidget *parent);
};


#endif  // kpSubWindow_H
