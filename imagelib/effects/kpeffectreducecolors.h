
/*
   Copyright (c) 2003-2006 Clarence Dang <dang@kde.org>
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


#ifndef KP_EFFECT_REDUCE_COLORS_H
#define KP_EFFECT_REDUCE_COLORS_H


#include <kdemacros.h>

#include <kpEffectCommandBase.h>
#include <kpimage.h>


class QImage;
class QPixmap;
class QRadioButton;

class kpMainWindow;


class kpEffectReduceColors
{
public:
    // TODO: why isn't applyEffect() for the public API sufficient?  Ans: see TODO in kpdocument.cpp.
    static QImage convertImageDepth (const QImage &image, int depth, bool dither);

    // (always preserves mask)
    static void applyEffect (QPixmap *destPixmapPtr, int depth, bool dither);
    static QPixmap applyEffect (const QPixmap &pm, int depth, bool dither);
};



class kpEffectReduceColorsCommand : public kpEffectCommandBase
{
public:
    // depth must be 1 or 8
    kpEffectReduceColorsCommand (int depth, bool dither,
                                 bool actOnSelection,
                                 kpMainWindow *mainWindow);
    virtual ~kpEffectReduceColorsCommand ();

    QString commandName (int depth, int dither) const;

    //
    // kpEffectCommandBase interface
    //

protected:
    virtual kpImage applyEffect (const kpImage &image);

    int m_depth;
    bool m_dither;
};


#endif  // KP_EFFECT_REDUCE_COLORS_H
