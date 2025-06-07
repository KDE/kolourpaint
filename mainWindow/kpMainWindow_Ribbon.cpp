#include "mainWindow/kpMainWindow.h"
#include "kpMainWindowPrivate.h"
#include "kpLogCategories.h"

#include <SARibbonBar/SARibbonCategory.h>
#include <SARibbonBar/SARibbonPannel.h>
#include <SARibbonBar/SARibbonMenu.h>
#include <SARibbonBar/SARibbonButtonGroupWidget.h>

#include <KStandardAction>
#include <KToggleAction>
#include <KFontSizeAction>
#include <KFontAction>

#include <QCheckBox>
#include <QString>
#include <QHBoxLayout>

static QAction* createAction(QMainWindow *win, const char *text, const char *iconurl)
{
    QAction* act = new QAction(win);
    act->setText(QLatin1String(text));
    act->setIcon(QIcon(QLatin1String(iconurl)));
    act->setObjectName(QLatin1String(text));
    win->connect(act, &QAction::triggered, win, [ win, act ]() {
        // InnerWidget* w = qobject_cast< InnerWidget* >(widget());
        // if (w) {
        //     w->appendText(QString("action(%1) triggered").arg(act->text()));
        // }
    });
    return act;
}

class MySARibbonPannel : public SARibbonPannel
{
public:
    void addAction(KToggleAction *action, SARibbonPannelItem::RowProportion rp);
};

void MySARibbonPannel::addAction(KToggleAction *kAction, SARibbonPannelItem::RowProportion rp)
{
    auto checkbox = new QCheckBox();

    checkbox->setText(kAction->text());
    checkbox->setToolTip(kAction->toolTip() + (kAction->shortcut().isEmpty() ? QLatin1String("") : QLatin1String(" (%1)").arg(kAction->shortcut().toString())));
    checkbox->setWhatsThis(kAction->whatsThis());
    // checkbox->addAction(kAction);

    QWidgetAction *wAction = (QWidgetAction *) this->addWidget(checkbox, rp);   // Check the code. It just gets casted down

    // Bind checked state       // TODO: QT6 use QBindable
    checkbox->setChecked(kAction->isChecked());
    QObject::connect(kAction, &QAction::toggled, [=]() {
        checkbox->setChecked(kAction->isChecked());
    });
    QObject::connect(checkbox, &QCheckBox::clicked, [kAction, checkbox]() {
        kAction->trigger();
    });

    // Bind sensitivity
    wAction->setEnabled(kAction->isEnabled());  // This doesnt work because the Panel wraps the widget in a QWidgetAction
    QObject::connect(kAction, &QAction::changed, checkbox, [wAction, kAction]() {
        wAction->setEnabled(kAction->isEnabled());
    });
}

void kpMainWindow::setupRibbon()
{
    d->ribbon->setTitleVisible(false);
    d->ribbon->setRibbonStyle(SARibbonBar::RibbonStyleCompactThreeRow);
    d->ribbon->setApplicationButton(nullptr);


    SARibbonCategory* page1 = new SARibbonCategory();
    page1->setCategoryName(QLatin1String("page1"));
    SARibbonPannel* pannel1 = new SARibbonPannel(QLatin1String("pannel1"), page1);
    page1->addPannel(pannel1);
    QAction* act = createAction(this, "  save  ", ":/icon/icon/save.svg");
    act->setIconText(QLatin1String("  save  "));
    pannel1->addLargeAction(act);
    pannel1->addLargeAction(createAction(this, "open", ":/icon/icon/folder-star.svg"));
    pannel1->addSmallAction(createAction(this, "action1", ":/icon/icon/action.svg"));
    pannel1->addSmallAction(createAction(this, "action2", ":/icon/icon/action2.svg"));
    SARibbonPannel* pannel2 = new SARibbonPannel(QLatin1String("pannel2"), page1);
    page1->addPannel(pannel2);
    pannel2->addLargeAction(createAction(this, "setting", ":/icon/icon/customize0.svg"));
    pannel2->addLargeAction(createAction(this, "windowsflag", ":/icon/icon/windowsflag-normal.svg"));
    d->ribbon->addCategoryPage(page1);

    SARibbonCategory* pgView = new SARibbonCategory();
    pgView->setCategoryName(QLatin1String("View"));
        SARibbonPannel* pnZoom = new SARibbonPannel(QLatin1String("Zoom"), pgView);
        pgView->addPannel(pnZoom);
        pnZoom->addLargeAction(d->actionZoomIn);
        pnZoom->addLargeAction(d->actionZoomOut);
        pnZoom->addSmallAction(d->actionActualSize);

        SARibbonMenu* fitMenu = new SARibbonMenu(this);
        fitMenu->addAction(d->actionFitToPage);
        fitMenu->addAction(d->actionFitToWidth);
        fitMenu->addAction(d->actionFitToHeight);
        auto act2 = createAction(this, "Fit to...", nullptr);
        act2->setIcon(d->actionFitToPage->icon());
        act2->setMenu(fitMenu);
        pnZoom->addSmallAction(act2, QToolButton::InstantPopup);

        MySARibbonPannel* pnShow = (MySARibbonPannel*) new SARibbonPannel(QLatin1String("Show or hide"), pgView);
        pgView->addPannel(pnShow);
        pnShow->addAction((d->actionShowGrid), SARibbonPannelItem::Small);
        pnShow->addAction((KStandardAction::showStatusbar(this, nullptr, this)), SARibbonPannelItem::Small);
        pnShow->addLargeAction(d->actionFullScreen);

        MySARibbonPannel* pnThumb = (MySARibbonPannel*) new SARibbonPannel(QLatin1String("Thumbnail"), pgView);
        pgView->addPannel(pnThumb);
        pnThumb->addLargeAction(d->actionShowThumbnail);
        pnThumb->addAction((d->actionZoomedThumbnail), SARibbonPannelItem::Small);
        pnThumb->addAction((d->actionShowThumbnailRectangle), SARibbonPannelItem::Small);
    d->ribbon->addCategoryPage(pgView);

    d->ribTextTools = d->ribbon->addContextCategory(QLatin1String("Text tools"), QColor(), 1);
    SARibbonCategory* pgText = d->ribTextTools->addCategoryPage(QLatin1String("Text"));
        SARibbonPannel* pnFont = new SARibbonPannel(QLatin1String("Font"), pgText);
        pgText->addPannel(pnFont);
        pnFont->addMediumAction(static_cast<QAction*>(d->actionTextFontFamily));
        auto hBox = new QWidget(this);
        auto hLayout = new QHBoxLayout(hBox);
            hLayout->addWidget(d->actionTextFontSize->requestWidget(hBox));
            auto fontSettingsGrp = new SARibbonButtonGroupWidget(pnFont);
                fontSettingsGrp->addAction(d->actionTextBold);
                fontSettingsGrp->addAction(d->actionTextItalic);
                fontSettingsGrp->addAction(d->actionTextUnderline);
                fontSettingsGrp->addAction(d->actionTextStrikeThru);
            hLayout->addWidget(fontSettingsGrp);
        pnFont->addMediumWidget(hBox);
    d->ribbon->hideContextCategory(d->ribTextTools);
    
}
