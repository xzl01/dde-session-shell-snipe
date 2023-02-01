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

#ifndef VIRTUAL_KEYBOARD_MODULE_H
#define VIRTUAL_KEYBOARD_MODULE_H

#include "tray_module_interface.h"

#include <QLabel>

class TipContentWidget;
class KeyboardIconWidget;

using namespace dss::module;
class VirtualKeyboardModule : public QObject, public TrayModuleInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.deepin.dde.shell.Modules.Tray" FILE "virtualkeyboard.json")
    Q_INTERFACES(dss::module::TrayModuleInterface)

public:
    explicit VirtualKeyboardModule(QObject *parent = nullptr);
    ~VirtualKeyboardModule();

    virtual bool isNeedInitPlugin() const;
    void init() override;

    inline QString key() const override { return objectName(); }
    QWidget *content() override;
    virtual QString icon() const override;

    virtual QWidget *itemWidget() const override;
    virtual QWidget *itemTipsWidget() const override;
    virtual const QString itemContextMenu() const override;
    virtual void invokedMenuItem(const QString &menuId, const bool checked) const override;

private:
    TipContentWidget *m_tipContentWidget;
    KeyboardIconWidget *m_keyboardIconWidget;
};

#endif // VIRTUAL_KEYBOARD_MODULE_H
