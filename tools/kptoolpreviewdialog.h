
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

#ifndef __kp_tool_preview_dialog_h__
#define __kp_tool_preview_dialog_h__


#include <qpixmap.h>

#include <kdialogbase.h>


class QLabel;
class QGridLayout;
class QGroupBox;

class kpDocument;
class kpMainWindow;
class kpResizeSignallingLabel;


class kpToolPreviewDialog : public KDialogBase
{
Q_OBJECT

public:
    enum Features
    {
        Dimensions = 1, Preview = 2,
        AllFeatures = Dimensions | Preview
    };

    // You must call slotUpdate() in your constructor
    kpToolPreviewDialog (Features features,
                         bool reserveTopRow,
                         // e.g. "Skew (Image|Selection)"
                         const QString &caption,
                         // (in the Dimensions Group Box) e.g. "After Skew:"
                         const QString &afterActionText,
                         bool actOnSelection,
                         kpMainWindow *parent,
                         const char *name = 0);
    virtual ~kpToolPreviewDialog ();

private:
    void createDimensionsGroupBox ();
    void createPreviewGroupBox ();

public:
    virtual bool isNoOp () const = 0;

protected:
    kpDocument *document () const;

    // All widgets must have mainWidget() as their parent
    void addCustomWidgetToFront (QWidget *w);  // see <reserveTopRow> in ctor
    void addCustomWidget (QWidget *w);
    void addCustomWidgetToBack (QWidget *w)
    {
        addCustomWidget (w);
    }

    virtual QSize newDimensions () const = 0;
    virtual QPixmap transformPixmap (const QPixmap &pixmap,
                                     int targetWidth, int targetHeight) const = 0;

private:
    void updateDimensions ();

public:
    static double aspectScale (int newWidth, int newHeight,
                               int oldWidth, int oldHeight);
    static int scaleDimension (int dimension, double scale, int min, int max);

private:
    void updateShrukenDocumentPixmap ();

protected slots:
    void updatePreview ();

    // Call this whenever a value (e.g. an angle) changes
    // and the Dimensions & Preview need to be updated
    virtual void slotUpdate ();

    virtual void slotUpdateWithWaitCursor ();

protected:
    QString m_afterActionText;
    bool m_actOnSelection;
    kpMainWindow *m_mainWindow;

    int m_oldWidth, m_oldHeight;

    QGroupBox *m_dimensionsGroupBox;
    QLabel *m_afterTransformDimensionsLabel;

    QGroupBox *m_previewGroupBox;
    kpResizeSignallingLabel *m_previewPixmapLabel;
    QSize m_previewPixmapLabelSizeWhenUpdatedPixmap;
    QPixmap m_shrunkenDocumentPixmap;

    QGridLayout *m_gridLayout;
    int m_gridNumRows;
};


#endif  // __kp_tool_preview_dialog_h__
