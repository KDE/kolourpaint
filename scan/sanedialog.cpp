/* ============================================================
 *
 * Date        : 2008-04-17
 * Description : Sane plugin interface for KDE
 *
 * SPDX-FileCopyrightText: 2008 Kare Sars <kare dot sars at iki dot fi>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 * ============================================================ */

#include "sanedialog.h"

#include "kpLogCategories.h"
#include <KConfigGroup>
#include <KLocalizedString>
#include <KMessageBox>
#include <KSharedConfig>
#include <KWindowConfig>

#include <QPushButton>

SaneDialog::SaneDialog(QWidget *parent)
    : KPageDialog(parent)
{
    setFaceType(static_cast<KPageDialog::FaceType>(Plain));
    setWindowTitle(i18nc("@title:window", "Acquire Image"));

    buttonBox()->setStandardButtons(QDialogButtonBox::Close);
    buttonBox()->button(QDialogButtonBox::Close)->setDefault(true);

    m_ksanew = new KSaneIface::KSaneWidget(this);
    addPage(m_ksanew, QString());

    connect(m_ksanew, &KSaneIface::KSaneWidget::scannedImageReady, this, &SaneDialog::imageReady);

    m_openDev = QString();
}

bool SaneDialog::setup()
{
    if (!m_ksanew) {
        // new failed
        return false;
    }
    if (!m_openDev.isEmpty()) {
        return true;
    }
    // need to select a scanner
    m_openDev = m_ksanew->selectDevice(parentWidget());
    if (m_openDev.isEmpty()) {
        // either no scanner was found or then cancel was pressed.
        return false;
    }
    if (!m_ksanew->openDevice(m_openDev)) {
        // could not open the scanner
        KMessageBox::error(parentWidget(), i18n("Opening the selected scanner failed."));
        m_openDev = QString();
        return false;
    }

    // restore scan dialog size and all options for the selected device if available
    KSharedConfigPtr configPtr = KSharedConfig::openConfig(QStringLiteral("scannersettings"));
    KWindowConfig::restoreWindowSize(windowHandle(), KConfigGroup(configPtr, QStringLiteral("ScanDialog")));
    QString groupName = m_openDev;
    if (configPtr->hasGroup(groupName)) {
        KConfigGroup group(configPtr, groupName);
        QStringList keys = group.keyList();
        for (int i = 0; i < keys.count(); i++) {
            m_ksanew->setOptionValue(keys[i], group.readEntry(keys[i]));
        }
    }

    return true;
}

SaneDialog::~SaneDialog()
{
    if (m_ksanew && !m_openDev.isEmpty()) {
        // save scan dialog size and all options for the selected device if available
        KSharedConfigPtr configPtr = KSharedConfig::openConfig(QStringLiteral("scannersettings"));
        KConfigGroup group(configPtr, QStringLiteral("ScanDialog"));
        KWindowConfig::saveWindowSize(windowHandle(), group, KConfigGroup::Persistent);
        group = configPtr->group(m_openDev);
        QMap<QString, QString> opts;
        m_ksanew->getOptionValues(opts);
        QMap<QString, QString>::const_iterator i = opts.constBegin();
        for (; i != opts.constEnd(); ++i) {
            group.writeEntry(i.key(), i.value(), KConfigGroup::Persistent);
        }
    }
}

void SaneDialog::imageReady(const QImage &img)
{
    Q_EMIT finalImage(img, nextId());
}

int SaneDialog::nextId()
{
    return ++m_currentId;
}

#include "moc_sanedialog.cpp"
