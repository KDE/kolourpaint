
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


#ifndef kpSetOverrideCursorSaver_H
#define kpSetOverrideCursorSaver_H


class QCursor;


//
// A less error-prone way of setting the override cursor, compared to
// the QApplication::{set,restore}OverrideCursor() pair.
//
// To use this class, allocate it on the stack with the desired cursor.
//
// This class sets the application's override cursor to <cursor> on
// construction.  It restores the cursor on destruction.
//
// Example Usage 1 - Cursor active during the entire method:
//
//     void method ()
//     {
//         kpSetOverrideCursorSaver cursorSaver (Qt::WaitCursor);
//
//         <potentially time-consuming operation>
//
//     }   // Stack unwinds, calling cursorSaver's destructor,
//         // which restores the cursor
//
// Example Usage 2 - Cursor active during part of the method:
//
//     void method ()
//     {
//         <some code>
//
//         {
//             kpSetOverrideCursorSaver cursorSaver (Qt::WaitCursor);
//
//             <potentially time-consuming operation>
//
//         }   // Stack unwinds, calling cursorSaver's destructor,
//             // which restores the cursor
//
//         // (cursor is restored before this code executes)
//         <more code>
//     }
//
// Note that the kpSetOverrideCursorSaver must be defined as a local
// variable inside -- not outside --- the braces containing the code the
// cursor can be active over, else the destruction and cursor restoration
// might not occur at the right time.  In other words, the following usage
// is wrong:
//
// INCORRECT Example Usage 2:
//
//     void method ()
//     {
//         <some code>
//
//         kpSetOverrideCursorSaver cursorSaver (Qt::WaitCursor);
//         // BUG: These braces do nothing to limit the effect of cursorSaver.
//         {
//             <potentially time-consuming operation>
//         }
//
//         // BUG: cursorSaver has not yet restored the cursor before this
//         //      code executes.
//         <more code>
//
//     }   // Stack unwinds, calling cursorSaver's destructor,
//         // which restores the cursor
//
class kpSetOverrideCursorSaver
{
public:
    kpSetOverrideCursorSaver (const QCursor &cursor);
    ~kpSetOverrideCursorSaver ();
};


#endif  // kpSetOverrideCursorSaver_H
