
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


#ifndef KP_COLOR_EFFECT_H
#define KP_COLOR_EFFECT_H

#include <qstring.h>
#include <qwidget.h>

#include <kpcommandhistory.h>

class QPixmap;

class kpDocument;
class kpMainWindow;


class kpColorEffectCommand : public kpCommand
{
public:
    kpColorEffectCommand (const QString &name,
                          bool actOnSelection,
                          kpMainWindow *mainWindow);
    virtual ~kpColorEffectCommand ();

    virtual QString name () const;
    virtual int size () const;

public:
    virtual void execute ();
    virtual void unexecute ();

public:
    // Return true if applyColorEffect(applyColorEffect(pixmap)) == pixmap
    // to avoid storing the old pixmap, saving memory.
    virtual bool isInvertible () const { return false; }

protected:
    virtual QPixmap applyColorEffect (const QPixmap &pixmap) = 0;

private:
    QString m_name;
    bool m_actOnSelection;

    QPixmap *m_oldPixmapPtr;
};


class kpColorEffectWidget : public QWidget
{
Q_OBJECT

public:
    kpColorEffectWidget (bool actOnSelection,
                         kpMainWindow *mainWindow,
                         QWidget *parent, const char *name = 0);
    virtual ~kpColorEffectWidget ();

signals:
    void settingsChangedNoWaitCursor ();
    
    void settingsChanged ();

    // (same as settingsChanged() but preview doesn't update until there
    //  has been no activity for a while - used for sliders in slow effects)
    void settingsChangedDelayed ();

public:
    virtual QString caption () const;

    virtual bool isNoOp () const = 0;
    virtual QPixmap applyColorEffect (const QPixmap &pixmap) = 0;

    virtual kpColorEffectCommand *createCommand () const = 0;

protected:
    int marginHint () const;
    int spacingHint () const;

protected:
    bool m_actOnSelection;
    kpMainWindow *m_mainWindow;
};


#endif  // KP_COLOR_EFFECT_H
