
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef kpTransformPreviewDialog_H
#define kpTransformPreviewDialog_H

#include <QDialog>

class QLabel;
class QGridLayout;
class QGroupBox;

class kpDocument;
class kpResizeSignallingLabel;
class kpTransformDialogEnvironment;

class kpTransformPreviewDialog : public QDialog
{
    Q_OBJECT

public:
    enum Features {
        Dimensions = 1,
        Preview = 2,
        AllFeatures = Dimensions | Preview
    };

    // You must call slotUpdate() in your constructor
    kpTransformPreviewDialog(Features features,
                             bool reserveTopRow,
                             // e.g. "Skew (Image|Selection)"
                             const QString &caption,
                             // (in the Dimensions Group Box) e.g. "After Skew:"
                             const QString &afterActionText,
                             bool actOnSelection,
                             kpTransformDialogEnvironment *_env,
                             QWidget *parent);
    ~kpTransformPreviewDialog() override;

private:
    void createDimensionsGroupBox();
    void createPreviewGroupBox();

public:
    virtual bool isNoOp() const = 0;

protected:
    kpDocument *document() const;

    QWidget *mainWidget() const;

    // All widgets must have mainWidget() as their parent
    void addCustomWidgetToFront(QWidget *w); // see <reserveTopRow> in ctor
    void addCustomWidget(QWidget *w);
    void addCustomWidgetToBack(QWidget *w)
    {
        addCustomWidget(w);
    }

    virtual QSize newDimensions() const = 0;
    virtual QImage transformPixmap(const QImage &pixmap, int targetWidth, int targetHeight) const = 0;

public:
    // Use to avoid excessive, expensive preview pixmap label recalculations,
    // during init and widget relayouts.
    //
    // Setting <enable> to true automatically calls slotUpdateWithWaitCursor().
    //
    // WARNING: This overrides a non-virtual method in QWidget.
    void setUpdatesEnabled(bool enable);

private:
    void updateDimensions();

public:
    static double aspectScale(int newWidth, int newHeight, int oldWidth, int oldHeight);
    static int scaleDimension(int dimension, double scale, int min, int max);

private:
    void updateShrunkenDocumentPixmap();

protected Q_SLOTS:
    void updatePreview();

    // Call this whenever a value (e.g. an angle) changes
    // and the Dimensions & Preview need to be updated
    virtual void slotUpdate();

    virtual void slotUpdateWithWaitCursor();

protected:
    // REFACTOR: Use d-ptr
    QString m_afterActionText;
    bool m_actOnSelection;

    int m_oldWidth, m_oldHeight;

    QWidget *m_mainWidget;
    QGroupBox *m_dimensionsGroupBox;
    QLabel *m_afterTransformDimensionsLabel;

    QGroupBox *m_previewGroupBox;
    kpResizeSignallingLabel *m_previewPixmapLabel;
    QSize m_previewPixmapLabelSizeWhenUpdatedPixmap;
    QImage m_shrunkenDocumentPixmap;

    QGridLayout *m_gridLayout;
    int m_gridNumRows;

    kpTransformDialogEnvironment *m_environ;
};

#endif // kpTransformPreviewDialog_H
