
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef kpToolTextPrivate_H
#define kpToolTextPrivate_H

class kpToolTextInsertCommand;
class kpToolTextEnterCommand;
class kpToolTextBackspaceCommand;
class kpToolTextDeleteCommand;

struct kpToolTextPrivate {
    kpToolTextInsertCommand *insertCommand;
    kpToolTextEnterCommand *enterCommand;
    kpToolTextBackspaceCommand *backspaceCommand, *backspaceWordCommand;
    kpToolTextDeleteCommand *deleteCommand, *deleteWordCommand;
};

#endif // kpToolTextPrivate_H
