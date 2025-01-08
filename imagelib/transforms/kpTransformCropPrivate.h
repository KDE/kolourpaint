
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef kpTransformCropPrivate_H
#define kpTransformCropPrivate_H

class QString;

class kpCommand;
class kpMainWindow;

// Adds a kpMacroCommand, with name <commandName>, to the command history.
//
// The first subcommand of this kpMacroCommand should be <resizeDocCommand>
// which resizes the document to the size of the selection.
void kpTransformCrop_TextSelection(kpMainWindow *mainWindow, const QString &commandName, kpCommand *resizeDocCommand);
void kpTransformCrop_ImageSelection(kpMainWindow *mainWindow, const QString &commandName, kpCommand *resizeDocCommand);

#endif // kpTransformCropPrivate_H
