#include "mainWindow/kpMainWindow.h"
#include "kpMainWindowPrivate.h"
#include "tools/kpTool.h"
#include "tools/selection/text/kpToolText.h"
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
#include <QButtonGroup>
#include <QString>
#include <QHBoxLayout>

    // m_toolWidgets.append (m_toolWidgetBrush =
    //     new kpToolWidgetBrush (m_baseWidget, QStringLiteral("Tool Widget Brush")));
    // m_toolWidgets.append (m_toolWidgetEraserSize =
    //     new kpToolWidgetEraserSize (m_baseWidget, QStringLiteral("Tool Widget Eraser Size")));
    // m_toolWidgets.append (m_toolWidgetFillStyle =
    //     new kpToolWidgetFillStyle (m_baseWidget, QStringLiteral("Tool Widget Fill Style")));
    // m_toolWidgets.append (m_toolWidgetLineWidth =
    //     new kpToolWidgetLineWidth (m_baseWidget, QStringLiteral("Tool Widget Line Width")));
    // m_toolWidgets.append (m_toolWidgetOpaqueOrTransparent =
    //     new kpToolWidgetOpaqueOrTransparent (m_baseWidget, QStringLiteral("Tool Widget Opaque/Transparent")));
    // m_toolWidgets.append (m_toolWidgetSpraycanSize =
    //     new kpToolWidgetSpraycanSize (m_baseWidget, QStringLiteral("Tool Widget Spraycan Size")));
    // for (auto *w : m_toolWidgets)
    // {
    //   connect (w, &kpToolWidgetBase::optionSelected,
    //            this, &kpToolToolBar::toolWidgetOptionSelected);
    // }

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


    SARibbonCategory* pgHome = new SARibbonCategory();
    pgHome->setCategoryName(QLatin1String("Home"));
        SARibbonPannel* pnClp = new SARibbonPannel(QLatin1String("Clipboard"), pgHome);
        pgHome->addPannel(pnClp);
        pnClp->addLargeAction(d->actionPaste, QToolButton::MenuButtonPopup);
            SARibbonMenu* pasteMenu = new SARibbonMenu(pnClp);
            pasteMenu->addAction(d->actionPasteInNewWindow);
            pasteMenu->addAction(d->actionPasteFromFile);
            d->actionPaste->setMenu(pasteMenu);
        pnClp->addSmallAction(d->actionCut);
        SARibbonMenu* copyMenu = new SARibbonMenu(pnClp);
        copyMenu->addAction(d->actionCopyToFile);
        d->actionCopy->setMenu(copyMenu);
        pnClp->addSmallAction(d->actionCopy, QToolButton::MenuButtonPopup);

        SARibbonPannel* pnSel = new SARibbonPannel(QLatin1String("Selection"), pgHome);
        pgHome->addPannel(pnSel);
        auto actSelectMenu = createAction(this, "Select", nullptr);
            SARibbonMenu* selectMenu = new SARibbonMenu(pnSel);
                selectMenu->addAction(d->toolRectSelection->action());
                selectMenu->addAction(d->toolEllipticalSelection->action());
                selectMenu->addAction(d->toolFreeFormSelection->action());
            actSelectMenu->setIcon(d->toolRectSelection->action()->icon());
            actSelectMenu->setMenu(selectMenu);
        pnSel->addLargeAction(actSelectMenu, QToolButton::InstantPopup);
        pnSel->addSmallAction(d->actionSelectAll);
        pnSel->addSmallAction(d->actionDeselect);
        pnSel->addSmallAction(d->actionDelete);

        SARibbonPannel* pnTools = new SARibbonPannel(QLatin1String("Tools"), pgHome);
        pgHome->addPannel(pnTools);
        auto toolsRow1 = new SARibbonButtonGroupWidget(pnTools);
            toolsRow1->addAction(d->toolPen->action());
            toolsRow1->addAction(d->toolLine->action());
            toolsRow1->addAction(d->toolPolygon->action());
            toolsRow1->addAction(d->toolText->action());
        auto toolsRow2 = new SARibbonButtonGroupWidget(pnTools);
            toolsRow2->addAction(d->toolFloodFill->action());
            toolsRow2->addAction(d->toolEraser->action());
            toolsRow2->addAction(d->toolColorPicker->action());
            toolsRow2->addAction(d->toolZoom->action());
        pnTools->addMediumWidget(toolsRow1);
        pnTools->addMediumWidget(toolsRow2);

    d->ribbon->addCategoryPage(pgHome);

    SARibbonCategory* pgColors = new SARibbonCategory();
    pgColors->setCategoryName(QLatin1String("Colors"));
    d->ribbon->addCategoryPage(pgColors);

    SARibbonCategory* pgImage = new SARibbonCategory();
    pgImage->setCategoryName(QLatin1String("Image"));
    d->ribbon->addCategoryPage(pgImage);

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
        // SARibbonPannel* pnClp2 = new SARibbonPannel(QLatin1String("Clipboard"), pgText);
        // pgText->addPannel(pnClp2);
        // pnClp2->addLargeAction(KStandardAction::create(KStandardAction::Paste, this, nullptr, this));
        // pnClp2->addMediumAction(KStandardAction::create(KStandardAction::Cut, this, nullptr, this));
        // pnClp2->addMediumAction(KStandardAction::create(KStandardAction::Copy, this, nullptr, this));

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

        SARibbonPannel* pnBg = new SARibbonPannel(QLatin1String("Background"), pgText);
        pgText->addPannel(pnBg);
            d->bgroupOpaqueOrTransparent = new QButtonGroup(pnBg);
            d->bgroupOpaqueOrTransparent->setExclusive(true);
            connect(d->bgroupOpaqueOrTransparent, &QButtonGroup::idToggled, this, &kpMainWindow::updateActionDrawOpaqueChecked);

            auto tparencyOpaque = new QPushButton(QPixmap(QStringLiteral(":/icons/option_opaque")), QLatin1String("Opaque"), pnBg);
            tparencyOpaque->setCheckable(true);
            tparencyOpaque->setChecked(true);
            d->bgroupOpaqueOrTransparent->addButton(tparencyOpaque, 0);
            pnBg->addSmallWidget(tparencyOpaque);

            auto tparencyTparent = new QPushButton(QPixmap(QStringLiteral(":/icons/option_transparent")), QLatin1String("Transparent"), pnBg);
            tparencyTparent->setCheckable(true);
            tparencyTparent->setChecked(true);
            d->bgroupOpaqueOrTransparent->addButton(tparencyTparent, 1);
            pnBg->addSmallWidget(tparencyTparent);
        // Transparent / Opaque
        // pnClp2->addSmallAction(KStandardAction::create(KStandardAction::Paste, this, nullptr, this));
    d->ribbon->hideContextCategory(d->ribTextTools);
    
}
