#include "kpToolWidgetBase.h"

#include "kpDefs.h"

#include <KConfig>
#include <KConfigGroup>
#include <KSharedConfig>
#include "kpLogCategories.h"

#include <QItemSelectionModel>

kpToolWidgetBase::kpToolWidgetBase (QWidget *parent, const QString &name) :
    SARibbonGallery(parent)
{
    // m_gallery->setSizePolicy (QSizePolicy::Maximum, QSizePolicy::Maximum);
    m_galleryGroup = SARibbonGallery::addGalleryGroup();
    m_galleryGroup->setGroupTitle(name);
    // m_galleryGroup->setDisplayRow(SARibbonGalleryGroup::DisplayRow::DisplayTwoRow);

    m_galleryGroup->setupGroupModel();

    connect(this, &SARibbonGallery::triggered, this, &kpToolWidgetBase::callOptionSelected);
    // connect(m_galleryGroup->selectionModel(), &QItemSelectionModel::selectionChanged, this, &kpToolWidgetBase::callOptionSelected);

    setObjectName (name);
}

kpToolWidgetBase::~kpToolWidgetBase () = default;

void kpToolWidgetBase::addOption (const QPixmap &pixmap, const QString &toolTip)
{
    m_galleryGroup->addItem(toolTip, pixmap);
    m_galleryGroup->recalcGridSize();
}

void kpToolWidgetBase::startNewOptionRow ()
{
    Q_ASSERT(false);    // Code that hasn't been refactored away from using this function will crash. Even if it thinks it's using the old brush selection widget, it must treat it as a 1d widget and not a 2d widget.
}

void kpToolWidgetBase::finishConstruction (int fallBackRow, int fallBackCol)
{
}

QPair <int, int> kpToolWidgetBase::defaultSelectedRowAndCol () const
{
    return qMakePair(0, 0);
}

int kpToolWidgetBase::defaultSelectedRow () const
{
    return defaultSelectedRowAndCol ().first;
}

int kpToolWidgetBase::defaultSelectedCol () const
{
    return defaultSelectedRowAndCol ().second;
}

void kpToolWidgetBase::saveSelectedAsDefault () const
{
    if (objectName ().isEmpty ()) {
        return;
    }

    KConfigGroup cfg (KSharedConfig::openConfig (), QStringLiteral(kpSettingsGroupTools));

    cfg.writeEntry (objectName () + QLatin1String (" Row"), m_selectedRow);
    cfg.writeEntry (objectName () + QLatin1String (" Col"), m_selectedCol);
    cfg.sync ();
}

void kpToolWidgetBase::relayoutOptions ()
{
}

int kpToolWidgetBase::selectedRow () const
{
    return this->selected();
}

int kpToolWidgetBase::selectedCol () const
{
    return 0;
}

int kpToolWidgetBase::selected () const
{
    fprintf(stderr, "\tn selected: %d\n", m_galleryGroup->selectionModel()->selectedIndexes().count());

    if (m_galleryGroup->selectionModel()->selectedIndexes().count() > 0)
    {
        fprintf(stderr, "\t          : %d\n", m_galleryGroup->selectionModel()->selectedIndexes().at(0).row());
        return m_galleryGroup->selectionModel()->selectedIndexes().at(0).row();
    }
    else
        return /*-1*/0;
}

bool kpToolWidgetBase::hasPreviousOption (int *row, int *col) const
{
    int sel = this->selected();

    if (sel == -1 || sel == 0)
    {
        if (row)
            *row = -1;
        if (col)
            *col = -1;

        return false;
    }
    else
    {
        if (row)
            *row = sel - 1;
        if (col)
            *col = 0;
        return true;
    }
}

bool kpToolWidgetBase::hasNextOption (int *row, int *col) const
{
    int sel = selected();
    int count = m_galleryGroup->groupModel()->rowCount(QModelIndex());

    if (sel == -1 || sel == count - 1)
    {
        if (row)
            *row = -1;
        if (col)
            *col = -1;
        return false;
    }
    else
    {
        if (row)
            *row = sel + 1;
        if (col)
            *col = 0;

        return true;
    }
}

// public slot virtual
bool kpToolWidgetBase::setSelected (int row, int col, bool saveAsDefault)
{
    setSelected(row, col);

    if (saveAsDefault)
        saveSelectedAsDefault();

    return true;
}

// public slot
bool kpToolWidgetBase::setSelected (int row, int col)
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
bool kpToolWidgetBase::selectPreviousOption ()
{
    int newRow, newCol;
    if (!hasPreviousOption (&newRow, &newCol)) {
        return false;
    }

    return setSelected (newRow, newCol);
}

// public slot
bool kpToolWidgetBase::selectNextOption ()
{
    int newRow, newCol;
    if (!hasNextOption (&newRow, &newCol)) {
        return false;
    }

    return setSelected (newRow, newCol);
}

void kpToolWidgetBase::callOptionSelected()
{
    Q_EMIT optionSelected(selectedRow(), selectedCol());
}


