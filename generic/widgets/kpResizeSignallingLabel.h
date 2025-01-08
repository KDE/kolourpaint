
/*
   SPDX-FileCopyrightText: 2003-2007 Clarence Dang <dang@kde.org>

   SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef KP_RESIZE_SIGNALLING_LABEL_H
#define KP_RESIZE_SIGNALLING_LABEL_H

#include <QLabel>

class QResizeEvent;

class kpResizeSignallingLabel : public QLabel
{
    Q_OBJECT

public:
    kpResizeSignallingLabel(const QString &string, QWidget *parent);
    kpResizeSignallingLabel(QWidget *parent);
    ~kpResizeSignallingLabel() override;

Q_SIGNALS:
    void resized();

protected:
    void resizeEvent(QResizeEvent *e) override;
};

#endif // KP_RESIZE_SIGNALLING_LABEL_H
