
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

#ifndef __kpdocument_h__
#define __kpdocument_h__

#include <qobject.h>
#include <qstring.h>

#include <kurl.h>

class QColor;
class QPixmap;
class QPoint;
class QRect;

class kpMainWindow;

class kpDocument : public QObject
{
Q_OBJECT

public:
    kpDocument (int w, int h, int colorDepth, kpMainWindow *mainWindow);
    ~kpDocument ();


    /*
     * File I/O
     */

    void openNew (const KURL &url);
    bool open (const KURL &url, bool newDocSameNameIfNotExist = false);
    bool save ();
    bool saveAs (const KURL &url, const QString &mimetype, bool overwritePrompt = true);

    KURL url () const;

    // (will convert: empty URL --> "Untitled")
    static QString prettyURLForURL (const KURL &url);
    QString prettyURL () const;

    // (will convert: empty URL --> "Untitled")
    static QString prettyFilenameForURL (const KURL &url);
    QString prettyFilename () const;

    QString mimetype () const;


    /*
     * Properties (modified, width, height, color depth...)
     */

    void setModified (bool yes = true);
    bool isModified () const;
    bool isEmpty () const;

    int width () const;
    int oldWidth () const;  // only valid in a slot connected to sizeChanged()
    void setWidth (int w, const QColor &backgroundColor);

    int height () const;
    int oldHeight () const;  // only valid in a slot connected to sizeChanged()
    void setHeight (int h, const QColor &backgroundColor);

    QRect rect () const;

    int colorDepth () const;
    int oldColorDepth () const;  // only valid in a slot connected to colorDepthChanged()
    bool setColorDepth (int depth);


    /*
     * Pixmap access
     */

    // get a copy of a bit of the doc's pixmap
    // (not including the selection)
    QPixmap getPixmapAt (const QRect &rect) const;

    void setPixmapAt (const QPixmap &pixmap, const QPoint &at);

    void paintPixmapAt (const QPixmap &pixmap, const QPoint &at);

    // (not including the selection)
    QPixmap *pixmap () const;
    void setPixmap (const QPixmap &pixmap);
    
    // same as pixmap() but returns a _copy_ of the current pixmap
    // + any selection pasted on top
    QPixmap pixmapWithSelection () const;


    /*
     * Transformations
     */

    void fill (const QColor &color);
    void resize (int w, int h, const QColor &backgroundColor, bool fillNewAreas = true);
    bool scale (int w, int h);
    bool skew (double hangle, double vangle, const QColor &backgroundColor);  // -90 < x < 90
    bool flip (bool horz, bool vert);
    bool rotate (double angle, const QColor &backgroundColor);  // 0 <= angle < 360 (clockwise)
    static bool isLosslessRotation (double angle);
    bool convertToGrayscale ();
    bool invertColors ();


public slots:
    // these will emit signals!
    void slotContentsChanged (const QRect &rect);
    void slotSizeChanged (int newWidth, int newHeight);

signals:
    void documentOpened ();
    void documentSaved ();

    // this is the _only_ signal that may be emitted in addition to the others
    void documentModified ();

    void contentsChanged (const QRect &rect);
    void sizeChanged (int newWidth, int newHeight);  // see oldWidth(), oldHeight()
    void colorDepthChanged (int newDepth);  // see oldColorDepth()

private:
    QPixmap *m_pixmap;
    int m_oldWidth, m_oldHeight;
    int m_colorDepth, m_oldColorDepth;
    kpMainWindow *m_mainWindow;
    KURL m_url;
    QString m_mimetype;
    bool m_modified;
};

#endif  // __kpdocument_h__
