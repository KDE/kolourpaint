
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


#define DEBUG_KP_COMMAND_HISTORY 0


#include <kpcommandhistory.h>

#include <limits.h>

#include <qdatetime.h>

#include <kactionclasses.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>
#include <klocale.h>
#include <kpopupmenu.h>
#include <kstdaccel.h>
#include <kstdaction.h>

#include <kpdefs.h>
#include <kpdocument.h>
#include <kpmainwindow.h>
#include <kptool.h>


//template <typename T>
static void clearPointerList (QValueList <kpCommand *> *listPtr)
{
    if (!listPtr)
        return;

    for (QValueList <kpCommand *>::iterator it = listPtr->begin ();
         it != listPtr->end ();
         it++)
    {
        delete (*it);
    }

    listPtr->clear ();
}


//
// kpCommand
//

kpCommand::kpCommand (kpMainWindow *mainWindow)
    : m_mainWindow (mainWindow)
{
    if (!mainWindow)
        kdError () << "kpCommand::kpCommand() passed 0 mainWindow" << endl;
}

kpCommand::~kpCommand ()
{
}


// protected
kpMainWindow *kpCommand::mainWindow () const
{
    return m_mainWindow;
}


// protected
kpDocument *kpCommand::document () const
{
    return m_mainWindow ? m_mainWindow->document () : 0;
}

// protected
kpSelection *kpCommand::selection () const
{
    kpDocument *doc = document ();
    if (!doc)
        return 0;

    return doc->selection ();
}


// protected
kpViewManager *kpCommand::viewManager () const
{
    return m_mainWindow ? m_mainWindow->viewManager () : 0;
}


//
// kpNamedCommand
//

kpNamedCommand::kpNamedCommand (const QString &name, kpMainWindow *mainWindow)
    : kpCommand (mainWindow),
      m_name (name)
{
}

kpNamedCommand::~kpNamedCommand ()
{
}


// public virtual [base kpCommand]
QString kpNamedCommand::name () const
{
    return m_name;
}


//
// kpMacroCommand
//

struct kpMacroCommandPrivate
{
};

kpMacroCommand::kpMacroCommand (const QString &name, kpMainWindow *mainWindow)
    : kpNamedCommand (name, mainWindow),
      d (new kpMacroCommandPrivate ())
{
}

kpMacroCommand::~kpMacroCommand ()
{
    clearPointerList (&m_commandList);
    delete d;
}


// public virtual [base kpCommand]
int kpMacroCommand::size () const
{
#if DEBUG_KP_COMMAND_HISTORY && 0
    kdDebug () << "kpMacroCommand::size()" << endl;
#endif
    int s = 0;

#if DEBUG_KP_COMMAND_HISTORY && 0
    kdDebug () << "\tcalculating:" << endl;
#endif
    for (QValueList <kpCommand *>::const_iterator it = m_commandList.begin ();
         it != m_commandList.end ();
         it++)
    {
    #if DEBUG_KP_COMMAND_HISTORY && 0
        kdDebug () << "\t\tcurrentSize=" << s << " + "
                   << (*it)->name () << ".size=" << (*it)->size ()
                   << endl;
    #endif
        if (s > INT_MAX - (*it)->size ())
        {
        #if DEBUG_KP_COMMAND_HISTORY && 0
            kdDebug () << "\t\t\toverflow" << endl;
        #endif
            s = INT_MAX;
            break;
        }
        else
        {
            s += (*it)->size ();
        }
    }

#if DEBUG_KP_COMMAND_HISTORY && 0
    kdDebug () << "\treturning " << s << endl;
#endif
    return s;
}


// public virtual [base kpCommand]
void kpMacroCommand::execute ()
{
#if DEBUG_KP_COMMAND_HISTORY
    kdDebug () << "kpMacroCommand::execute()" << endl;
#endif
    for (QValueList <kpCommand *>::const_iterator it = m_commandList.begin ();
         it != m_commandList.end ();
         it++)
    {
    #if DEBUG_KP_COMMAND_HISTORY
        kdDebug () << "\texecuting " << (*it)->name () << endl;
    #endif
        (*it)->execute ();
    }
}

// public virtual [base kpCommand]
void kpMacroCommand::unexecute ()
{
#if DEBUG_KP_COMMAND_HISTORY
    kdDebug () << "kpMacroCommand::unexecute()" << endl;
#endif
    QValueList <kpCommand *>::const_iterator it = m_commandList.end ();
    it--;

    while (it != m_commandList.end ())
    {
    #if DEBUG_KP_COMMAND_HISTORY
        kdDebug () << "\tunexecuting " << (*it)->name () << endl;
    #endif
        (*it)->unexecute ();

        it--;
    }
}


// public
void kpMacroCommand::addCommand (kpCommand *command)
{
    m_commandList.push_back (command);
}


//
// kpCommandHistoryBase
//

struct kpCommandHistoryBasePrivate
{
};


kpCommandHistoryBase::kpCommandHistoryBase (bool doReadConfig,
                                            KActionCollection *ac)
    : d (new kpCommandHistoryBasePrivate ())
{
    m_actionUndo = new KToolBarPopupAction (undoActionText (),
        QString::fromLatin1 ("undo"),
        KStdAccel::shortcut (KStdAccel::Undo),
        this, SLOT (undo ()),
        ac, KStdAction::name (KStdAction::Undo));

    m_actionRedo = new KToolBarPopupAction (redoActionText (),
        QString::fromLatin1 ("redo"),
        KStdAccel::shortcut (KStdAccel::Redo),
        this, SLOT (redo ()),
        ac, KStdAction::name (KStdAction::Redo));


    m_actionUndo->setEnabled (false);
    m_actionRedo->setEnabled (false);


    connect (m_actionUndo->popupMenu (), SIGNAL (activated (int)),
             this, SLOT (undoUpToNumber (int)));
    connect (m_actionRedo->popupMenu (), SIGNAL (activated (int)),
             this, SLOT (redoUpToNumber (int)));


    m_undoMinLimit = 10;
    m_undoMaxLimit = 500;
    m_undoMaxLimitSizeLimit = 16 * 1048576;


    m_documentRestoredPosition = 0;


    if (doReadConfig)
        readConfig ();
}

kpCommandHistoryBase::~kpCommandHistoryBase ()
{
    clearPointerList (&m_undoCommandList);
    clearPointerList (&m_redoCommandList);

    delete d;
}


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
    kdDebug () << "kpCommandHistoryBase::setUndoMinLimit("
               << limit << ")"
               << endl;
#endif

    if (limit < 1 || limit > 5000/*"ought to be enough for anybody"*/)
    {
        kdError () << "kpCommandHistoryBase::setUndoMinLimit("
                   << limit << ")"
                   << endl;
        return;
    }

    if (limit == m_undoMinLimit)
        return;

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
    kdDebug () << "kpCommandHistoryBase::setUndoMaxLimit("
               << limit << ")"
               << endl;
#endif

    if (limit < 1 || limit > 5000/*"ought to be enough for anybody"*/)
    {
        kdError () << "kpCommandHistoryBase::setUndoMaxLimit("
                   << limit << ")"
                   << endl;
        return;
    }

    if (limit == m_undoMaxLimit)
        return;

    m_undoMaxLimit = limit;
    trimCommandListsUpdateActions ();
}


// public
int kpCommandHistoryBase::undoMaxLimitSizeLimit () const
{
    return m_undoMaxLimitSizeLimit;
}

// public
void kpCommandHistoryBase::setUndoMaxLimitSizeLimit (int sizeLimit)
{
#if DEBUG_KP_COMMAND_HISTORY
    kdDebug () << "kpCommandHistoryBase::setUndoMaxLimitSizeLimit("
               << sizeLimit << ")"
               << endl;
#endif

    if (sizeLimit < 0 ||
        sizeLimit > (500 * 1048576)/*"ought to be enough for anybody"*/)
    {
        kdError () << "kpCommandHistoryBase::setUndoMaxLimitSizeLimit("
                   << sizeLimit << ")"
                   << endl;
        return;
    }

    if (sizeLimit == m_undoMaxLimitSizeLimit)
        return;

    m_undoMaxLimitSizeLimit = sizeLimit;
    trimCommandListsUpdateActions ();
}


// public
void kpCommandHistoryBase::readConfig ()
{
#if DEBUG_KP_COMMAND_HISTORY
    kdDebug () << "kpCommandHistoryBase::readConfig()" << endl;
#endif
    KConfigGroupSaver cfgGroupSaver (kapp->config (), kpSettingsGroupUndoRedo);
    KConfigBase *cfg = cfgGroupSaver.config ();

    setUndoMinLimit (cfg->readNumEntry (kpSettingUndoMinLimit, undoMinLimit ()));
    setUndoMaxLimit (cfg->readNumEntry (kpSettingUndoMaxLimit, undoMaxLimit ()));
    setUndoMaxLimitSizeLimit (cfg->readNumEntry (kpSettingUndoMaxLimitSizeLimit,
                                                 undoMaxLimitSizeLimit ()));

    trimCommandListsUpdateActions ();
}

// public
void kpCommandHistoryBase::writeConfig ()
{
#if DEBUG_KP_COMMAND_HISTORY
    kdDebug () << "kpCommandHistoryBase::writeConfig()" << endl;
#endif
    KConfigGroupSaver cfgGroupSaver (kapp->config (), kpSettingsGroupUndoRedo);
    KConfigBase *cfg = cfgGroupSaver.config ();

    cfg->writeEntry (kpSettingUndoMinLimit, undoMinLimit ());
    cfg->writeEntry (kpSettingUndoMaxLimit, undoMaxLimit ());
    cfg->writeEntry (kpSettingUndoMaxLimitSizeLimit, undoMaxLimitSizeLimit ());

    cfg->sync ();
}


// public
void kpCommandHistoryBase::addCommand (kpCommand *command, bool execute)
{
#if DEBUG_KP_COMMAND_HISTORY
    kdDebug () << "kpCommandHistoryBase::addCommand("
               << command
               << ",execute=" << execute << ")"
               << endl;
#endif

    if (execute)
        command->execute ();

    m_undoCommandList.push_front (command);
    clearPointerList (&m_redoCommandList);

#if DEBUG_KP_COMMAND_HISTORY
    kdDebug () << "\tdocumentRestoredPosition=" << m_documentRestoredPosition
               << endl;
#endif
    if (m_documentRestoredPosition != INT_MAX)
    {
        if (m_documentRestoredPosition > 0)
            m_documentRestoredPosition = INT_MAX;
        else
            m_documentRestoredPosition--;
    #if DEBUG_KP_COMMAND_HISTORY
        kdDebug () << "\t\tdocumentRestoredPosition=" << m_documentRestoredPosition
                << endl;
    #endif
    }

    trimCommandListsUpdateActions ();
}

// public
void kpCommandHistoryBase::clear ()
{
#if DEBUG_KP_COMMAND_HISTORY
    kdDebug () << "kpCommandHistoryBase::clear()" << endl;
#endif

    clearPointerList (&m_undoCommandList);
    clearPointerList (&m_redoCommandList);

    m_documentRestoredPosition = 0;

    updateActions ();
}


// protected slot
void kpCommandHistoryBase::undoInternal ()
{
#if DEBUG_KP_COMMAND_HISTORY
    kdDebug () << "kpCommandHistoryBase::undoInternal()" << endl;
#endif

    kpCommand *undoCommand = nextUndoCommand ();
    if (!undoCommand)
        return;

    undoCommand->unexecute ();


    m_undoCommandList.erase (m_undoCommandList.begin ());
    m_redoCommandList.push_front (undoCommand);


#if DEBUG_KP_COMMAND_HISTORY
    kdDebug () << "\tdocumentRestoredPosition=" << m_documentRestoredPosition
               << endl;
#endif
    if (m_documentRestoredPosition != INT_MAX)
    {
        m_documentRestoredPosition++;
        if (m_documentRestoredPosition == 0)
            emit documentRestored ();
    #if DEBUG_KP_COMMAND_HISTORY
        kdDebug () << "\t\tdocumentRestoredPosition=" << m_documentRestoredPosition
                   << endl;
    #endif
    }
}

// protected slot
void kpCommandHistoryBase::redoInternal ()
{
#if DEBUG_KP_COMMAND_HISTORY
    kdDebug () << "kpCommandHistoryBase::redoInternal()" << endl;
#endif

    kpCommand *redoCommand = nextRedoCommand ();
    if (!redoCommand)
        return;

    redoCommand->execute ();


    m_redoCommandList.erase (m_redoCommandList.begin ());
    m_undoCommandList.push_front (redoCommand);


#if DEBUG_KP_COMMAND_HISTORY
    kdDebug () << "\tdocumentRestoredPosition=" << m_documentRestoredPosition
               << endl;
#endif
    if (m_documentRestoredPosition != INT_MAX)
    {
        m_documentRestoredPosition--;
        if (m_documentRestoredPosition == 0)
            emit documentRestored ();
    #if DEBUG_KP_COMMAND_HISTORY
        kdDebug () << "\t\tdocumentRestoredPosition=" << m_documentRestoredPosition
                   << endl;
    #endif
    }
}


// public slot virtual
void kpCommandHistoryBase::undo ()
{
#if DEBUG_KP_COMMAND_HISTORY
    kdDebug () << "kpCommandHistoryBase::undo()" << endl;
#endif

    undoInternal ();
    trimCommandListsUpdateActions ();
}

// public slot virtual
void kpCommandHistoryBase::redo ()
{
#if DEBUG_KP_COMMAND_HISTORY
    kdDebug () << "kpCommandHistoryBase::redo()" << endl;
#endif

    redoInternal ();
    trimCommandListsUpdateActions ();
}


// public slot virtual
void kpCommandHistoryBase::undoUpToNumber (int which)
{
#if DEBUG_KP_COMMAND_HISTORY
    kdDebug () << "kpCommandHistoryBase::undoUpToNumber(" << which << ")" << endl;
#endif

    for (int i = 0;
         i <= which && !m_undoCommandList.isEmpty ();
         i++)
    {
        undoInternal ();
    }

    trimCommandListsUpdateActions ();
}

// public slot virtual
void kpCommandHistoryBase::redoUpToNumber (int which)
{
#if DEBUG_KP_COMMAND_HISTORY
    kdDebug () << "kpCommandHistoryBase::redoUpToNumber(" << which << ")" << endl;
#endif

    for (int i = 0;
         i <= which && !m_redoCommandList.isEmpty ();
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

    if (undoCommand)
        return i18n ("&Undo: %1").arg (undoCommand->name ());
    else
        return i18n ("&Undo");
}

// protected
QString kpCommandHistoryBase::redoActionText () const
{
    kpCommand *redoCommand = nextRedoCommand ();

    if (redoCommand)
        return i18n ("&Redo: %1").arg (redoCommand->name ());
    else
        return i18n ("&Redo");
}


// protected
void kpCommandHistoryBase::trimCommandListsUpdateActions ()
{
#if DEBUG_KP_COMMAND_HISTORY
    kdDebug () << "kpCommandHistoryBase::trimCommandListsUpdateActions()" << endl;
#endif

    trimCommandLists ();
    updateActions ();
}

// protected
void kpCommandHistoryBase::trimCommandList (QValueList <kpCommand *> *commandList)
{
#if DEBUG_KP_COMMAND_HISTORY
    kdDebug () << "kpCommandHistoryBase::trimCommandList()" << endl;
    QTime timer; timer.start ();
#endif

    if (!commandList)
    {
        kdError () << "kpCommandHistoryBase::trimCommandList() passed 0 commandList"
                   << endl;
        return;
    }


#if DEBUG_KP_COMMAND_HISTORY
    kdDebug () << "\tsize=" << commandList->size ()
               << "    undoMinLimit=" << m_undoMinLimit
               << " undoMaxLimit=" << m_undoMaxLimit
               << " undoMaxLimitSizeLimit=" << m_undoMaxLimitSizeLimit
               << endl;
#endif
    if ((int) commandList->size () <= m_undoMinLimit)
    {
    #if DEBUG_KP_COMMAND_HISTORY
        kdDebug () << "\t\tsize under undoMinLimit - done" << endl;
    #endif
        return;
    }


#if DEBUG_KP_COMMAND_HISTORY && 0
    kdDebug () << "\tsize over undoMinLimit - iterating thru cmds:" << endl;
#endif

    QValueList <kpCommand *>::iterator it = commandList->begin ();
    int upto = 0;

    int sizeSoFar = 0;

    while (it != commandList->end ())
    {
        bool advanceIt = true;

        if (sizeSoFar <= m_undoMaxLimitSizeLimit)
        {
            if (sizeSoFar > INT_MAX - (*it)->size ())
                sizeSoFar = INT_MAX;
            else
                sizeSoFar += (*it)->size ();
        }

    #if DEBUG_KP_COMMAND_HISTORY && 0
        kdDebug () << "\t\t" << upto << ":"
                   << " name='" << (*it)->name ()
                   << "' size=" << (*it)->size ()
                   << "    sizeSoFar=" << sizeSoFar
                   << endl;
    #endif

        if (upto >= m_undoMinLimit)
        {
            if (upto >= m_undoMaxLimit ||
                sizeSoFar > m_undoMaxLimitSizeLimit)
            {
            #if DEBUG_KP_COMMAND_HISTORY && 0
                kdDebug () << "\t\t\tkill" << endl;
            #endif
                delete (*it);
                it = m_undoCommandList.erase (it);
                advanceIt = false;
            }
        }

        if (advanceIt)
            it++;
        upto++;
    }

#if DEBUG_KP_COMMAND_HISTORY
    kdDebug () << "\ttook " << timer.elapsed () << "ms" << endl;
#endif
}

// protected
void kpCommandHistoryBase::trimCommandLists ()
{
#if DEBUG_KP_COMMAND_HISTORY
    kdDebug () << "kpCommandHistoryBase::trimCommandLists()" << endl;
#endif

    trimCommandList (&m_undoCommandList);
    trimCommandList (&m_redoCommandList);

#if DEBUG_KP_COMMAND_HISTORY
    kdDebug () << "\tdocumentRestoredPosition=" << m_documentRestoredPosition
               << endl;
#endif
    if (m_documentRestoredPosition != INT_MAX)
    {
    #if DEBUG_KP_COMMAND_HISTORY
        kdDebug () << "\t\tundoCmdList.size=" << m_undoCommandList.size ()
                   << " redoCmdList.size=" << m_redoCommandList.size ()
                   << endl;
    #endif
        if (m_documentRestoredPosition > (int) m_redoCommandList.size () ||
            -m_documentRestoredPosition > (int) m_undoCommandList.size ())
        {
        #if DEBUG_KP_COMMAND_HISTORY
            kdDebug () << "\t\t\tinvalidate documentRestoredPosition" << endl;
        #endif
            m_documentRestoredPosition = INT_MAX;
        }
    }
}


static void populatePopupMenu (KPopupMenu *popupMenu,
                               const QString &undoOrRedo,
                               const QValueList <kpCommand *> &commandList)
{
    if (!popupMenu)
        return;

    popupMenu->clear ();

    QValueList <kpCommand *>::const_iterator it = commandList.begin ();
    int i = 0;
    while (i < 10 && it != commandList.end ())
    {
        popupMenu->insertItem (i18n ("%1: %2").arg (undoOrRedo).arg ((*it)->name ()), i/*id*/);
        i++, it++;
    }

    if (it != commandList.end ())
    {
        // TODO: maybe have a scrollview show all the items instead
        KPopupTitle *title = new KPopupTitle (popupMenu);
        title->setTitle (i18n ("%n more item", "%n more items",
                               commandList.size () - i));

        popupMenu->insertItem (title);
    }
}


// protected
void kpCommandHistoryBase::updateActions ()
{
#if DEBUG_KP_COMMAND_HISTORY
    kdDebug () << "kpCommandHistoryBase::updateActions()" << endl;
#endif

    m_actionUndo->setEnabled ((bool) nextUndoCommand ());
    m_actionUndo->setText (undoActionText ());
#if DEBUG_KP_COMMAND_HISTORY
    QTime timer; timer.start ();
#endif
    populatePopupMenu (m_actionUndo->popupMenu (),
                       i18n ("Undo"),
                       m_undoCommandList);
#if DEBUG_KP_COMMAND_HISTORY
    kdDebug () << "\tpopuplatePopupMenu undo=" << timer.elapsed ()
               << "ms" << endl;;
#endif

    m_actionRedo->setEnabled ((bool) nextRedoCommand ());
    m_actionRedo->setText (redoActionText ());
#if DEBUG_KP_COMMAND_HISTORY
    timer.restart ();
#endif
    populatePopupMenu (m_actionRedo->popupMenu (),
                       i18n ("Redo"),
                       m_redoCommandList);
#if DEBUG_KP_COMMAND_HISTORY
    kdDebug () << "\tpopuplatePopupMenu redo=" << timer.elapsed ()
               << "ms" << endl;
#endif
}


// public
kpCommand *kpCommandHistoryBase::nextUndoCommand () const
{
    if (m_undoCommandList.isEmpty ())
        return 0;

    return m_undoCommandList.first ();
}

// public
kpCommand *kpCommandHistoryBase::nextRedoCommand () const
{
    if (m_redoCommandList.isEmpty ())
        return 0;

    return m_redoCommandList.first ();
}


// public
void kpCommandHistoryBase::setNextUndoCommand (kpCommand *command)
{
#if DEBUG_KP_COMMAND_HISTORY
    kdDebug () << "kpCommandHistoryBase::setNextUndoCommand("
               << command
               << ")"
               << endl;
#endif

    if (m_undoCommandList.isEmpty ())
        return;


    delete m_undoCommandList [0];
    m_undoCommandList [0] = command;


    trimCommandListsUpdateActions ();
}


// public slot virtual
void kpCommandHistoryBase::documentSaved ()
{
#if DEBUG_KP_COMMAND_HISTORY
    kdDebug () << "kpCommandHistoryBase::documentSaved()" << endl;
#endif

    m_documentRestoredPosition = 0;
}


//
// kpCommandHistory
//

kpCommandHistory::kpCommandHistory (bool doReadConfig, kpMainWindow *mainWindow)
    : kpCommandHistoryBase (doReadConfig, mainWindow->actionCollection ()),
      m_mainWindow (mainWindow)
{
}

kpCommandHistory::~kpCommandHistory ()
{
}


// public slot virtual [base KCommandHistory]
void kpCommandHistory::undo ()
{
#if DEBUG_KP_COMMAND_HISTORY
    kdDebug () << "kpCommandHistory::undo() CALLED!" << endl;
#endif
    if (m_mainWindow && m_mainWindow->toolHasBegunShape ())
    {
    #if DEBUG_KP_COMMAND_HISTORY
        kdDebug () << "\thas begun shape - cancel draw" << endl;
    #endif
        m_mainWindow->tool ()->cancelShapeInternal ();
    }
    else
        kpCommandHistoryBase::undo ();
}

// public slot virtual [base KCommandHistory]
void kpCommandHistory::redo ()
{
    if (m_mainWindow && m_mainWindow->toolHasBegunShape ())
    {
        // Not completely obvious but what else can we do?
        //
        // Ignoring the request would not be intuitive for tools like
        // Polygon & Polyline (where it's not always apparent to the user
        // that s/he's still drawing a shape even though the mouse isn't
        // down).
        m_mainWindow->tool ()->cancelShapeInternal ();
    }
    else
        kpCommandHistoryBase::redo ();
}

#include <kpcommandhistory.moc>
