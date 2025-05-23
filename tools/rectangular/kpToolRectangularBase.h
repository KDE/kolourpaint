
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef KP_TOOL_RECTANGULAR_BASE_H
#define KP_TOOL_RECTANGULAR_BASE_H

#include "imagelib/kpImage.h"
#include "tools/kpTool.h"

class QPoint;
class QRect;
class QString;

class kpColor;

struct kpToolRectangularBasePrivate;

// it turns out that these shapes are all really the same thing
// (same options, same feel) - the only real difference is the
// drawing function i.e. drawShape().
class kpToolRectangularBase : public kpTool
{
    Q_OBJECT

public:
    typedef void (*DrawShapeFunc)(kpImage * /*image*/,
                                  int /*x*/,
                                  int /*y*/,
                                  int /*width*/,
                                  int /*height*/,
                                  const kpColor & /*fcolor*/,
                                  int /*penWidth = 1*/,
                                  const kpColor & /*bcolor = kpColor::Invalid*/);

    kpToolRectangularBase(const QString &text,
                          const QString &description,
                          DrawShapeFunc drawShapeFunc,
                          int key,
                          kpToolEnvironment *environ,
                          QObject *parent,
                          const QString &name);

    ~kpToolRectangularBase() override;

    bool careAboutModifierState() const override
    {
        return true;
    }

private Q_SLOTS:
    virtual void slotLineWidthChanged();
    virtual void slotFillStyleChanged();

private:
    QString haventBegunDrawUserMessage() const;

public:
    void begin() override;
    void end() override;

private:
    void applyModifiers();
    void beginDraw() override;

private:
    kpColor drawingForegroundColor() const;
    kpColor drawingBackgroundColor() const;
    void updateShape();

public:
    void draw(const QPoint &, const QPoint &, const QRect &) override;
    void cancelShape() override;
    void releasedAllButtons() override;
    void endDraw(const QPoint &, const QRect &) override;

private:
    kpToolRectangularBasePrivate *const d;
};

#endif // KP_TOOL_RECTANGULAR_BASE_H
