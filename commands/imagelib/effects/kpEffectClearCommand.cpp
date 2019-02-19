
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


#include "kpEffectClearCommand.h"

#include "layers/selections/image/kpAbstractImageSelection.h"
#include "kpDefs.h"
#include "document/kpDocument.h"

#include <KLocalizedString>

//--------------------------------------------------------------------------------

kpEffectClearCommand::kpEffectClearCommand (bool actOnSelection,
        const kpColor &newColor,
        kpCommandEnvironment *environ)
    : kpCommand (environ),
      m_actOnSelection (actOnSelection),
      m_newColor (newColor),
      m_oldImagePtr (nullptr)
{
}

kpEffectClearCommand::~kpEffectClearCommand ()
{
    delete m_oldImagePtr;
}


// public virtual [base kpCommand]
QString kpEffectClearCommand::name () const
{
    QString opName = i18n ("Clear");

    return (m_actOnSelection) ? i18n ("Selection: %1", opName) : opName;
}


// public virtual [base kpCommand]
kpCommandSize::SizeType kpEffectClearCommand::size () const
{
    return ImageSize (m_oldImagePtr);
}


// public virtual [base kpCommand]
void kpEffectClearCommand::execute ()
{
    kpDocument *doc = document ();
    Q_ASSERT (doc);


    m_oldImagePtr = new kpImage ();
    *m_oldImagePtr = doc->image (m_actOnSelection);


    // REFACTOR: Would like to derive entire class from kpEffectCommandBase but
    //           this code makes it difficult since it's not just acting on pixels
    //           (kpAbstractImageSelection::fill() takes into account the shape of a selection).
    if (m_actOnSelection)
    {
        // OPT: could just edit pixmap directly and signal change
        kpAbstractImageSelection *sel = doc->imageSelection ();
        Q_ASSERT (sel);
        sel->fill (m_newColor);
    }
    else {
        doc->fill (m_newColor);
    }
}

// public virtual [base kpCommand]
void kpEffectClearCommand::unexecute ()
{
    kpDocument *doc = document ();
    Q_ASSERT (doc);


    doc->setImage (m_actOnSelection, *m_oldImagePtr);


    delete m_oldImagePtr;
    m_oldImagePtr = nullptr;
}

