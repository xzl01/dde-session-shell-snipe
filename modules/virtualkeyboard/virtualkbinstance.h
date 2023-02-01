/*
 * Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co.,Ltd.
 *
 * Author:     zhaoyingzhen <zhaoyingzhen@uniontech.com>
 *
 * Maintainer: zhaoyingzhen <zhaoyingzhen@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef VIRTUALKBINSTANCE_H
#define VIRTUALKBINSTANCE_H

#include <QObject>

class QProcess;
class VirtualKBInstance : public QObject
{
    Q_OBJECT
public:
    static VirtualKBInstance &Instance();
    ~VirtualKBInstance() override;

    void init();
    void stopVirtualKBProcess();

public slots:
    void showKeyboardWidget(QWidget *parent);
    void hideKeyboardWidget();
    void onVirtualKBProcessFinished(int exitCode);

private:
    explicit VirtualKBInstance(QObject *parent = nullptr);
    VirtualKBInstance(const VirtualKBInstance &) = delete;

private:
    QWidget *m_parentWidget;
    QWidget *m_virtualKBWidget;
    QProcess *m_virtualKBProcess;
};

#endif // VIRTUALKBINSTANCE_H
