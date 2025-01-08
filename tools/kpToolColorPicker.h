
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef KP_TOOL_COLOR_PICKER_H
#define KP_TOOL_COLOR_PICKER_H

#include "imagelib/kpColor.h"
#include "tools/kpTool.h"

class QPoint;
class QRect;

class kpToolColorPicker : public kpTool
{
    Q_OBJECT

public:
    kpToolColorPicker(kpToolEnvironment *environ, QObject *parent);
    ~kpToolColorPicker() override;

    // generally the user goes to pick a color but wants to return to using
    // his/her previous drawing tool
    bool returnToPreviousToolAfterEndDraw() const override
    {
        return true;
    }

private:
    kpColor colorAtPixel(const QPoint &p);

    QString haventBegunDrawUserMessage() const;

public:
    void begin() override;
    void beginDraw() override;
    void draw(const QPoint &thisPoint, const QPoint &, const QRect &) override;
    void cancelShape() override;
    void releasedAllButtons() override;
    void endDraw(const QPoint &thisPoint, const QRect &) override;

private:
    kpColor m_oldColor;
};

#endif // KP_TOOL_COLOR_PICKER_H
