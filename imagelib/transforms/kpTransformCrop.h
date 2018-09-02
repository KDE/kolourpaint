
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


#ifndef KP_TRANSFORM_CROP_H
#define KP_TRANSFORM_CROP_H


class kpMainWindow;


//
// ASSUMPTION: There is a current selection.
//
//
// In all cases, the document is resized to be the same size as the bounding
// rectangle of the selection.  Additional behavior depends on the type of
// selection:
//
//
// If it's a text box:
//
// 1. It's moved to (0, 0) and kept editable.
//
// 2. The document background always becomes completely transparent.
//
//    Text boxes with transparent backgrounds, before calling this method,
//    antialias their text with the pixels of the document below.  Such
//    pixels are unlikely to be all of the same color, so there is no single
//    "correct" color for the new document background.  We choose transparent
//    because it's the most neutral and forces the text to not antialias.
//    TODO: Perhaps a better approach would have been to simply copy the
//          pixels of the document below the text box to (0, 0)?
//
//    For text boxes with opaque backgrounds, the new transparent document
//    background means that the extents of text boxes are clear, when the
//    boxes are moved around -- this is handy.
//
//
// If it's an image selection:
//
// 1. The pixels of the document starting from position (0, 0) are set the
//    same as those inside the selection region.  Unlike other image selection
//    commands, if the selection is not floating, there is still no pulling
//    of the selection from the document.
//
//    The pixels outside the selection region are set to the background color.
//
// 2. The selection border is discarded -- even if the selection was floating
//    before -- and replaced by a new one, of the same shape, but located at (0, 0).
//    This allows the user to pull off a selection, if they would like.
//
//    For user convenience, this border is created by a undoable
//    create-selection-border command added to the undo history.
//
void kpTransformCrop (kpMainWindow *mainWindow);


#endif  // KP_TRANSFORM_CROP_H
