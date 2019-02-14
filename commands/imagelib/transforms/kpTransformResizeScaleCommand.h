
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


#ifndef kpTransformResizeScaleCommand_H
#define kpTransformResizeScaleCommand_H


#include <QPixmap>

#include "imagelib/kpColor.h"
#include "commands/kpCommand.h"
#include "imagelib/kpImage.h"


class QSize;

class kpAbstractSelection;


// REFACTOR: Split into multiple classes, each doing a different thing
//           e.g. resize, scale and smooth scale.
// REFACTOR: Replace kpToolSelectionResizeScaleCommand with us.
class kpTransformResizeScaleCommand : public kpCommand
{
public:
    enum Type
    {
        Resize, Scale, SmoothScale
    };

    kpTransformResizeScaleCommand (bool actOnSelection,
        int newWidth, int newHeight,
        Type type,
        kpCommandEnvironment *environ);
    ~kpTransformResizeScaleCommand () override;

    QString name () const override;
    SizeType size () const override;

public:
    int newWidth () const;
    void setNewWidth (int width);

    int newHeight () const;
    void setNewHeight (int height);

    QSize newSize () const;
    virtual void resize (int width, int height);

public:
    bool scaleSelectionWithImage () const;

private:
    void scaleSelectionRegionWithDocument ();

public:
    void execute () override;
    void unexecute () override;

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


#endif  // kpTransformResizeScaleCommand_H
