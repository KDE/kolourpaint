
/* This file is part of the KolourPaint project
   Copyright (c) 2003 Clarence Dang <dang@kde.org>
   All rights reserved.
   
   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:
   
   1. Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
   2. Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
   3. Neither the names of the copyright holders nor the names of
      contributors may be used to endorse or promote products derived from
      this software without specific prior written permission.
   
   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
   PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
   HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef __kpdefs_h__
#define __kpdefs_h__

class kpViewManager;
class QRect;


#define KP_AREA 30509  // TODO: get own area

#define KP_BLANK_DOCUMENT_COLOR Qt::white

#define KP_DOCUMENT_MAX_WIDTH 4000
#define KP_DOCUMENT_MAX_HEIGHT 4000

#define KP_PI 3.141592653589793238462
#define KP_EPSILON 0.0001


#define KP_DEGREES_TO_RADIANS(deg) ((deg) * KP_PI / 180.0)
#define KP_RADIANS_TO_DEGREES(rad) ((rad) * 180.0 / KP_PI)

// settings
#define kpSettingsGroupGeneral QString ("General")
#define kpSettingFirstTime QString ("First Time")
#define kpSettingShowGrid QString ("Show Grid")
#define kpSettingShowPath QString ("Show Path")
#define kpSettingDefaultOutputMimetype QString ("Default Output Mimetype")

#endif  // __kpdefs_h__

