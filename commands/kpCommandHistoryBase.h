
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef kpCommandHistoryBase_H
#define kpCommandHistoryBase_H

#include <QList>
#include <QObject>
#include <QString>

#include "commands/kpCommandSize.h"

class QAction;

class KActionCollection;
class KToolBarPopupAction;

class kpCommand;

// Clone of KCommandHistory with features required by KolourPaint but which
// could also be useful for other apps:
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
    kpCommandHistoryBase(bool doReadConfig, KActionCollection *ac);
    ~kpCommandHistoryBase() override;

public:
    // (provided for compatibility with KCommandHistory)
    int undoLimit() const;
    void setUndoLimit(int limit);

    int undoMinLimit() const;
    void setUndoMinLimit(int limit);

    int undoMaxLimit() const;
    void setUndoMaxLimit(int limit);

    kpCommandSize::SizeType undoMaxLimitSizeLimit() const;
    void setUndoMaxLimitSizeLimit(kpCommandSize::SizeType sizeLimit);

public:
    // Read and write above config
    void readConfig();
    void writeConfig();

public:
    void addCommand(kpCommand *command, bool execute = true);
    void clear();

protected Q_SLOTS:
    // (same as undo() & redo() except they don't call
    //  trimCommandListsUpdateActions())
    void undoInternal();
    void redoInternal();

public Q_SLOTS:
    virtual void undo();
    virtual void redo();

    virtual void undoUpToNumber(QAction *which);
    virtual void redoUpToNumber(QAction *which);

protected:
    QString undoActionText() const;
    QString redoActionText() const;

    QString undoActionToolTip() const;
    QString redoActionToolTip() const;

    void trimCommandListsUpdateActions();
    void trimCommandList(QList<kpCommand *> &commandList);
    void trimCommandLists();
    void updateActions();

public:
    kpCommand *nextUndoCommand() const;
    kpCommand *nextRedoCommand() const;

    void setNextUndoCommand(kpCommand *command);

public Q_SLOTS:
    virtual void documentSaved();

Q_SIGNALS:
    void documentRestored();

protected:
    KToolBarPopupAction *m_actionUndo, *m_actionRedo;

    // (Front element is the next one)
    QList<kpCommand *> m_undoCommandList;
    QList<kpCommand *> m_redoCommandList;

    int m_undoMinLimit, m_undoMaxLimit;
    kpCommandSize::SizeType m_undoMaxLimitSizeLimit;

    // What you have to do to get back to the document's unmodified state:
    // * -x: must Undo x times
    // * 0: unmodified
    // * +x: must Redo x times
    // * INT_MAX: can never become unmodified again
    //
    // ASSUMPTION: will never have INT_MAX commands in any list.
    int m_documentRestoredPosition;
};

#endif // kpCommandHistoryBase_H
