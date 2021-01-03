
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


#define DEBUG_KP_COMMAND_HISTORY 0


#include "kpCommandHistoryBase.h"

#include <climits>

#include <QMenu>

#include <KSharedConfig>
#include <KConfigGroup>
#include <KStandardShortcut>
#include <KStandardAction>
#include <KToolBarPopupAction>
#include <KActionCollection>
#include <KLocalizedString>

#include "kpCommand.h"
#include "kpLogCategories.h"
#include "environments/commands/kpCommandEnvironment.h"
#include "kpDefs.h"
#include "document/kpDocument.h"
#include "mainWindow/kpMainWindow.h"
#include "tools/kpTool.h"

//---------------------------------------------------------------------

static void ClearPointerList(QList<kpCommand *> &list)
{
    qDeleteAll(list);
    list.clear();
}

//--------------------------------------------------------------------------------

kpCommandHistoryBase::kpCommandHistoryBase (bool doReadConfig,
                                            KActionCollection *ac)
{
    m_actionUndo = new KToolBarPopupAction(QIcon::fromTheme(QStringLiteral("edit-undo")), undoActionText (), this);
    ac->addAction (KStandardAction::name (KStandardAction::Undo), m_actionUndo);
    ac->setDefaultShortcuts (m_actionUndo, KStandardShortcut::shortcut (KStandardShortcut::Undo));
    connect (m_actionUndo, &KToolBarPopupAction::triggered, this, &kpCommandHistoryBase::undo);

    m_actionRedo = new KToolBarPopupAction(QIcon::fromTheme(QStringLiteral("edit-redo")), redoActionText (), this);
    ac->addAction (KStandardAction::name (KStandardAction::Redo), m_actionRedo);
    ac->setDefaultShortcuts (m_actionRedo, KStandardShortcut::shortcut (KStandardShortcut::Redo));
    connect (m_actionRedo, &KToolBarPopupAction::triggered, this, &kpCommandHistoryBase::redo );


    m_actionUndo->setEnabled (false);
    m_actionRedo->setEnabled (false);


    connect (m_actionUndo->menu(), &QMenu::triggered,
             this, &kpCommandHistoryBase::undoUpToNumber);

    connect (m_actionRedo->menu(), &QMenu::triggered,
             this, &kpCommandHistoryBase::redoUpToNumber);


    m_undoMinLimit = 10;
    m_undoMaxLimit = 500;
    m_undoMaxLimitSizeLimit = 16 * 1048576;


    m_documentRestoredPosition = 0;


    if (doReadConfig) {
        readConfig ();
    }
}

//--------------------------------------------------------------------------------

kpCommandHistoryBase::~kpCommandHistoryBase ()
{
    ::ClearPointerList(m_undoCommandList);
    ::ClearPointerList(m_redoCommandList);
}

//--------------------------------------------------------------------------------

// public
int kpCommandHistoryBase::undoLimit () const
{
    return undoMinLimit ();
}

// public
void kpCommandHistoryBase::setUndoLimit (int limit)
{
    setUndoMinLimit (limit);
}


// public
int kpCommandHistoryBase::undoMinLimit () const
{
    return m_undoMinLimit;
}

// public
void kpCommandHistoryBase::setUndoMinLimit (int limit)
{
#if DEBUG_KP_COMMAND_HISTORY
    qCDebug(kpLogCommands) << "kpCommandHistoryBase::setUndoMinLimit("
               << limit << ")";
#endif

    if (limit < 1 || limit > 5000/*"ought to be enough for anybody"*/)
    {
        qCCritical(kpLogCommands) << "kpCommandHistoryBase::setUndoMinLimit("
                   << limit << ")";
        return;
    }

    if (limit == m_undoMinLimit) {
        return;
    }

    m_undoMinLimit = limit;
    trimCommandListsUpdateActions ();
}


// public
int kpCommandHistoryBase::undoMaxLimit () const
{
    return m_undoMaxLimit;
}

// public
void kpCommandHistoryBase::setUndoMaxLimit (int limit)
{
#if DEBUG_KP_COMMAND_HISTORY
    qCDebug(kpLogCommands) << "kpCommandHistoryBase::setUndoMaxLimit("
               << limit << ")";
#endif

    if (limit < 1 || limit > 5000/*"ought to be enough for anybody"*/)
    {
        qCCritical(kpLogCommands) << "kpCommandHistoryBase::setUndoMaxLimit("
                   << limit << ")";
        return;
    }

    if (limit == m_undoMaxLimit) {
        return;
    }

    m_undoMaxLimit = limit;
    trimCommandListsUpdateActions ();
}


// public
kpCommandSize::SizeType kpCommandHistoryBase::undoMaxLimitSizeLimit () const
{
    return m_undoMaxLimitSizeLimit;
}

// public
void kpCommandHistoryBase::setUndoMaxLimitSizeLimit (kpCommandSize::SizeType sizeLimit)
{
#if DEBUG_KP_COMMAND_HISTORY
    qCDebug(kpLogCommands) << "kpCommandHistoryBase::setUndoMaxLimitSizeLimit("
               << sizeLimit << ")";
#endif

    if (sizeLimit < 0 ||
        sizeLimit > (500 * 1048576)/*"ought to be enough for anybody"*/)
    {
        qCCritical(kpLogCommands) << "kpCommandHistoryBase::setUndoMaxLimitSizeLimit("
                   << sizeLimit << ")";
        return;
    }

    if (sizeLimit == m_undoMaxLimitSizeLimit) {
        return;
    }

    m_undoMaxLimitSizeLimit = sizeLimit;
    trimCommandListsUpdateActions ();
}


// public
void kpCommandHistoryBase::readConfig ()
{
#if DEBUG_KP_COMMAND_HISTORY
    qCDebug(kpLogCommands) << "kpCommandHistoryBase::readConfig()";
#endif
    KConfigGroup cfg (KSharedConfig::openConfig (), kpSettingsGroupUndoRedo);

    setUndoMinLimit (cfg.readEntry (kpSettingUndoMinLimit, undoMinLimit ()));
    setUndoMaxLimit (cfg.readEntry (kpSettingUndoMaxLimit, undoMaxLimit ()));
    setUndoMaxLimitSizeLimit (
        cfg.readEntry <kpCommandSize::SizeType> (kpSettingUndoMaxLimitSizeLimit,
                                                 undoMaxLimitSizeLimit ()));

    trimCommandListsUpdateActions ();
}

// public
void kpCommandHistoryBase::writeConfig ()
{
#if DEBUG_KP_COMMAND_HISTORY
    qCDebug(kpLogCommands) << "kpCommandHistoryBase::writeConfig()";
#endif
    KConfigGroup cfg (KSharedConfig::openConfig (), kpSettingsGroupUndoRedo);

    cfg.writeEntry (kpSettingUndoMinLimit, undoMinLimit ());
    cfg.writeEntry (kpSettingUndoMaxLimit, undoMaxLimit ());
    cfg.writeEntry <kpCommandSize::SizeType> (
        kpSettingUndoMaxLimitSizeLimit, undoMaxLimitSizeLimit ());

    cfg.sync ();
}


// public
void kpCommandHistoryBase::addCommand (kpCommand *command, bool execute)
{
#if DEBUG_KP_COMMAND_HISTORY
    qCDebug(kpLogCommands) << "kpCommandHistoryBase::addCommand("
               << command
               << ",execute=" << execute << ")"
#endif

    if (execute) {
        command->execute ();
    }

    m_undoCommandList.push_front (command);
    ::ClearPointerList(m_redoCommandList);

#if DEBUG_KP_COMMAND_HISTORY
    qCDebug(kpLogCommands) << "\tdocumentRestoredPosition=" << m_documentRestoredPosition;
#endif
    if (m_documentRestoredPosition != INT_MAX)
    {
        if (m_documentRestoredPosition > 0) {
            m_documentRestoredPosition = INT_MAX;
        }
        else {
            m_documentRestoredPosition--;
        }
    #if DEBUG_KP_COMMAND_HISTORY
        qCDebug(kpLogCommands) << "\t\tdocumentRestoredPosition=" << m_documentRestoredPosition;
    #endif
    }

    trimCommandListsUpdateActions ();
}

// public
void kpCommandHistoryBase::clear ()
{
#if DEBUG_KP_COMMAND_HISTORY
    qCDebug(kpLogCommands) << "kpCommandHistoryBase::clear()";
#endif

    ::ClearPointerList(m_undoCommandList);
    ::ClearPointerList(m_redoCommandList);

    m_documentRestoredPosition = 0;

    updateActions ();
}

//---------------------------------------------------------------------

// protected slot
void kpCommandHistoryBase::undoInternal ()
{
#if DEBUG_KP_COMMAND_HISTORY
    qCDebug(kpLogCommands) << "kpCommandHistoryBase::undoInternal()";
#endif

    kpCommand *undoCommand = nextUndoCommand ();
    if (!undoCommand) {
        return;
    }

    undoCommand->unexecute ();


    m_undoCommandList.erase (m_undoCommandList.begin ());
    m_redoCommandList.push_front (undoCommand);

#if DEBUG_KP_COMMAND_HISTORY
    qCDebug(kpLogCommands) << "\tdocumentRestoredPosition=" << m_documentRestoredPosition;
#endif
    if (m_documentRestoredPosition != INT_MAX)
    {
        m_documentRestoredPosition++;
        if (m_documentRestoredPosition == 0)
            emit documentRestored ();
    #if DEBUG_KP_COMMAND_HISTORY
        qCDebug(kpLogCommands) << "\t\tdocumentRestoredPosition=" << m_documentRestoredPosition;
    #endif
    }
}

//---------------------------------------------------------------------

// protected slot
void kpCommandHistoryBase::redoInternal ()
{
#if DEBUG_KP_COMMAND_HISTORY
    qCDebug(kpLogCommands) << "kpCommandHistoryBase::redoInternal()";
#endif

    kpCommand *redoCommand = nextRedoCommand ();
    if (!redoCommand) {
        return;
    }

    redoCommand->execute ();


    m_redoCommandList.erase (m_redoCommandList.begin ());
    m_undoCommandList.push_front (redoCommand);

#if DEBUG_KP_COMMAND_HISTORY
    qCDebug(kpLogCommands) << "\tdocumentRestoredPosition=" << m_documentRestoredPosition;
#endif
    if (m_documentRestoredPosition != INT_MAX)
    {
        m_documentRestoredPosition--;
        if (m_documentRestoredPosition == 0) {
            emit documentRestored ();
        }
    #if DEBUG_KP_COMMAND_HISTORY
        qCDebug(kpLogCommands) << "\t\tdocumentRestoredPosition=" << m_documentRestoredPosition;
    #endif
    }
}

//---------------------------------------------------------------------

// public slot virtual
void kpCommandHistoryBase::undo ()
{
#if DEBUG_KP_COMMAND_HISTORY
    qCDebug(kpLogCommands) << "kpCommandHistoryBase::undo()";
#endif

    undoInternal ();
    trimCommandListsUpdateActions ();
}

//---------------------------------------------------------------------

// public slot virtual
void kpCommandHistoryBase::redo ()
{
#if DEBUG_KP_COMMAND_HISTORY
    qCDebug(kpLogCommands) << "kpCommandHistoryBase::redo()";
#endif

    redoInternal ();
    trimCommandListsUpdateActions ();
}

//---------------------------------------------------------------------

// public slot virtual
void kpCommandHistoryBase::undoUpToNumber (QAction *which)
{
#if DEBUG_KP_COMMAND_HISTORY
    qCDebug(kpLogCommands) << "kpCommandHistoryBase::undoUpToNumber(" << which << ")";
#endif

    for (int i = 0;
         i <= which->data().toInt() && !m_undoCommandList.isEmpty ();
         i++)
    {
        undoInternal ();
    }

    trimCommandListsUpdateActions ();
}

// public slot virtual
void kpCommandHistoryBase::redoUpToNumber (QAction *which)
{
#if DEBUG_KP_COMMAND_HISTORY
    qCDebug(kpLogCommands) << "kpCommandHistoryBase::redoUpToNumber(" << which << ")";
#endif

    for (int i = 0;
         i <= which->data().toInt() && !m_redoCommandList.isEmpty ();
         i++)
    {
        redoInternal ();
    }

    trimCommandListsUpdateActions ();
}


// protected
QString kpCommandHistoryBase::undoActionText () const
{
    kpCommand *undoCommand = nextUndoCommand ();

    return (undoCommand) ? i18n ("&Undo: %1", undoCommand->name ()) : i18n ("&Undo");
}

// protected
QString kpCommandHistoryBase::redoActionText () const
{
    kpCommand *redoCommand = nextRedoCommand ();

    return (redoCommand) ? i18n ("&Redo: %1", redoCommand->name ()) : i18n ("&Redo");
}


// protected
QString kpCommandHistoryBase::undoActionToolTip () const
{
    kpCommand *undoCommand = nextUndoCommand ();

    return (undoCommand) ? i18n ("Undo: %1", undoCommand->name ()) : i18n ("Undo");
}

// protected
QString kpCommandHistoryBase::redoActionToolTip () const
{
    kpCommand *redoCommand = nextRedoCommand ();

    return (redoCommand) ? i18n ("Redo: %1", redoCommand->name ()) : i18n ("Redo");
}


// protected
void kpCommandHistoryBase::trimCommandListsUpdateActions ()
{
#if DEBUG_KP_COMMAND_HISTORY
    qCDebug(kpLogCommands) << "kpCommandHistoryBase::trimCommandListsUpdateActions()";
#endif

    trimCommandLists ();
    updateActions ();
}

//--------------------------------------------------------------------------------

// protected
void kpCommandHistoryBase::trimCommandList(QList<kpCommand *> &commandList)
{
#if DEBUG_KP_COMMAND_HISTORY
    qCDebug(kpLogCommands) << "kpCommandHistoryBase::trimCommandList()";
    QTime timer; timer.start ();

    qCDebug(kpLogCommands) << "\tsize=" << commandList.size()
               << "    undoMinLimit=" << m_undoMinLimit
               << " undoMaxLimit=" << m_undoMaxLimit
               << " undoMaxLimitSizeLimit=" << m_undoMaxLimitSizeLimit;
#endif
    if ( commandList.size() <= m_undoMinLimit )
    {
    #if DEBUG_KP_COMMAND_HISTORY
        qCDebug(kpLogCommands) << "\t\tsize under undoMinLimit - done";
    #endif
        return;
    }


#if DEBUG_KP_COMMAND_HISTORY && 0
    qCDebug(kpLogCommands) << "\tsize over undoMinLimit - iterating thru cmds:";
#endif

    QList <kpCommand *>::iterator it = commandList.begin ();
    int upto = 0;

    kpCommandSize::SizeType sizeSoFar = 0;

    while (it != commandList.end ())
    {
        bool advanceIt = true;

        if (sizeSoFar <= m_undoMaxLimitSizeLimit)
        {
            sizeSoFar += (*it)->size ();
        }

    #if DEBUG_KP_COMMAND_HISTORY && 0
        qCDebug(kpLogCommands) << "\t\t" << upto << ":"
                   << " name='" << (*it)->name ()
                   << "' size=" << (*it)->size ()
                   << "    sizeSoFar=" << sizeSoFar;
    #endif

        if (upto >= m_undoMinLimit)
        {
            if (upto >= m_undoMaxLimit ||
                sizeSoFar > m_undoMaxLimitSizeLimit)
            {
            #if DEBUG_KP_COMMAND_HISTORY && 0
                qCDebug(kpLogCommands) << "\t\t\tkill";
            #endif
                delete (*it);
                it = m_undoCommandList.erase (it);
                advanceIt = false;
            }
        }

        if (advanceIt) {
            it++;
        }
        upto++;
    }

#if DEBUG_KP_COMMAND_HISTORY
    qCDebug(kpLogCommands) << "\ttook " << timer.elapsed () << "ms";
#endif
}

//--------------------------------------------------------------------------------

// protected
void kpCommandHistoryBase::trimCommandLists ()
{
#if DEBUG_KP_COMMAND_HISTORY
    qCDebug(kpLogCommands) << "kpCommandHistoryBase::trimCommandLists()";
#endif

    trimCommandList(m_undoCommandList);
    trimCommandList(m_redoCommandList);

#if DEBUG_KP_COMMAND_HISTORY
    qCDebug(kpLogCommands) << "\tdocumentRestoredPosition=" << m_documentRestoredPosition
#endif
    if (m_documentRestoredPosition != INT_MAX)
    {
    #if DEBUG_KP_COMMAND_HISTORY
        qCDebug(kpLogCommands) << "\t\tundoCmdList.size=" << m_undoCommandList.size ()
                   << " redoCmdList.size=" << m_redoCommandList.size ();
    #endif
        if (m_documentRestoredPosition > static_cast<int> (m_redoCommandList.size ()) ||
            -m_documentRestoredPosition > static_cast<int> (m_undoCommandList.size ()))
        {
        #if DEBUG_KP_COMMAND_HISTORY
            qCDebug(kpLogCommands) << "\t\t\tinvalidate documentRestoredPosition";
        #endif
            m_documentRestoredPosition = INT_MAX;
        }
    }
}


static void populatePopupMenu (QMenu *popupMenu,
                               const QString &undoOrRedo,
                               const QList <kpCommand *> &commandList)
{
    if (!popupMenu) {
        return;
    }

    popupMenu->clear ();

    QList <kpCommand *>::const_iterator it = commandList.begin ();
    int i = 0;
    while (i < 10 && it != commandList.end ())
    {
        QAction *action = new QAction(i18n ("%1: %2", undoOrRedo, (*it)->name ()), popupMenu);
        action->setData(i);
        popupMenu->addAction (action);
        i++;
        it++;
    }

    if (it != commandList.end ())
    {
        // TODO: maybe have a scrollview show all the items instead, like KOffice in KDE 3
        // LOCOMPAT: should be centered text.
        popupMenu->addSection (i18np ("%1 more item", "%1 more items",
                                    commandList.size () - i));
    }
}


// protected
void kpCommandHistoryBase::updateActions ()
{
#if DEBUG_KP_COMMAND_HISTORY
    qCDebug(kpLogCommands) << "kpCommandHistoryBase::updateActions()";
#endif

    m_actionUndo->setEnabled (static_cast<bool> (nextUndoCommand ()));
    // Don't want to keep changing toolbar text.
    // TODO: As a bad side-effect, the menu doesn't have "Undo: <action>"
    //       anymore.  In any case, the KDE4 KToolBarPopupAction
    //       sucks in menus as it forces the clicking of a submenu.  IMO,
    //       there should be no submenu in the menu.
    //m_actionUndo->setText (undoActionText ());

    // But in icon mode, a tooltip with context is useful.
    m_actionUndo->setToolTip (undoActionToolTip ());
#if DEBUG_KP_COMMAND_HISTORY
    QTime timer; timer.start ();
#endif
    populatePopupMenu (m_actionUndo->menu (),
                       i18n ("Undo"),
                       m_undoCommandList);
#if DEBUG_KP_COMMAND_HISTORY
    qCDebug(kpLogCommands) << "\tpopuplatePopupMenu undo=" << timer.elapsed ()
               << "ms";
#endif

    m_actionRedo->setEnabled (static_cast<bool> (nextRedoCommand ()));
    // Don't want to keep changing toolbar text.
    // TODO: As a bad side-effect, the menu doesn't have "Undo: <action>"
    //       anymore.  In any case, the KDE4 KToolBarPopupAction
    //       sucks in menus as it forces the clicking of a submenu.  IMO,
    //       there should be no submenu in the menu.
    //m_actionRedo->setText (redoActionText ());

    // But in icon mode, a tooltip with context is useful.
    m_actionRedo->setToolTip (redoActionToolTip ());
#if DEBUG_KP_COMMAND_HISTORY
    timer.restart ();
#endif
    populatePopupMenu (m_actionRedo->menu (),
                       i18n ("Redo"),
                       m_redoCommandList);
#if DEBUG_KP_COMMAND_HISTORY
    qCDebug(kpLogCommands) << "\tpopuplatePopupMenu redo=" << timer.elapsed ()
               << "ms";
#endif
}


// public
kpCommand *kpCommandHistoryBase::nextUndoCommand () const
{
    if (m_undoCommandList.isEmpty ()) {
        return nullptr;
    }

    return m_undoCommandList.first ();
}

// public
kpCommand *kpCommandHistoryBase::nextRedoCommand () const
{
    if (m_redoCommandList.isEmpty ()) {
        return nullptr;
    }

    return m_redoCommandList.first ();
}


// public
void kpCommandHistoryBase::setNextUndoCommand (kpCommand *command)
{
#if DEBUG_KP_COMMAND_HISTORY
    qCDebug(kpLogCommands) << "kpCommandHistoryBase::setNextUndoCommand("<< command << ")";
#endif

    if (m_undoCommandList.isEmpty ()) {
        return;
    }

    delete *m_undoCommandList.begin ();
    *m_undoCommandList.begin () = command;

    trimCommandListsUpdateActions ();
}


// public slot virtual
void kpCommandHistoryBase::documentSaved ()
{
#if DEBUG_KP_COMMAND_HISTORY
    qCDebug(kpLogCommands) << "kpCommandHistoryBase::documentSaved()";
#endif

    m_documentRestoredPosition = 0;
}


