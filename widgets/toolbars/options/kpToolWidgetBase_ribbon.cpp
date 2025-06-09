#include "kpToolWidgetBase.h"

#include "kpDefs.h"

#include <KConfig>
#include <KConfigGroup>
#include <KSharedConfig>
#include "kpLogCategories.h"

#include <QItemSelectionModel>

kpToolWidgetBase_Ribbon::kpToolWidgetBase_Ribbon (QWidget *parent, const QString &name) :
    kpToolWidgetBase(parent)
{
    m_gallery = new SARibbonGallery(this);
    m_gallery->setSizePolicy (QSizePolicy::Maximum, QSizePolicy::Maximum);
    m_galleryGroup = m_gallery->addGalleryGroup();

    m_galleryGroup->setupGroupModel();

    connect(m_galleryGroup->selectionModel(), &QItemSelectionModel::selectionChanged, this, &kpToolWidgetBase_Ribbon::callOptionSelected);

    setObjectName (name);
}

kpToolWidgetBase_Ribbon::~kpToolWidgetBase_Ribbon () = default;

void kpToolWidgetBase_Ribbon::addOption (const QPixmap &pixmap, const QString &toolTip)
{
    m_galleryGroup->addItem(toolTip, pixmap);
}

void kpToolWidgetBase_Ribbon::startNewOptionRow ()
{
    Q_ASSERT(false);    // Code that hasn't been refactored away from using this function will crash. Even if it thinks it's using the old brush selection widget, it must treat it as a 1d widget and not a 2d widget.
}

void kpToolWidgetBase_Ribbon::finishConstruction (int fallBackRow, int fallBackCol)
{
}

QPair <int, int> kpToolWidgetBase_Ribbon::defaultSelectedRowAndCol () const
{
    return qMakePair(0, 0);
}

int kpToolWidgetBase_Ribbon::defaultSelectedRow () const
{
    return defaultSelectedRowAndCol ().first;
}

int kpToolWidgetBase_Ribbon::defaultSelectedCol () const
{
    return defaultSelectedRowAndCol ().second;
}

void kpToolWidgetBase_Ribbon::saveSelectedAsDefault () const
{
    if (objectName ().isEmpty ()) {
        return;
    }

    KConfigGroup cfg (KSharedConfig::openConfig (), QStringLiteral(kpSettingsGroupTools));

    cfg.writeEntry (objectName () + QLatin1String (" Row"), m_selectedRow);
    cfg.writeEntry (objectName () + QLatin1String (" Col"), m_selectedCol);
    cfg.sync ();
}

void kpToolWidgetBase_Ribbon::relayoutOptions ()
{
}

int kpToolWidgetBase_Ribbon::selectedRow () const
{
    return this->selected();
}

int kpToolWidgetBase_Ribbon::selectedCol () const
{
    return 0;
}

int kpToolWidgetBase_Ribbon::selected () const
{
    if (m_galleryGroup->selectionModel()->selectedIndexes().count() > 0)
        return m_galleryGroup->selectionModel()->selectedIndexes().at(0).row();
    else
        return -1;
}

bool kpToolWidgetBase_Ribbon::hasPreviousOption (int *row, int *col) const
{
    int sel = this->selected();

    if (sel == -1 || sel == 0)
    {
        *row = *col = -1;
        return false;
    }
    else
    {
        *row = sel - 1;
        *col = 0;
        return true;
    }
}

bool kpToolWidgetBase_Ribbon::hasNextOption (int *row, int *col) const
{
    int sel = selected();
    int count = m_galleryGroup->groupModel()->rowCount(QModelIndex());

    if (sel == -1 || sel == count - 1)
    {
        *row = *col = -1;
        return false;
    }
    else
    {
        *row = sel + 1;
        *col = 0;
        return true;
    }
}

// public slot virtual
bool kpToolWidgetBase_Ribbon::setSelected (int row, int col, bool saveAsDefault)
{
    setSelected(row, col);

    if (saveAsDefault)
        saveSelectedAsDefault();

    return true;
}

// public slot
bool kpToolWidgetBase_Ribbon::setSelected (int row, int col)
{
    m_galleryGroup->selectionModel()->clearSelection();
    m_galleryGroup->selectionModel()->select(
        m_galleryGroup->groupModel()->index(row, col, QModelIndex()),
        QItemSelectionModel::Select
    );

    Q_EMIT optionSelected(row, col);

    return true;
}

// public slot
bool kpToolWidgetBase_Ribbon::selectPreviousOption ()
{
    int newRow, newCol;
    if (!hasPreviousOption (&newRow, &newCol)) {
        return false;
    }

    return setSelected (newRow, newCol);
}

// public slot
bool kpToolWidgetBase_Ribbon::selectNextOption ()
{
    int newRow, newCol;
    if (!hasNextOption (&newRow, &newCol)) {
        return false;
    }

    return setSelected (newRow, newCol);
}

void kpToolWidgetBase_Ribbon::callOptionSelected()
{
    Q_EMIT optionSelected(selectedRow(), selectedCol());
}

// protected virtual [base QWidget]
bool kpToolWidgetBase_Ribbon::event (QEvent *e)
{
    return QWidget::event (e);
}

// protected virtual [base QWidget]
void kpToolWidgetBase_Ribbon::mousePressEvent (QMouseEvent *e)
{
    // delete me
}

// protected virtual [base QWidget]
void kpToolWidgetBase_Ribbon::paintEvent (QPaintEvent *e)
{
    // Draw frame first.
    QFrame::paintEvent (e);
}
