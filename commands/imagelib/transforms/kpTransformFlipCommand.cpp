
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


#include "kpTransformFlipCommand.h"

#include <QApplication>

#include "kpLogCategories.h"
#include "layers/selections/image/kpAbstractImageSelection.h"
#include "environments/commands/kpCommandEnvironment.h"
#include "kpDefs.h"
#include "document/kpDocument.h"
#include "pixmapfx/kpPixmapFX.h"

#include <KLocalizedString>

//---------------------------------------------------------------------

kpTransformFlipCommand::kpTransformFlipCommand (bool actOnSelection,
        bool horiz, bool vert,
        kpCommandEnvironment *environ)
    : kpCommand (environ),
      m_actOnSelection (actOnSelection),
      m_horiz (horiz), m_vert (vert)
{
}

//---------------------------------------------------------------------

kpTransformFlipCommand::~kpTransformFlipCommand () = default;

//---------------------------------------------------------------------
// public virtual [base kpCommand]

QString kpTransformFlipCommand::name () const
{
    QString opName;


#if 1
    opName = i18n ("Flip");
#else  // re-enable when giving full descriptions for all actions
    if (m_horiz && m_vert)
        opName = i18n ("Flip horizontally and vertically");
    else if (m_horiz)
        opName = i18n ("Flip horizontally");
    else if (m_vert)
        opName = i18n ("Flip vertically");
    else
    {
        qCCritical(kpLogCommands) << "kpTransformFlipCommand::name() not asked to flip";
        return {};
    }
#endif


    if (m_actOnSelection) {
        return i18n ("Selection: %1", opName);
    }

    return opName;
}

//---------------------------------------------------------------------
// public virtual [base kpCommand]

kpCommandSize::SizeType kpTransformFlipCommand::size () const
{
    return 0;
}

//---------------------------------------------------------------------
// public virtual [base kpCommand]

void kpTransformFlipCommand::execute ()
{
    flip ();
}

//---------------------------------------------------------------------
// public virtual [base kpCommand]

void kpTransformFlipCommand::unexecute ()
{
    flip ();
}

//---------------------------------------------------------------------
// private

void kpTransformFlipCommand::flip ()
{
    kpDocument *doc = document ();
    Q_ASSERT (doc);

    QApplication::setOverrideCursor (Qt::WaitCursor);

    if (m_actOnSelection)
    {
        Q_ASSERT (doc->imageSelection ());
        doc->imageSelection ()->flip (m_horiz, m_vert);
        environ ()->somethingBelowTheCursorChanged ();
    }
    else
    {
        doc->setImage(doc->image().mirrored(m_horiz, m_vert));
    }

    QApplication::restoreOverrideCursor ();
}
