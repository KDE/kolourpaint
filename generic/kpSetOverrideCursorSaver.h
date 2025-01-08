
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
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
    kpSetOverrideCursorSaver(const QCursor &cursor);
    ~kpSetOverrideCursorSaver();
};

#endif // kpSetOverrideCursorSaver_H
