
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

#include <qpixmap.h>

#include <klocale.h>

#include <kpdefs.h>
#include <kpdocument.h>
#include <kptoolclear.h>

kpToolClearCommand::kpToolClearCommand (kpDocument *document, kpViewManager *viewManager,
                                        const QColor &color)
    : m_document (document), m_viewManager (viewManager),
      m_newColor (color),
      m_oldPixmapPtr (0)
{
}

// virtual
QString kpToolClearCommand::name () const
{
    return i18n ("Clear");
}

kpToolClearCommand::~kpToolClearCommand ()
{
    delete m_oldPixmapPtr;
}

// virtual
void kpToolClearCommand::execute ()
{
    m_oldPixmapPtr = new QPixmap ();
    *m_oldPixmapPtr = *m_document->pixmap ();

    m_document->fill (m_newColor);
}

// virtual
void kpToolClearCommand::unexecute ()
{
    m_document->setPixmapAt (*m_oldPixmapPtr, QPoint (0, 0));

    delete m_oldPixmapPtr;
    m_oldPixmapPtr = 0;
}

#include <kptoolclear.moc>
