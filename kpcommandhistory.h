
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


#ifndef KP_COMMAND_HISTORY_H
#define KP_COMMAND_HISTORY_H

#include <qobject.h>
#include <qstring.h>
#include <qvaluelist.h>


class KActionCollection;
class KToolBarPopupAction;

class kpDocument;
class kpMainWindow;
class kpSelection;
class kpViewManager;


class kpCommand
{
public:
    kpCommand (kpMainWindow *mainWindow);
    virtual ~kpCommand ();

public:
    virtual QString name () const = 0;

    // Returns the estimated size in bytes.
    //
    // You only have to factor in the size of variables that change according
    // to the amount of input e.g. pixmap size, text size.  There is no need
    // to include the size of O(1) variables unless they are huge.
    //
    // If in doubt, return the largest possible amount of memory that your
    // command will take.  This is better than making the user unexpectedly
    // run out of memory.
    virtual int size () const = 0;

    virtual void execute () = 0;
    virtual void unexecute () = 0;

protected:
    kpMainWindow *mainWindow () const;

    kpDocument *document () const;
    kpSelection *selection () const;

    kpViewManager *viewManager () const;

protected:
    kpMainWindow *m_mainWindow;
};


class kpNamedCommand : public kpCommand
{
public:
    kpNamedCommand (const QString &name, kpMainWindow *mainWindow);
    virtual ~kpNamedCommand ();

    virtual QString name () const;

protected:
    QString m_name;
};


class kpMacroCommand : public kpNamedCommand
{
public:
    kpMacroCommand (const QString &name, kpMainWindow *mainWindow);
    virtual ~kpMacroCommand ();


    //
    // kpCommand Interface
    //

    virtual int size () const;

    virtual void execute ();
    virtual void unexecute ();


    //
    // Interface
    //

    void addCommand (kpCommand *command);

protected:
    QValueList <kpCommand *> m_commandList;

private:
    class kpMacroCommandPrivate *d;
};


// Clone of KCommandHistory with features required by KolourPaint:
// - nextUndoCommand()/nextRedoCommand()
// - undo/redo history limited by both number and size
//
// Features not required by KolourPaint (e.g. commandExecuted()) are not
// implemented and undo limit == redo limit.  So compared to
// KCommandHistory, this is only "almost source compatible".
class kpCommandHistoryBase : public QObject
{
Q_OBJECT

public:
    kpCommandHistoryBase (bool doReadConfig, KActionCollection *ac);
    virtual ~kpCommandHistoryBase ();

public:
    // (provided for compatibility with KCommandHistory)
    int undoLimit () const;
    void setUndoLimit (int limit);


    int undoMinLimit () const;
    void setUndoMinLimit (int limit);

    int undoMaxLimit () const;
    void setUndoMaxLimit (int limit);

    int undoMaxLimitSizeLimit () const;
    void setUndoMaxLimitSizeLimit (int sizeLimit);

public:
    // Read and write above config
    void readConfig ();
    void writeConfig ();

public:
    void addCommand (kpCommand *command, bool execute = true);
    void clear ();

protected slots:
    // (same as undo() & redo() except they don't call
    //  trimCommandListsUpdateActions())
    void undoInternal ();
    void redoInternal ();

public slots:
    virtual void undo ();
    virtual void redo ();

    virtual void undoUpToNumber (int which);
    virtual void redoUpToNumber (int which);

protected:
    QString undoActionText () const;
    QString redoActionText () const;

    void trimCommandListsUpdateActions ();
    void trimCommandList (QValueList <kpCommand *> *commandList);
    void trimCommandLists ();
    void updateActions ();

public:
    kpCommand *nextUndoCommand () const;
    kpCommand *nextRedoCommand () const;

    void setNextUndoCommand (kpCommand *command);

public slots:
    virtual void documentSaved ();

signals:
    void documentRestored ();

protected:
    KToolBarPopupAction *m_actionUndo, *m_actionRedo;

    // (Front element is the next one)
    QValueList <kpCommand *> m_undoCommandList;
    QValueList <kpCommand *> m_redoCommandList;

    int m_undoMinLimit, m_undoMaxLimit, m_undoMaxLimitSizeLimit;

    // What you have to do to get back to the document's unmodified state:
    // * -x: must Undo x times
    // * 0: unmodified
    // * +x: must Redo x times
    // * INT_MAX: can never become unmodified again
    //
    // ASSUMPTION: will never have INT_MAX commands in any list.
    int m_documentRestoredPosition;

private:
    class kpCommandHistoryBasePrivate *d;
};


// Intercepts Undo/Redo requests:
//
// If the user is currently drawing a shape, it cancels it.
// Else it passes on the Undo/Redo request to kpCommandHistoryBase.
//
// TODO: This is wrong.  It won't work if the Undo action is disabled,
//       for instance.
//
//       Maybe the real solution is to call kpCommandHistoryBase::addCommand()
//       as _soon_ as the shape starts - not after it ends.  But the
//       trouble with this solution is that if the user Undoes/cancels
//       the shape s/he's currently drawing, it would replace a Redo
//       slot in the history.  Arguably you shouldn't be able to Redo
//       something you never finished drawing.
//
//       The solution is to add this functionality to kpCommandHistoryBase.
class kpCommandHistory : public kpCommandHistoryBase
{
Q_OBJECT

public:
    kpCommandHistory (bool doReadConfig, kpMainWindow *mainWindow);
    virtual ~kpCommandHistory ();

public slots:
    virtual void undo ();
    virtual void redo ();

protected:
    kpMainWindow *m_mainWindow;
};


#endif  // KP_COMMAND_HISTORY_H
