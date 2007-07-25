
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


#ifndef kpMainWindowPrivate_H
#define kpMainWindowPrivate_H


class QAction;
class QActionGroup;

class KSelectAction;
class KToggleAction;

class kpCommandEnvironment;
class kpDocumentEnvironment;
class kpToolSelectionEnvironment;
class kpTransformDialogEnvironment;


struct kpMainWindowPrivate
{
    kpMainWindowPrivate ()
        : documentEnvironment (0),
          commandEnvironment (0),
          toolSelectionEnvironment (0),
          toolsActionGroup (0),
          transformDialogEnvironment (0)
    {
    }


    kpDocumentEnvironment *documentEnvironment;
    kpCommandEnvironment *commandEnvironment;


    //
    // Tools
    //

    kpToolSelectionEnvironment *toolSelectionEnvironment;
    QActionGroup *toolsActionGroup;


    //
    // File Menu
    //

    bool configOpenImagesInSameWindow;


    //
    // View Menu
    //

    bool m_configThumbnailShowRectangle;
    KToggleAction *m_actionShowThumbnailRectangle;


    //
    // Image Menu
    //

    kpTransformDialogEnvironment *transformDialogEnvironment;

    // Implemented in kpMainWindow_Tools.cpp, not kpImageWindow_Image.cpp
    // since they're really setting tool options.
    KToggleAction *actionDrawOpaque;
    QAction *actionDrawColorSimilarity;

    int m_moreEffectsDialogLastEffect;
    bool m_resizeScaleDialogLastKeepAspect;


    //
    // Colors Menu
    //

    QAction *actionColorsDefault;
    KSelectAction *actionColorsKDE;
    QAction *actionColorsOpen;

    QAction *actionColorsSaveAs;

    QAction *actionColorsAppendRow;
    QAction *actionColorsDeleteRow;


    //
    // Help Menu
    //

    QAction *m_actionHelpTakingScreenshots;
};


#endif  // kpMainWindowPrivate_H
