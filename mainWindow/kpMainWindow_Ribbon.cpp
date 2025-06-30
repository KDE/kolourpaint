#include "mainWindow/kpMainWindow.h"
#include "kpMainWindowPrivate.h"
#include "tools/kpTool.h"
#include "tools/selection/text/kpToolText.h"
#include "widgets/kpColorCells.h"
#include "widgets/kpColorPalette.h"
#include "widgets/toolbars/kpColorToolBar.h"
#include "widgets/colorSimilarity/kpColorSimilarityToolBarItem.h"
#include "lgpl/generic/kpUrlFormatter.h"
#include "kpLogCategories.h"

#include <SARibbonBar/SARibbonCategory.h>
#include <SARibbonBar/SARibbonPannel.h>
#include <SARibbonBar/SARibbonMenu.h>
#include <SARibbonBar/SARibbonQuickAccessBar.h>
#include <SARibbonBar/SARibbonButtonGroupWidget.h>
#include <SARibbonBar/SARibbonColorToolButton.h>

#include <KStandardAction>
#include <KToggleAction>
#include <KFontSizeAction>
#include <KFontAction>

#include <QCheckBox>
#include <QButtonGroup>
#include <QString>
#include <QHBoxLayout>
#include <QWidgetAction>

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
    void addAction(KToggleAction *action, SARibbonPannelItem::RowProportion rp);    // This is an implementation specific to `KToggleAction` that creates a checkbox instead of a button
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

    d->ribbon->quickAccessBar()->addAction(d->actionSave);
    d->ribbon->quickAccessBar()->addSeparator();
    d->ribbon->quickAccessBar()->addAction(d->actionUndo);
    d->ribbon->quickAccessBar()->addAction(d->actionRedo);

    /* Pages */

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

    {
        SARibbonCategory* pg = new SARibbonCategory();
        pg->setCategoryName(QLatin1String("Colours"));
        d->ribbon->addCategoryPage(pg);

        {
            SARibbonPannel* pn = new SARibbonPannel(QLatin1String("Colours"), pg);
            pg->addPannel(pn);

            //
            
            {
                auto fgButton = new SARibbonColorToolButton(pn);
                auto bgButton = new SARibbonColorToolButton(pn);
                SAColorMenu *fgMenu = fgButton->setupStandardColorMenu();
                SAColorMenu *bgMenu = bgButton->setupStandardColorMenu();
                fgButton->setButtonType(SARibbonToolButton::LargeButton);
                bgButton->setButtonType(SARibbonToolButton::LargeButton);
                fgButton->setColorStyle(SARibbonColorToolButton::ColorFillToIcon);
                bgButton->setColorStyle(SARibbonColorToolButton::ColorFillToIcon);
                fgButton->setText(QLatin1String("Foreground"));
                bgButton->setText(QLatin1String("Background"));
                pn->addLargeWidget(fgButton);
                pn->addLargeWidget(bgButton);

                connect(fgButton, &SARibbonColorToolButton::colorChanged, [&](const QColor& color) {
                    d->colorToolBar->setForegroundColor(kpColor(color.rgba()));
                });
                connect(bgButton, &SARibbonColorToolButton::colorChanged, [&](const QColor& color) {
                    d->colorToolBar->setBackgroundColor(kpColor(color.rgba()));
                });
                connect(d->colorToolBar, &kpColorToolBar::foregroundColorChanged, [&, fgButton](const kpColor& color) {
                    fgButton->setColor(color.isTransparent() ? QColor(/*invalid*/) : color.toQColor());
                });
                connect(d->colorToolBar, &kpColorToolBar::backgroundColorChanged, [&, bgButton](const kpColor& color) {
                    bgButton->setColor(color.isTransparent() ? QColor(/*invalid*/) : color.toQColor());
                });
            }

            pn->addSmallAction(d->actionColorsAppendRow);
            pn->addSmallAction(d->actionColorsDeleteRow);
            pn->addSmallAction(d->actionColorsSwap);
        }
        {
            SARibbonPannel* pn = d->pnPalette = new SARibbonPannel(QLatin1String(""), pg);
            pg->addPannel(pn);

            connect (this, &kpMainWindow::newPaletteTitle, [&](QString title) {
                d->pnPalette->setPannelName(title);
            });

            QAction* actionFloatPalette = new QAction(this);
            pn->setOptionAction(actionFloatPalette);
            connect (actionFloatPalette, &QAction::triggered, this, &kpMainWindow::slotFloatPalette);

            {
                auto w = new QWidget(pn);   // We have it inside a placeholder widget so that we have something to put it inside when we transplant it back from the floating window.
                d->colorPalette = new kpColorPalette(w);

                d->colorPaletteContainer = new QVBoxLayout(w);
                w->setLayout(d->colorPaletteContainer);
                d->colorPaletteContainer->addWidget(d->colorPalette);
                w->adjustSize();

                connect (d->colorPalette, &kpColorPalette::foregroundColorChanged, [&](const kpColor& color) {
                    d->colorToolBar->setForegroundColor(color);
                });

                connect (d->colorPalette, &kpColorPalette::backgroundColorChanged, [&](const kpColor& color) {
                    d->colorToolBar->setBackgroundColor(color);
                });

                connect (colorCells (), &kpColorCells::rowCountChanged,
                    this, &kpMainWindow::slotUpdateColorsDeleteRowActionEnabled);

                connect (colorCells (), &kpColorCells::isModifiedChanged,
                    this, &kpMainWindow::updatePaletteNameOrUrlLabel);

                connect (colorCells (), &kpColorCells::urlChanged,
                    this, &kpMainWindow::updatePaletteNameOrUrlLabel);

                connect (colorCells (), &kpColorCells::nameChanged,
                    this, &kpMainWindow::updatePaletteNameOrUrlLabel);

                updatePaletteNameOrUrlLabel ();

                pn->addLargeWidget(w);
            }
        }
        {
            SARibbonPannel* pn = new SARibbonPannel(QLatin1String("Palette"), pg);
            pg->addPannel(pn);

            {
                pn->addSmallAction(d->actionColorsOpen, QToolButton::MenuButtonPopup);

                auto menu = new SARibbonMenu(pn);
                menu->addAction(d->actionColorsDefault);
                menu->addAction(d->actionColorsKDE);
                d->actionColorsOpen->setMenu(menu);
            }
            {
                pn->addSmallAction(d->actionColorsSave, QToolButton::MenuButtonPopup);

                auto menu = new SARibbonMenu(pn);
                menu->addAction(d->actionColorsSaveAs);
                d->actionColorsSave->setMenu(menu);
            }
            pn->addSmallAction(d->actionColorsReload);
        }
        {
            SARibbonPannel* pn = new SARibbonPannel(QLatin1String("Colour Similarity"), pg);
            pg->addPannel(pn);

            auto colorSimWidg = new kpColorSimilarityToolBarItem (pn);
            connect (
                colorSimWidg,
                &kpColorSimilarityToolBarItem::colorSimilarityChanged,
                [&, colorSimWidg](double similarity, int processedSimilarity) {
                    colorSimWidg->blockSignals(true);
                    d->colorToolBar->setColorSimilarity(similarity);
                    colorSimWidg->blockSignals(false);
                }
            );
            connect(
                d->colorToolBar,
                &kpColorToolBar::colorSimilarityChanged,
                [&, colorSimWidg](double similarity, int processedSimilarity) {
                    colorSimWidg->setColorSimilarity(similarity);
                }
            );

            pn->addLargeWidget(colorSimWidg);
        }
    }

    {
        SARibbonCategory* pg = new SARibbonCategory();
        pg->setCategoryName(QLatin1String("Image"));
        d->ribbon->addCategoryPage(pg);

        {
            SARibbonPannel* pn = new SARibbonPannel(QLatin1String("Size"), pg);
            pg->addPannel(pn);
            pn->addLargeAction(d->actionResizeScale);
            pn->addSmallAction(d->actionAutoCrop);
        }
        {
            SARibbonPannel* pn = new SARibbonPannel(QLatin1String("Manipulate"), pg);
            pg->addPannel(pn);

            {
                auto group = new SARibbonButtonGroupWidget(pnTools);
                pn->addMediumWidget(group);

                group->addAction(d->actionRotateLeft);
                group->addAction(d->actionRotateRight);
            }
            {
                auto group = new SARibbonButtonGroupWidget(pnTools);
                pn->addMediumWidget(group);

                group->addAction(d->actionMirror);
                group->addAction(d->actionFlip);
            }

            pn->addLargeAction(d->actionRotate);
            pn->addLargeAction(d->actionSkew);
        }
        {
            SARibbonPannel* pn = new SARibbonPannel(QLatin1String("Image Effects"), pg);
            pg->addPannel(pn);

            pn->addSmallAction(d->actionInvertColors);
            {
                SARibbonMenu* menu = new SARibbonMenu(pn);
                menu->addAction(d->actionConvertToGrayscale);
                menu->addAction(d->actionConvertToBlackAndWhite);

                auto act1 = createAction(this, "Reduce to", nullptr);
                // act1->setIcon();
                act1->setMenu(menu);

                pn->addSmallAction(act1, QToolButton::InstantPopup);
            }
            pn->addSmallAction(d->actionBlur);
            pn->addLargeAction(d->actionMoreEffects);
        }
    }

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

PaletteToolWindow::PaletteToolWindow(QWidget *parent)
    : QMainWindow(parent, Qt::Tool)
{
}

PaletteToolWindow::~PaletteToolWindow() = default;

void PaletteToolWindow::closeEvent(QCloseEvent *event)
{
    Q_EMIT aboutToClose();
    this->QMainWindow::closeEvent(event);
}

void kpMainWindow::slotFloatPalette()
{
    auto win = new PaletteToolWindow(this);
    win->setCentralWidget(d->colorPalette);
    win->adjustSize();
    win->setWindowTitle(d->pnPalette->pannelName());
    win->show();

    connect(win, &PaletteToolWindow::aboutToClose, this, &kpMainWindow::slotUnfloatPalette);

    connect(this, &kpMainWindow::newPaletteTitle, [&, win](QString title) {
        win->setWindowTitle(title);
    });

    d->pnPalette->setVisible(false);    // Hide the ribbon pannel that it came from
}

void kpMainWindow::slotUnfloatPalette()
{
    d->colorPaletteContainer->addWidget(d->colorPalette);   // Transplant it back

    d->pnPalette->setVisible(true);
}

void kpMainWindow::updatePaletteNameOrUrlLabel()
{
    QString name;

    if (!colorCells()->url ().isEmpty ()) {
        name = kpUrlFormatter::PrettyFilename (colorCells()->url ());
    }
    else
    {
        if (!colorCells()->name ().isEmpty ()) {
            name = colorCells()->name ();
        }
        else {
            name = i18n ("KolourPaint Defaults");
        }
    }

    if (name.isEmpty ()) {
        name = i18n ("Untitled");
    }


    KLocalizedString labelStr;

    if (!colorCells()->isModified ())
    {
        labelStr =
            ki18nc ("Colors: name_or_url_of_color_palette",
                    "Colors: %1")
                .subs (name);
    }
    else
    {
        labelStr =
            ki18nc ("Colors: name_or_url_of_color_palette [modified]",
                    "Colors: %1 [modified]")
                .subs (name);
    }

    Q_EMIT newPaletteTitle(name);
}
