
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


#ifndef __kp_tool_text_h__
#define __kp_tool_text_h__

#include <qstring.h>

#include <kpcommandhistory.h>

#include <kptextstyle.h>
#include <kptoolselection.h>

class kpColor;
class kpMainWindow;
class kpSelection;
class kpViewManager;

class kpToolText : public kpToolSelection
{
Q_OBJECT

public:
    kpToolText (kpMainWindow *mainWindow);
    virtual ~kpToolText ();

    virtual bool careAboutColorsSwapped () const { return true; }

    virtual void begin ();
    virtual void end ();

    bool hasBegunText () const;
    virtual bool hasBegunShape () const;
    virtual void cancelShape ();
    virtual void endShape (const QPoint &thisPoint, const QRect &normalizedRect);

protected:
    virtual void keyPressEvent (QKeyEvent *e);
    virtual void imStartEvent (QIMEvent *e);
    virtual void imComposeEvent (QIMEvent *e);
    virtual void imEndEvent (QIMEvent *e);

protected:
    bool shouldChangeTextStyle () const;
    void changeTextStyle (const QString &name,
                          const kpTextStyle &newTextStyle,
                          const kpTextStyle &oldTextStyle);

protected slots:
    virtual void slotIsOpaqueChanged ();
    virtual void slotColorsSwapped (const kpColor &newForegroundColor,
                                    const kpColor &newBackgroundColor);
    virtual void slotForegroundColorChanged (const kpColor &color);
    virtual void slotBackgroundColorChanged (const kpColor &color);
    virtual void slotColorSimilarityChanged (double, int);

public slots:
    void slotFontFamilyChanged (const QString &fontFamily, const QString &oldFontFamily);
    void slotFontSizeChanged (int fontSize, int oldFontSize);
    void slotBoldChanged (bool isBold);
    void slotItalicChanged (bool isItalic);
    void slotUnderlineChanged (bool isUnderline);
    void slotStrikeThruChanged (bool isStrikeThru);

protected:
    class kpToolTextInsertCommand *m_insertCommand;
    class kpToolTextEnterCommand *m_enterCommand;
    class kpToolTextBackspaceCommand *m_backspaceCommand;
    class kpToolTextDeleteCommand *m_deleteCommand;

    bool m_isIMStarted;
    int m_IMStartCursorRow;
    int m_IMStartCursorCol;
    QString m_IMPreeditStr;
};


class kpToolTextChangeStyleCommand : public kpNamedCommand
{
public:
    kpToolTextChangeStyleCommand (const QString &name,
        const kpTextStyle &newTextStyle, const kpTextStyle &oldTextStyle,
        kpMainWindow *mainWindow);
    virtual ~kpToolTextChangeStyleCommand ();

    virtual int size () const;
    
    virtual void execute ();
    virtual void unexecute ();

protected:
    kpTextStyle m_newTextStyle, m_oldTextStyle;
};

class kpToolTextInsertCommand : public kpNamedCommand
{
public:
    kpToolTextInsertCommand (const QString &name,
        int row, int col, QString newText,
        kpMainWindow *mainWindow);
    virtual ~kpToolTextInsertCommand ();

    void addText (const QString &moreText);

    virtual int size () const;
    
    virtual void execute ();
    virtual void unexecute ();

protected:
    int m_row, m_col;
    QString m_newText;
};

class kpToolTextEnterCommand : public kpNamedCommand
{
public:
    kpToolTextEnterCommand (const QString &name,
        int row, int col,
        kpMainWindow *mainWindow);
    virtual ~kpToolTextEnterCommand ();

    void addEnter ();

    virtual int size () const;
    
    virtual void execute ();
    virtual void unexecute ();

protected:
    int m_row, m_col;
    int m_numEnters;
};

class kpToolTextBackspaceCommand : public kpNamedCommand
{
public:
    kpToolTextBackspaceCommand (const QString &name,
        int row, int col,
        kpMainWindow *mainWindow);
    virtual ~kpToolTextBackspaceCommand ();

    void addBackspace ();

    virtual int size () const;
    
    virtual void execute ();
    virtual void unexecute ();

protected:
    int m_row, m_col;
    int m_numBackspaces;
    QString m_deletedText;
};

class kpToolTextDeleteCommand : public kpNamedCommand
{
public:
    kpToolTextDeleteCommand (const QString &name,
        int row, int col,
        kpMainWindow *mainWindow);
    virtual ~kpToolTextDeleteCommand ();

    void addDelete ();

    virtual int size () const;
    
    virtual void execute ();
    virtual void unexecute ();

protected:
    int m_row, m_col;
    int m_numDeletes;
    QString m_deletedText;
};

#endif  // __kp_tool_text_h__

