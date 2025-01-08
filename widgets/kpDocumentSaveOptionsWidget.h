
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef kpDocumentSaveOptionsWidget_H
#define kpDocumentSaveOptionsWidget_H

#include <QRect>
#include <QWidget>

#include "document/kpDocumentSaveOptions.h"
#include "imagelib/kpDocumentMetaInfo.h"

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
    kpDocumentSaveOptionsWidget(const QImage &docPixmap, const kpDocumentSaveOptions &saveOptions, const kpDocumentMetaInfo &metaInfo, QWidget *parent);
    kpDocumentSaveOptionsWidget(QWidget *parent);

private:
    void init();

public:
    ~kpDocumentSaveOptionsWidget() override;

    // <visualParent> is usually the filedialog
    void setVisualParent(QWidget *visualParent);

protected:
    bool mimeTypeHasConfigurableColorDepth() const;
    bool mimeTypeHasConfigurableQuality() const;

public:
    QString mimeType() const;
public Q_SLOTS:
    void setMimeType(const QString &string);

public:
    int colorDepth() const;
    bool dither() const;

protected:
    static int colorDepthComboItemFromColorDepthAndDither(int depth, bool dither);
public Q_SLOTS:
    void setColorDepthDither(int depth, bool dither = kpDocumentSaveOptions::initialDither());
protected Q_SLOTS:
    void slotColorDepthSelected();

public:
    int quality() const;
public Q_SLOTS:
    void setQuality(int newQuality);

public:
    kpDocumentSaveOptions documentSaveOptions() const;
public Q_SLOTS:
    void setDocumentSaveOptions(const kpDocumentSaveOptions &saveOptions);

public:
    void setDocumentPixmap(const QImage &documentPixmap);
    void setDocumentMetaInfo(const kpDocumentMetaInfo &metaInfo);

protected:
    enum Mode {
        // (mutually exclusive)
        None,
        ColorDepth,
        Quality
    };

    Mode mode() const;
    void setMode(Mode mode);

protected Q_SLOTS:
    void repaintLabels();

protected Q_SLOTS:
    void showPreview(bool yes = true);
    void hidePreview();
    void updatePreviewDelayed();
    void updatePreview();
    void updatePreviewDialogLastRelativeGeometry();

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

#endif // kpDocumentSaveOptionsWidget_H
