
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


#ifndef __kptool_skew_h__
#define __kptool_skew_h__

#include <qpixmap.h>

#include <kpcommandhistory.h>
#include <kdialogbase.h>

#include <kpcolor.h>
#include <kpselection.h>
#include <kptoolpreviewdialog.h>

class QGroupBox;
class QLabel;
class QPixmap;

class KIntNumInput;

class kpDocument;
class kpMainWindow;


class kpToolSkewCommand : public kpCommand
{
public:
    kpToolSkewCommand (bool actOnSelection,
                       int hangle, int vangle,
                       kpMainWindow *mainWindow);
    virtual ~kpToolSkewCommand ();

    virtual QString name () const;

    virtual int size () const;

    virtual void execute ();
    virtual void unexecute ();

private:
    bool m_actOnSelection;
    int m_hangle, m_vangle;

    kpColor m_backgroundColor;
    QPixmap *m_oldPixmapPtr;
    kpSelection m_oldSelection;
};


class kpToolSkewDialog : public kpToolPreviewDialog
{
Q_OBJECT

public:
    kpToolSkewDialog (bool actOnSelection, kpMainWindow *parent,
                      const char *name = 0);
    virtual ~kpToolSkewDialog ();

private:
    static int s_lastWidth, s_lastHeight;
    static int s_lastHorizontalAngle, s_lastVerticalAngle;

    void createAngleGroupBox ();

    virtual QSize newDimensions () const;
    virtual QPixmap transformPixmap (const QPixmap &pixmap,
                                     int targetWidth, int targetHeight) const;

    void updateLastAngles ();

private slots:
    virtual void slotUpdate ();

public:
    // These are the angles the users sees in the dialog and...
    int horizontalAngle () const;
    int verticalAngle () const;

    // ...these functions translate them for use in kpPixmapFX::skew().
    static int horizontalAngleForPixmapFX (int hangle);
    static int verticalAngleForPixmapFX (int vangle);

    int horizontalAngleForPixmapFX () const;
    int verticalAngleForPixmapFX () const;

    virtual bool isNoOp () const;

private slots:
    virtual void slotOk ();

private:
    KIntNumInput *m_horizontalSkewInput, *m_verticalSkewInput;
};

#endif  // __kptool_skew_h__
