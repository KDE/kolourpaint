/*
   Copyright(c) 2003-2007 Clarence Dang <dang@kde.org>
   Copyright(c) 2011 Martin Koller <kollix@aon.at>
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
   INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES(INCLUDING, BUT
   NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
   THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "tools/kpToolAction.h"
#include <KActionCollection>
#include "tools/kpTool.h"


//---------------------------------------------------------------------

kpToolAction::kpToolAction(const QString &text,
                            const QString &pic, const QList<QKeySequence> &shortcut,
                            const QObject *receiver, const char *slot,
                            KActionCollection *ac, const QString &name)
    : KToggleAction(QIcon::fromTheme(pic), text, ac)
{
  ac->setDefaultShortcuts(this, shortcut);


  if ( receiver && slot ) {
      connect (this, SIGNAL(triggered(bool)), receiver, slot);
  }

  updateToolTip();
  connect (this, &kpToolAction::changed, this, &kpToolAction::updateToolTip);

  ac->addAction(name, this);
}

//---------------------------------------------------------------------

// protected
void kpToolAction::updateToolTip()
{
  const QString newToolTip =
      kpTool::toolTipForTextAndShortcut(text(), shortcuts()).remove(QLatin1Char('&'));

  if ( newToolTip == toolTip() ) {
    return;
  }

  setToolTip(newToolTip);
}

//---------------------------------------------------------------------

