
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef kpTransformResizeScaleCommand_H
#define kpTransformResizeScaleCommand_H

#include "commands/kpCommand.h"
#include "imagelib/kpColor.h"
#include "imagelib/kpImage.h"

class QSize;

class kpAbstractSelection;

// REFACTOR: Split into multiple classes, each doing a different thing
//           e.g. resize, scale and smooth scale.
// REFACTOR: Replace kpToolSelectionResizeScaleCommand with us.
class kpTransformResizeScaleCommand : public kpCommand
{
public:
    enum Type {
        Resize,
        Scale,
        SmoothScale
    };

    kpTransformResizeScaleCommand(bool actOnSelection, int newWidth, int newHeight, Type type, kpCommandEnvironment *environ);
    ~kpTransformResizeScaleCommand() override;

    QString name() const override;
    SizeType size() const override;

public:
    int newWidth() const;
    void setNewWidth(int width);

    int newHeight() const;
    void setNewHeight(int height);

    QSize newSize() const;
    virtual void resize(int width, int height);

public:
    bool scaleSelectionWithImage() const;

private:
    void scaleSelectionRegionWithDocument();

public:
    void execute() override;
    void unexecute() override;

protected:
    bool m_actOnSelection;
    int m_newWidth, m_newHeight;
    Type m_type;
    bool m_isLosslessScale;
    bool m_scaleSelectionWithImage;
    kpColor m_backgroundColor;

    int m_oldWidth, m_oldHeight;
    bool m_actOnTextSelection;
    kpImage m_oldImage, m_oldRightImage, m_oldBottomImage;
    kpAbstractSelection *m_oldSelectionPtr;
};

#endif // kpTransformResizeScaleCommand_H
