
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


#ifndef KP_COLOR_EFFECT_H
#define KP_COLOR_EFFECT_H

#include <qstring.h>
#include <qwidget.h>

#include <kcommand.h>

class QPixmap;

class kpDocument;
class kpMainWindow;

class kpColorEffectCommand : public KCommand
{
public:
    kpColorEffectCommand (const QString &name,
                          bool actOnSelection,
                          kpMainWindow *mainWindow);
    virtual QString name () const;
    virtual ~kpColorEffectCommand ();

private:
    kpDocument *document () const;

public:
    virtual void execute ();
    virtual void unexecute ();

protected:
    virtual QPixmap applyColorEffect (const QPixmap &pixmap) = 0;

private:
    QString m_name;
    bool m_actOnSelection;
    kpMainWindow *m_mainWindow;

    QPixmap *m_oldPixmapPtr;
};


class kpColorEffectWidget : public QWidget
{
Q_OBJECT

public:
    kpColorEffectWidget (QWidget *parent, const char *name = 0);
    virtual ~kpColorEffectWidget ();

signals:
    void settingsChanged ();

public:
    virtual bool isNoOp () const = 0;
    virtual QPixmap applyColorEffect (const QPixmap &pixmap) = 0;

    virtual kpColorEffectCommand *createCommand (bool actOnSelection,
                                                 kpMainWindow *mainWindow) const = 0;

protected:
    int marginHint () const;
    int spacingHint () const;
};


#endif  // KP_COLOR_EFFECT_H
