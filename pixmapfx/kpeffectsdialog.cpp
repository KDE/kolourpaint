
/*
   Copyright (c) 2003-2004 Clarence Dang <dang@kde.org>
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


#include <kpeffectsdialog.h>

#include <klocale.h>

#include <kpdocument.h>
#include <kppixmapfx.h>


kpEffectsDialog::kpEffectsDialog (bool actOnSelection,
                                  kpMainWindow *parent,
                                  const char *name)
    : kpToolPreviewDialog (kpToolPreviewDialog::Preview,
                           QString::null/*actionName*/,
                           actOnSelection,
                           parent,
                           name)
{
    if (actOnSelection)
        setCaption (i18n ("More Image Effects (Selection)"));
    else
        setCaption (i18n ("More Image Effects"));
}

kpEffectsDialog::~kpEffectsDialog ()
{
}


// public virtual [base kpToolPreviewDialog]
bool kpEffectsDialog::isNoOp () const
{
    return true;  // TODO
}


// protected virtual [base kpToolPreviewDialog]
QSize kpEffectsDialog::newDimensions () const
{
    kpDocument *doc = document ();
    if (!doc)
        return QSize ();

    return QSize (doc->width (m_actOnSelection),
                  doc->height (m_actOnSelection));
}

// protected virtual [base kpToolPreviewDialog]
QPixmap kpEffectsDialog::transformPixmap (const QPixmap &pixmap,
                                               int targetWidth, int targetHeight) const
{
    QPixmap pixmapWithEffect = pixmap;
    return kpPixmapFX::scale (pixmapWithEffect, targetWidth, targetHeight);
}


#include <kpeffectsdialog.moc>
