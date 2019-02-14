
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


#ifndef kpDocumentSaveOptionsWidget_H
#define kpDocumentSaveOptionsWidget_H


#include <QRect>
#include <QSize>
#include <QWidget>

#include "imagelib/kpDocumentMetaInfo.h"
#include "document/kpDocumentSaveOptions.h"


class QComboBox;
class QImage;
class QLabel;
class QTimer;
class QSpinBox;
class QPushButton;

class kpDocumentSaveOptionsPreviewDialog;


class kpDocumentSaveOptionsWidget : public QWidget
{
Q_OBJECT

public:
    kpDocumentSaveOptionsWidget (const QImage &docPixmap,
                                 const kpDocumentSaveOptions &saveOptions,
                                 const kpDocumentMetaInfo &metaInfo,
                                 QWidget *parent);
    kpDocumentSaveOptionsWidget (QWidget *parent);
private:
    void init ();
public:
    ~kpDocumentSaveOptionsWidget () override;


    // <visualParent> is usually the filedialog
    void setVisualParent (QWidget *visualParent);


protected:
    bool mimeTypeHasConfigurableColorDepth () const;
    bool mimeTypeHasConfigurableQuality () const;

public:
    QString mimeType () const;
public slots:
    void setMimeType (const QString &string);

public:
    int colorDepth () const;
    bool dither () const;
protected:
    static int colorDepthComboItemFromColorDepthAndDither (int depth, bool dither);
public slots:
    void setColorDepthDither (int depth,
                              bool dither = kpDocumentSaveOptions::initialDither ());
protected slots:
    void slotColorDepthSelected ();

public:
    int quality () const;
public slots:
    void setQuality (int newQuality);

public:
    kpDocumentSaveOptions documentSaveOptions () const;
public slots:
    void setDocumentSaveOptions (const kpDocumentSaveOptions &saveOptions);


public:
    void setDocumentPixmap (const QImage &documentPixmap);
    void setDocumentMetaInfo (const kpDocumentMetaInfo &metaInfo);


protected:
    enum Mode
    {
        // (mutually exclusive)
        None, ColorDepth, Quality
    };

    Mode mode () const;
    void setMode (Mode mode);

protected slots:
    void repaintLabels ();


protected slots:
    void showPreview (bool yes = true);
    void hidePreview ();
    void updatePreviewDelayed ();
    void updatePreview ();
    void updatePreviewDialogLastRelativeGeometry ();


protected:
    QWidget *m_visualParent;

    Mode m_mode;

    QImage *m_documentPixmap;

    kpDocumentSaveOptions m_baseDocumentSaveOptions;
    kpDocumentMetaInfo m_documentMetaInfo;

    QLabel *m_colorDepthLabel;
    QComboBox *m_colorDepthCombo;
    int m_colorDepthComboLastSelectedItem;
    QWidget *m_colorDepthSpaceWidget;

    QLabel *m_qualityLabel;
    QSpinBox *m_qualityInput;

    QPushButton *m_previewButton;
    kpDocumentSaveOptionsPreviewDialog *m_previewDialog;
    QRect m_previewDialogLastRelativeGeometry;
    QTimer *m_updatePreviewTimer;
    int m_updatePreviewDelay;
    QTimer *m_updatePreviewDialogLastRelativeGeometryTimer;
};


#endif  // kpDocumentSaveOptionsWidget_H
