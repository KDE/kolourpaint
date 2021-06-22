
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


#ifndef KP_DEFS_H
#define KP_DEFS_H


#include <limits.h>

#include <QtGlobal>
#include <QPoint>
#include <QSize>
#include <QString>


// approx. 2896x2896x32bpp or 3344x3344x24bpp (TODO: 24==32?) or 4096*4096x16bpp
#define KP_BIG_IMAGE_SIZE (32 * 1048576)


#define KP_INVALID_POINT QPoint (INT_MIN / 8, INT_MIN / 8)
#define KP_INVALID_WIDTH (INT_MIN / 8)
#define KP_INVALID_HEIGHT (INT_MIN / 8)
#define KP_INVALID_SIZE QSize (INT_MIN / 8, INT_MIN / 8)


#define KP_INCHES_PER_METER (100 / 2.54)
#define KP_MILLIMETERS_PER_INCH 25.4


//
// Settings
//

#define kpSettingsGroupRecentFiles "Recent Files"

#define kpSettingsGroupGeneral "General Settings"
#define kpSettingFirstTime "First Time"
#define kpSettingShowGrid "Show Grid"
#define kpSettingShowPath "Show Path"
#define kpSettingDrawAntiAliased "Draw AntiAliased"
#define kpSettingColorSimilarity "Color Similarity"
#define kpSettingDitherOnOpen "Dither on Open if Screen is 15/16bpp and Image Num Colors More Than"
#define kpSettingPrintImageCenteredOnPage "Print Image Centered On Page"
#define kpSettingOpenImagesInSameWindow "Open Images in the Same Window"

#define kpSettingsGroupFileSaveAs "File/Save As"
#define kpSettingsGroupFileExport "File/Export"
#define kpSettingsGroupEditCopyTo "Edit/Copy To"

#define kpSettingForcedMimeType "Forced MimeType"
#define kpSettingForcedColorDepth "Forced Color Depth"
#define kpSettingForcedDither "Forced Dither"
#define kpSettingForcedQuality "Forced Quality"

#define kpSettingLastDocSize "Last Document Size"

#define kpSettingMoreEffectsLastEffect "More Effects - Last Effect"

#define kpSettingsGroupUndoRedo "Undo/Redo Settings"
#define kpSettingUndoMinLimit "Min Limit"
#define kpSettingUndoMaxLimit "Max Limit"
#define kpSettingUndoMaxLimitSizeLimit "Max Limit Size Limit"


#define kpSettingsGroupThumbnail "Thumbnail Settings"
#define kpSettingThumbnailShown "Shown"
#define kpSettingThumbnailGeometry "Geometry"
#define kpSettingThumbnailZoomed "Zoomed"
#define kpSettingThumbnailShowRectangle "ShowRectangle"


#define kpSettingsGroupPreviewSave "Save Preview Settings"
#define kpSettingPreviewSaveGeometry "Geometry"
#define kpSettingPreviewSaveUpdateDelay "Update Delay"


#define kpSettingsGroupTools "Tool Settings"
#define kpSettingLastTool "Last Used Tool"
#define kpSettingToolBoxIconSize "Tool Box Icon Size"


#define kpSettingsGroupText "Text Settings"
#define kpSettingFontFamily "Font Family"
#define kpSettingFontSize "Font Size"
#define kpSettingBold "Bold"
#define kpSettingItalic "Italic"
#define kpSettingUnderline "Underline"
#define kpSettingStrikeThru "Strike Thru"


#define kpSettingsGroupFlattenEffect "Flatten Effect Settings"
#define kpSettingFlattenEffectColor1 "Color1"
#define kpSettingFlattenEffectColor2 "Color2"


//
// Session Restore Setting
//

// URL of the document in the main window.
//
// This key only exists if the document does.  If it exists, it can be empty.
// The URL need not point to a file that exists e.g. "kolourpaint doesnotexist.png".
#define kpSessionSettingDocumentUrl QString::fromLatin1 ("Session Document Url")

// The size of a document which is not from a URL e.g. "kolourpaint doesnotexist.png".
// This key does not exist for documents from URLs.
#define kpSessionSettingNotFromUrlDocumentSize QString::fromLatin1 ("Session Not-From-Url Document Size")


#endif  // KP_DEFS_H


