
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


#define DEBUG_KP_RESIZE_SIGNALLING_LABEL 0


#include "generic/widgets/kpResizeSignallingLabel.h"


#include "kpLogCategories.h"


kpResizeSignallingLabel::kpResizeSignallingLabel (const QString &string,
                                                  QWidget *parent )
    : QLabel (string, parent)
{
}

kpResizeSignallingLabel::kpResizeSignallingLabel (QWidget *parent )
    : QLabel (parent)
{
}

kpResizeSignallingLabel::~kpResizeSignallingLabel () = default;


// protected virtual [base QLabel]
void kpResizeSignallingLabel::resizeEvent (QResizeEvent *e)
{
#if DEBUG_KP_RESIZE_SIGNALLING_LABEL
    qCDebug(kpLogMisc) << "kpResizeSignallingLabel::resizeEvent() newSize=" << e->size ()
               << " oldSize=" << e->oldSize ();
#endif
    QLabel::resizeEvent (e);

    emit resized ();
}

#include "moc_kpResizeSignallingLabel.cpp"
