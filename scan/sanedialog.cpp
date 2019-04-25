/* ============================================================
 *
 * Date        : 2008-04-17
 * Description : Sane plugin interface for KDE
 *
 * Copyright (C) 2008 by Kare Sars <kare dot sars at iki dot fi>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) version 3, or any
 * later version accepted by the membership of KDE e.V. (or its
 * successor approved by the membership of KDE e.V.), which shall
 * act as a proxy defined in Section 6 of version 3 of the license.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * ============================================================ */

#include "sanedialog.h"

#include <KLocalizedString>
#include "kpLogCategories.h"
#include <KMessageBox>
#include <KSharedConfig>
#include <KWindowConfig>
#include <KConfigGroup>

#include <QPushButton>

SaneDialog::SaneDialog(QWidget *parent)
    : KPageDialog(parent)
{
    setFaceType(static_cast<KPageDialog::FaceType> (Plain));
    setWindowTitle(i18nc("@title:window", "Acquire Image"));

    buttonBox()->setStandardButtons(QDialogButtonBox::Close);
    buttonBox()->button(QDialogButtonBox::Close)->setDefault(true);


    m_ksanew = new KSaneIface::KSaneWidget(this);
    addPage(m_ksanew, QString());

    connect (m_ksanew, &KSaneIface::KSaneWidget::imageReady,
             this, &SaneDialog::imageReady);

    m_openDev = QString();
}

bool SaneDialog::setup()
{
    if(!m_ksanew) {
        // new failed
        return false;
    }
    if (!m_openDev.isEmpty()) {
        return true;
    }
    // need to select a scanner
    m_openDev = m_ksanew->selectDevice(nullptr);
    if (m_openDev.isEmpty()) {
       // either no scanner was found or then cancel was pressed.
        return false;
    }
    if (!m_ksanew->openDevice(m_openDev)) {
        // could not open the scanner
        KMessageBox::sorry(nullptr, i18n("Opening the selected scanner failed."));
        m_openDev = QString();
        return false;
    }

    // restore scan dialog size and all options for the selected device if available
    KSharedConfigPtr configPtr = KSharedConfig::openConfig(QStringLiteral("scannersettings"));
    KWindowConfig::restoreWindowSize(windowHandle(), KConfigGroup(configPtr, "ScanDialog"));
    QString groupName = m_openDev;
    if (configPtr->hasGroup(groupName)) {
        KConfigGroup group(configPtr, groupName);
        QStringList keys = group.keyList();
        for (int i = 0; i < keys.count(); i++) {
            m_ksanew->setOptVal(keys[i], group.readEntry(keys[i]));
        }
    }

   return true;
}

SaneDialog::~SaneDialog()
{
    if (m_ksanew && !m_openDev.isEmpty()) {
        // save scan dialog size and all options for the selected device if available
        KSharedConfigPtr configPtr = KSharedConfig::openConfig(QStringLiteral("scannersettings"));
        KConfigGroup group(configPtr, "ScanDialog");
        KWindowConfig::saveWindowSize(windowHandle(), group, KConfigGroup::Persistent);
        group = configPtr->group(m_openDev);
        QMap<QString, QString> opts;
        m_ksanew->getOptVals(opts);
        QMap<QString, QString>::const_iterator i = opts.constBegin();
        for (; i != opts.constEnd(); ++i) {
            group.writeEntry(i.key(), i.value(), KConfigGroup::Persistent);
        }
    }
}

void SaneDialog::imageReady(QByteArray &data, int w, int h, int bpl, int f)
{
    /* copy the image data into img */
    QImage img = m_ksanew->toQImage(data, w, h, bpl, static_cast<KSaneIface::KSaneWidget::ImageFormat> (f));
    emit finalImage(img, nextId());
}

int SaneDialog::nextId()
{
    return ++m_currentId;
}
