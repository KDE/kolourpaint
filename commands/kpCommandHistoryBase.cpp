
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


#include <kpCommandHistoryBase.h>

#include <climits>

#include <qdatetime.h>
#include <qlinkedlist.h>

#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kmenu.h>
#include <kstandardshortcut.h>
#include <kstandardaction.h>
#include <ktoolbarpopupaction.h>
#include <kactioncollection.h>
#include <kconfiggroup.h>

#include <kpCommand.h>
#include <kpCommandEnvironment.h>
#include <kpDefs.h>
#include <kpDocument.h>
#include <kpMainWindow.h>
#include <kpTool.h>


//template <typename T>
static void ClearPointerList (QLinkedList <kpCommand *> *listPtr)
{
    if (!listPtr)
        return;

    qDeleteAll (listPtr->begin (), listPtr->end ());

    listPtr->clear ();
}


struct kpCommandHistoryBasePrivate
{
};


kpCommandHistoryBase::kpCommandHistoryBase (bool doReadConfig,
                                            KActionCollection *ac)
    : d (new kpCommandHistoryBasePrivate ())
{
    m_actionUndo = new KToolBarPopupAction (KIcon ("edit-undo"), undoActionText (), this);
    ac->addAction (KStandardAction::name (KStandardAction::Undo), m_actionUndo);
    m_actionUndo->setShortcuts (KStandardShortcut::shortcut (KStandardShortcut::Undo));
    connect (m_actionUndo, SIGNAL(triggered(bool)), this, SLOT (undo ()));

    m_actionRedo = new KToolBarPopupAction (KIcon ("edit-redo"), redoActionText (), this);
    ac->addAction (KStandardAction::name (KStandardAction::Redo), m_actionRedo);
    m_actionRedo->setShortcuts (KStandardShortcut::shortcut (KStandardShortcut::Redo));
    connect (m_actionRedo, SIGNAL(triggered(bool)), this, SLOT (redo ()));


    m_actionUndo->setEnabled (false);
    m_actionRedo->setEnabled (false);


    connect (m_actionUndo->menu (), SIGNAL (triggered (QAction *)),
             this, SLOT (undoUpToNumber (QAction *)));
    connect (m_actionRedo->menu (), SIGNAL (triggered (QAction *)),
             this, SLOT (redoUpToNumber (QAction *)));


    m_undoMinLimit = 10;
    m_undoMaxLimit = 500;
    m_undoMaxLimitSizeLimit = 16 * 1048576;


    m_documentRestoredPosition = 0;


    if (doReadConfig)
        readConfig ();
}

kpCommandHistoryBase::~kpCommandHistoryBase ()
{
    ::ClearPointerList (&m_undoCommandList);
    ::ClearPointerList (&m_redoCommandList);

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
    kDebug () << "kpCommandHistoryBase::setUndoMinLimit("
               << limit << ")"
               << endl;
#endif

    if (limit < 1 || limit > 5000/*"ought to be enough for anybody"*/)
    {
        kError () << "kpCommandHistoryBase::setUndoMinLimit("
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
    kDebug () << "kpCommandHistoryBase::setUndoMaxLimit("
               << limit << ")"
               << endl;
#endif

    if (limit < 1 || limit > 5000/*"ought to be enough for anybody"*/)
    {
        kError () << "kpCommandHistoryBase::setUndoMaxLimit("
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
kpCommandSize::SizeType kpCommandHistoryBase::undoMaxLimitSizeLimit () const
{
    return m_undoMaxLimitSizeLimit;
}

// public
void kpCommandHistoryBase::setUndoMaxLimitSizeLimit (kpCommandSize::SizeType sizeLimit)
{
#if DEBUG_KP_COMMAND_HISTORY
    kDebug () << "kpCommandHistoryBase::setUndoMaxLimitSizeLimit("
               << sizeLimit << ")"
               << endl;
#endif

    if (sizeLimit < 0 ||
        sizeLimit > (500 * 1048576)/*"ought to be enough for anybody"*/)
    {
        kError () << "kpCommandHistoryBase::setUndoMaxLimitSizeLimit("
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
    kDebug () << "kpCommandHistoryBase::readConfig()";
#endif
    KConfigGroup cfg (KGlobal::config (), kpSettingsGroupUndoRedo);

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
    kDebug () << "kpCommandHistoryBase::writeConfig()";
#endif
    KConfigGroup cfg (KGlobal::config (), kpSettingsGroupUndoRedo);

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
    kDebug () << "kpCommandHistoryBase::addCommand("
               << command
               << ",execute=" << execute << ")"
               << endl;
#endif

    if (execute)
        command->execute ();

    m_undoCommandList.push_front (command);
    ::ClearPointerList (&m_redoCommandList);

#if DEBUG_KP_COMMAND_HISTORY
    kDebug () << "\tdocumentRestoredPosition=" << m_documentRestoredPosition
               << endl;
#endif
    if (m_documentRestoredPosition != INT_MAX)
    {
        if (m_documentRestoredPosition > 0)
            m_documentRestoredPosition = INT_MAX;
        else
            m_documentRestoredPosition--;
    #if DEBUG_KP_COMMAND_HISTORY
        kDebug () << "\t\tdocumentRestoredPosition=" << m_documentRestoredPosition
                << endl;
    #endif
    }

    trimCommandListsUpdateActions ();
}

// public
void kpCommandHistoryBase::clear ()
{
#if DEBUG_KP_COMMAND_HISTORY
    kDebug () << "kpCommandHistoryBase::clear()";
#endif

    ::ClearPointerList (&m_undoCommandList);
    ::ClearPointerList (&m_redoCommandList);

    m_documentRestoredPosition = 0;

    updateActions ();
}

//---------------------------------------------------------------------

// protected slot
void kpCommandHistoryBase::undoInternal ()
{
#if DEBUG_KP_COMMAND_HISTORY
    kDebug () << "kpCommandHistoryBase::undoInternal()";
#endif

    kpCommand *undoCommand = nextUndoCommand ();
    if (!undoCommand)
        return;

    undoCommand->unexecute ();


    m_undoCommandList.erase (m_undoCommandList.begin ());
    m_redoCommandList.push_front (undoCommand);


#if DEBUG_KP_COMMAND_HISTORY
    kDebug () << "\tdocumentRestoredPosition=" << m_documentRestoredPosition
               << endl;
#endif
    if (m_documentRestoredPosition != INT_MAX)
    {
        m_documentRestoredPosition++;
        if (m_documentRestoredPosition == 0)
            emit documentRestored ();
    #if DEBUG_KP_COMMAND_HISTORY
        kDebug () << "\t\tdocumentRestoredPosition=" << m_documentRestoredPosition
                   << endl;
    #endif
    }
}

//---------------------------------------------------------------------

// protected slot
void kpCommandHistoryBase::redoInternal ()
{
#if DEBUG_KP_COMMAND_HISTORY
    kDebug () << "kpCommandHistoryBase::redoInternal()";
#endif

    kpCommand *redoCommand = nextRedoCommand ();
    if (!redoCommand)
        return;

    redoCommand->execute ();


    m_redoCommandList.erase (m_redoCommandList.begin ());
    m_undoCommandList.push_front (redoCommand);


#if DEBUG_KP_COMMAND_HISTORY
    kDebug () << "\tdocumentRestoredPosition=" << m_documentRestoredPosition
               << endl;
#endif
    if (m_documentRestoredPosition != INT_MAX)
    {
        m_documentRestoredPosition--;
        if (m_documentRestoredPosition == 0)
            emit documentRestored ();
    #if DEBUG_KP_COMMAND_HISTORY
        kDebug () << "\t\tdocumentRestoredPosition=" << m_documentRestoredPosition
                   << endl;
    #endif
    }
}

//---------------------------------------------------------------------

// public slot virtual
void kpCommandHistoryBase::undo ()
{
#if DEBUG_KP_COMMAND_HISTORY
    kDebug () << "kpCommandHistoryBase::undo()";
#endif

    undoInternal ();
    trimCommandListsUpdateActions ();
}

//---------------------------------------------------------------------

// public slot virtual
void kpCommandHistoryBase::redo ()
{
#if DEBUG_KP_COMMAND_HISTORY
    kDebug () << "kpCommandHistoryBase::redo()";
#endif

    redoInternal ();
    trimCommandListsUpdateActions ();
}

//---------------------------------------------------------------------

// public slot virtual
void kpCommandHistoryBase::undoUpToNumber (QAction *which)
{
#if DEBUG_KP_COMMAND_HISTORY
    kDebug () << "kpCommandHistoryBase::undoUpToNumber(" << which << ")";
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
    kDebug () << "kpCommandHistoryBase::redoUpToNumber(" << which << ")";
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

    if (undoCommand)
        return i18n ("&Undo: %1", undoCommand->name ());
    else
        return i18n ("&Undo");
}

// protected
QString kpCommandHistoryBase::redoActionText () const
{
    kpCommand *redoCommand = nextRedoCommand ();

    if (redoCommand)
        return i18n ("&Redo: %1", redoCommand->name ());
    else
        return i18n ("&Redo");
}


// protected
QString kpCommandHistoryBase::undoActionToolTip () const
{
    kpCommand *undoCommand = nextUndoCommand ();

    if (undoCommand)
        return i18n ("Undo: %1", undoCommand->name ());
    else
        return i18n ("Undo");
}

// protected
QString kpCommandHistoryBase::redoActionToolTip () const
{
    kpCommand *redoCommand = nextRedoCommand ();

    if (redoCommand)
        return i18n ("Redo: %1", redoCommand->name ());
    else
        return i18n ("Redo");
}


// protected
void kpCommandHistoryBase::trimCommandListsUpdateActions ()
{
#if DEBUG_KP_COMMAND_HISTORY
    kDebug () << "kpCommandHistoryBase::trimCommandListsUpdateActions()";
#endif

    trimCommandLists ();
    updateActions ();
}

// protected
void kpCommandHistoryBase::trimCommandList (QLinkedList <kpCommand *> *commandList)
{
#if DEBUG_KP_COMMAND_HISTORY
    kDebug () << "kpCommandHistoryBase::trimCommandList()";
    QTime timer; timer.start ();
#endif

    if (!commandList)
    {
        kError () << "kpCommandHistoryBase::trimCommandList() passed 0 commandList"
                   << endl;
        return;
    }


#if DEBUG_KP_COMMAND_HISTORY
    kDebug () << "\tsize=" << commandList->size ()
               << "    undoMinLimit=" << m_undoMinLimit
               << " undoMaxLimit=" << m_undoMaxLimit
               << " undoMaxLimitSizeLimit=" << m_undoMaxLimitSizeLimit
               << endl;
#endif
    if ((int) commandList->size () <= m_undoMinLimit)
    {
    #if DEBUG_KP_COMMAND_HISTORY
        kDebug () << "\t\tsize under undoMinLimit - done";
    #endif
        return;
    }


#if DEBUG_KP_COMMAND_HISTORY && 0
    kDebug () << "\tsize over undoMinLimit - iterating thru cmds:";
#endif

    QLinkedList <kpCommand *>::iterator it = commandList->begin ();
    int upto = 0;

    kpCommandSize::SizeType sizeSoFar = 0;

    while (it != commandList->end ())
    {
        bool advanceIt = true;

        if (sizeSoFar <= m_undoMaxLimitSizeLimit)
        {
            sizeSoFar += (*it)->size ();
        }

    #if DEBUG_KP_COMMAND_HISTORY && 0
        kDebug () << "\t\t" << upto << ":"
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
                kDebug () << "\t\t\tkill";
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
    kDebug () << "\ttook " << timer.elapsed () << "ms";
#endif
}

// protected
void kpCommandHistoryBase::trimCommandLists ()
{
#if DEBUG_KP_COMMAND_HISTORY
    kDebug () << "kpCommandHistoryBase::trimCommandLists()";
#endif

    trimCommandList (&m_undoCommandList);
    trimCommandList (&m_redoCommandList);

#if DEBUG_KP_COMMAND_HISTORY
    kDebug () << "\tdocumentRestoredPosition=" << m_documentRestoredPosition
               << endl;
#endif
    if (m_documentRestoredPosition != INT_MAX)
    {
    #if DEBUG_KP_COMMAND_HISTORY
        kDebug () << "\t\tundoCmdList.size=" << m_undoCommandList.size ()
                   << " redoCmdList.size=" << m_redoCommandList.size ()
                   << endl;
    #endif
        if (m_documentRestoredPosition > (int) m_redoCommandList.size () ||
            -m_documentRestoredPosition > (int) m_undoCommandList.size ())
        {
        #if DEBUG_KP_COMMAND_HISTORY
            kDebug () << "\t\t\tinvalidate documentRestoredPosition";
        #endif
            m_documentRestoredPosition = INT_MAX;
        }
    }
}


static void populatePopupMenu (KMenu *popupMenu,
                               const QString &undoOrRedo,
                               const QLinkedList <kpCommand *> &commandList)
{
    if (!popupMenu)
        return;

    popupMenu->clear ();

    QLinkedList <kpCommand *>::const_iterator it = commandList.begin ();
    int i = 0;
    while (i < 10 && it != commandList.end ())
    {
        QAction *action = new QAction(i18n ("%1: %2", undoOrRedo, (*it)->name ()), popupMenu);
        action->setData(i);
        popupMenu->addAction (action);
        i++, it++;
    }

    if (it != commandList.end ())
    {
        // TODO: maybe have a scrollview show all the items instead, like KOffice in KDE 3
        // LOCOMPAT: should be centered text.
        popupMenu->addTitle (i18np ("%1 more item", "%1 more items",
                                    commandList.size () - i));
    }
}


// protected
void kpCommandHistoryBase::updateActions ()
{
#if DEBUG_KP_COMMAND_HISTORY
    kDebug () << "kpCommandHistoryBase::updateActions()";
#endif

    m_actionUndo->setEnabled ((bool) nextUndoCommand ());
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
    populatePopupMenu (qobject_cast<KMenu*> (m_actionUndo->menu ()),
                       i18n ("Undo"),
                       m_undoCommandList);
#if DEBUG_KP_COMMAND_HISTORY
    kDebug () << "\tpopuplatePopupMenu undo=" << timer.elapsed ()
               << "ms" << endl;;
#endif

    m_actionRedo->setEnabled ((bool) nextRedoCommand ());
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
    populatePopupMenu (qobject_cast<KMenu*> (m_actionRedo->menu ()),
                       i18n ("Redo"),
                       m_redoCommandList);
#if DEBUG_KP_COMMAND_HISTORY
    kDebug () << "\tpopuplatePopupMenu redo=" << timer.elapsed ()
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
    kDebug () << "kpCommandHistoryBase::setNextUndoCommand("
               << command
               << ")"
               << endl;
#endif

    if (m_undoCommandList.isEmpty ())
        return;


    delete *m_undoCommandList.begin ();
    *m_undoCommandList.begin () = command;


    trimCommandListsUpdateActions ();
}


// public slot virtual
void kpCommandHistoryBase::documentSaved ()
{
#if DEBUG_KP_COMMAND_HISTORY
    kDebug () << "kpCommandHistoryBase::documentSaved()";
#endif

    m_documentRestoredPosition = 0;
}


#include <kpCommandHistoryBase.moc>
