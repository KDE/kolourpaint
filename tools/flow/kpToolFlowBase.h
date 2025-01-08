
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef KP_TOOL_FLOW_BASE_H
#define KP_TOOL_FLOW_BASE_H

#include <QRect>

#include "layers/tempImage/kpTempImage.h"
#include "tools/kpTool.h"

class QPoint;
class QString;

class kpColor;
class kpToolFlowCommand;

class kpToolFlowBase : public kpTool
{
    Q_OBJECT

public:
    kpToolFlowBase(const QString &text, const QString &description, int key, kpToolEnvironment *environ, QObject *parent, const QString &name);

    ~kpToolFlowBase() override;

    // Returns the dirty rectangle for drawing a brush(of size
    // <brushWidth>x<brushHeight>) at <mousePoint>.  <mousePoint> will end
    // up being the midpoint of the returned rectangle(subject to integer
    // precision).
    static QRect hotRectForMousePointAndBrushWidthHeight(const QPoint &mousePoint, int brushWidth, int brushHeight);

    void begin() override;
    void end() override;

    void beginDraw() override;
    void hover(const QPoint &point) override;

    // drawPoint() normally calls drawLine(point,point).  Override drawPoint()
    // if you think you can be more efficient.
    virtual QRect drawPoint(const QPoint &point);
    virtual QRect drawLine(const QPoint &thisPoint, const QPoint &lastPoint) = 0;

    virtual bool drawShouldProceed(const QPoint & /*thisPoint*/, const QPoint & /*lastPoint*/, const QRect & /*normalizedRect*/)
    {
        return true;
    }
    void draw(const QPoint &thisPoint, const QPoint &lastPoint, const QRect &normalizedRect) override;
    void cancelShape() override;
    void releasedAllButtons() override;
    void endDraw(const QPoint &, const QRect &) override;

protected:
    virtual QString haventBegunDrawUserMessage() const = 0;

    virtual bool haveSquareBrushes() const
    {
        return false;
    }
    virtual bool haveDiverseBrushes() const
    {
        return false;
    }
    bool haveAnyBrushes() const
    {
        return (haveSquareBrushes() || haveDiverseBrushes());
    }

    virtual bool colorsAreSwapped() const
    {
        return false;
    }

    kpTempImage::UserFunctionType brushDrawFunction() const;
    void *brushDrawFunctionData() const;

    int brushWidth() const;
    int brushHeight() const;

    bool brushIsDiagonalLine() const;

    kpToolFlowCommand *currentCommand() const;
    virtual kpColor color(int which);
    QRect hotRect() const;

protected Q_SLOTS:
    void updateBrushAndCursor();

    void slotForegroundColorChanged(const kpColor &col) override;
    void slotBackgroundColorChanged(const kpColor &col) override;

private:
    void clearBrushCursorData();

private:
    struct kpToolFlowBasePrivate *d;
};

#endif // KP_TOOL_FLOW_BASE_H
