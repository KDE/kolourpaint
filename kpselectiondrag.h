
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


#ifndef __kp_selection_drag_h__
#define __kp_selection_drag_h__

#include <qdragobject.h>

#include <kppixmapfx.h>
#include <kpselection.h>


class kpSelectionDrag : public QImageDrag
{
Q_OBJECT

public:
    kpSelectionDrag (QWidget *dragSource = 0, const char *name = 0);
    kpSelectionDrag (const QImage &image, QWidget *dragSource = 0, const char *name = 0);
    kpSelectionDrag (const kpSelection &sel, QWidget *dragSource = 0, const char *name = 0);
    virtual ~kpSelectionDrag ();

    static const char * const selectionMimeType;

    void setSelection (const kpSelection &sel);

protected:
    bool holdsSelection () const;

public:
    virtual const char *format (int which = 0) const;
    virtual bool provides (const char *mimeType) const;
    virtual QByteArray encodedData (const char *mimeType) const;

    static bool canDecode (const QMimeSource *e);
    static bool decode (const QMimeSource *e, QImage &img);
    static bool decode (const QMimeSource *e, kpSelection &sel,
                        const kpPixmapFX::WarnAboutLossInfo &wali =
                            kpPixmapFX::WarnAboutLossInfo ());

protected:
    kpSelection m_selection;
};


#endif  // __kp_selection_drag_h__
