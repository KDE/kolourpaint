
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


#include <kpcoloreffect.h>

#include <qapplication.h>
#include <qpixmap.h>

#include <kdialog.h>
#include <klocale.h>

#include <kpdefs.h>
#include <kpdocument.h>
#include <kpmainwindow.h>
#include <kpselection.h>


kpColorEffectCommand::kpColorEffectCommand (const QString &name,
                                            bool actOnSelection,
                                            kpMainWindow *mainWindow)
    : kpCommand (mainWindow),
      m_name (name),
      m_actOnSelection (actOnSelection),
      m_oldPixmapPtr (0)
{
}

kpColorEffectCommand::~kpColorEffectCommand ()
{
    delete m_oldPixmapPtr; m_oldPixmapPtr = 0;
}


// public virtual [base kpCommand]
QString kpColorEffectCommand::name () const
{
    if (m_actOnSelection)
        return i18n ("Selection: %1").arg (m_name);
    else
        return m_name;
}


// public virtual [base kpCommand]
int kpColorEffectCommand::size () const
{
    return kpPixmapFX::pixmapSize (m_oldPixmapPtr);
}


// public virtual [base kpCommand]
void kpColorEffectCommand::execute ()
{
    kpDocument *doc = document ();
    if (!doc)
        return;

    QApplication::setOverrideCursor (Qt::waitCursor);


    const QPixmap oldPixmap = *doc->pixmap (m_actOnSelection);

    if (!isInvertible ())
    {
        m_oldPixmapPtr = new QPixmap ();
        *m_oldPixmapPtr = oldPixmap;
    }


    QPixmap newPixmap = /*pure virtual*/applyColorEffect (oldPixmap);

    doc->setPixmap (m_actOnSelection, newPixmap);


    QApplication::restoreOverrideCursor ();
}

// public virtual [base kpCommand]
void kpColorEffectCommand::unexecute ()
{
    kpDocument *doc = document ();
    if (!doc)
        return;

    QApplication::setOverrideCursor (Qt::waitCursor);


    QPixmap newPixmap;

    if (!isInvertible ())
    {
        newPixmap = *m_oldPixmapPtr;
    }
    else
    {
        newPixmap = /*pure virtual*/applyColorEffect (*doc->pixmap (m_actOnSelection));
    }

    doc->setPixmap (m_actOnSelection, newPixmap);


    delete m_oldPixmapPtr; m_oldPixmapPtr = 0;


    QApplication::restoreOverrideCursor ();
}


kpColorEffectWidget::kpColorEffectWidget (bool actOnSelection,
                                          kpMainWindow *mainWindow,
                                          QWidget *parent, const char *name)
    : QWidget (parent, name),
      m_actOnSelection (actOnSelection),
      m_mainWindow (mainWindow)
{
}

kpColorEffectWidget::~kpColorEffectWidget ()
{
}


// public
QString kpColorEffectWidget::caption () const
{
    return QString::null;
}


// protected
int kpColorEffectWidget::marginHint () const
{
    return 0;
}

// protected
int kpColorEffectWidget::spacingHint () const
{
    return KDialog::spacingHint ();
}


#include <kpcoloreffect.moc>
