
/*
   Copyright (c) 2003,2004,2005 Clarence Dang <dang@kde.org>
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


#ifndef __kp_defs_h__
#define __kp_defs_h__


#include <limits.h>

#include <qglobal.h>
#include <qpoint.h>
#include <qsize.h>
#include <qstring.h>

#include <kdeversion.h>


#define KP_IS_QT_3_3 (QT_VERSION >= 0x030300 && 1)
#define KP_IS_KDE_3_3 ((KDE_VERSION_MAJOR >= 3 && KDE_VERSION_MINOR >= 3) && 1)


// approx. 2896x2896x32bpp or 3344x3344x24bpp (TODO: 24==32?) or 4096*4096x16bpp
#define KP_BIG_IMAGE_SIZE (32 * 1048576)


#define KP_PI 3.141592653589793238462


#define KP_DEGREES_TO_RADIANS(deg) ((deg) * KP_PI / 180.0)
#define KP_RADIANS_TO_DEGREES(rad) ((rad) * 180.0 / KP_PI)


#define KP_INVALID_POINT QPoint (INT_MIN / 8, INT_MIN / 8)
#define KP_INVALID_WIDTH (INT_MIN / 8)
#define KP_INVALID_HEIGHT (INT_MIN / 8)
#define KP_INVALID_SIZE QSize (INT_MIN / 8, INT_MIN / 8)


//
// Settings
//

#define kpSettingsGroupGeneral QString::fromLatin1 ("General Settings")
#define kpSettingFirstTime QString::fromLatin1 ("First Time")
#define kpSettingShowGrid QString::fromLatin1 ("Show Grid")
#define kpSettingShowPath QString::fromLatin1 ("Show Path")
#define kpSettingColorSimilarity QString::fromLatin1 ("Color Similarity")
#define kpSettingDitherOnOpen QString::fromLatin1 ("Dither on Open if Screen is 15/16bpp and Image Num Colors More Than")
#define kpSettingPrintImageCenteredOnPage QString::fromLatin1 ("Print Image Centered On Page")

#define kpSettingsGroupFileSaveAs QString::fromLatin1 ("File/Save As")
#define kpSettingsGroupFileExport QString::fromLatin1 ("File/Export")
#define kpSettingsGroupEditCopyTo QString::fromLatin1 ("Edit/Copy To")

#define kpSettingForcedMimeType QString::fromLatin1 ("Forced MimeType")
#define kpSettingForcedColorDepth QString::fromLatin1 ("Forced Color Depth")
#define kpSettingForcedDither QString::fromLatin1 ("Forced Dither")
#define kpSettingForcedQuality QString::fromLatin1 ("Forced Quality")

#define kpSettingLastDocSize QString::fromLatin1 ("Last Document Size")

#define kpSettingMoreEffectsLastEffect QString::fromLatin1 ("More Effects - Last Effect")

#define kpSettingResizeScaleLastKeepAspect QString::fromLatin1 ("Resize Scale - Last Keep Aspect")


#define kpSettingsGroupMimeTypeProperties QString::fromLatin1 ("MimeType Properties Version 1.2-2")
#define kpSettingMimeTypeMaximumColorDepth QString::fromLatin1 ("Maximum Color Depth")
#define kpSettingMimeTypeHasConfigurableColorDepth QString::fromLatin1 ("Configurable Color Depth")
#define kpSettingMimeTypeHasConfigurableQuality QString::fromLatin1 ("Configurable Quality Setting")


#define kpSettingsGroupUndoRedo QString::fromLatin1 ("Undo/Redo Settings")
#define kpSettingUndoMinLimit QString::fromLatin1 ("Min Limit")
#define kpSettingUndoMaxLimit QString::fromLatin1 ("Max Limit")
#define kpSettingUndoMaxLimitSizeLimit QString::fromLatin1 ("Max Limit Size Limit")


#define kpSettingsGroupThumbnail QString::fromLatin1 ("Thumbnail Settings")
#define kpSettingThumbnailShown QString::fromLatin1 ("Shown")
#define kpSettingThumbnailGeometry QString::fromLatin1 ("Geometry")
#define kpSettingThumbnailZoomed QString::fromLatin1 ("Zoomed")
#define kpSettingThumbnailShowRectangle QString::fromLatin1 ("ShowRectangle")


#define kpSettingsGroupPreviewSave QString::fromLatin1 ("Save Preview Settings")
#define kpSettingPreviewSaveGeometry QString::fromLatin1 ("Geometry")
#define kpSettingPreviewSaveUpdateDelay QString::fromLatin1 ("Update Delay")


#define kpSettingsGroupTools QString::fromLatin1 ("Tool Settings")
#define kpSettingLastTool QString::fromLatin1 ("Last Used Tool")
#define kpSettingToolBoxIconSize QString::fromLatin1 ("Tool Box Icon Size")


#define kpSettingsGroupText QString::fromLatin1 ("Text Settings")
#define kpSettingFontFamily QString::fromLatin1 ("Font Family")
#define kpSettingFontSize QString::fromLatin1 ("Font Size")
#define kpSettingBold QString::fromLatin1 ("Bold")
#define kpSettingItalic QString::fromLatin1 ("Italic")
#define kpSettingUnderline QString::fromLatin1 ("Underline")
#define kpSettingStrikeThru QString::fromLatin1 ("Strike Thru")


#define kpSettingsGroupFlattenEffect QString::fromLatin1 ("Flatten Effect Settings")
#define kpSettingFlattenEffectColor1 QString::fromLatin1 ("Color1")
#define kpSettingFlattenEffectColor2 QString::fromLatin1 ("Color2")


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


#endif  // __kp_defs_h__

