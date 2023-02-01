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
#ifndef USER_ITEM_DELEGATE_H
#define USER_ITEM_DELEGATE_H

#include <QStyledItemDelegate>

/*!
 * \brief The UserItemDelegate class
 */
class UserItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    enum UserItemDataRole {
        StaticUserDataRole      = Qt::UserRole + 1, // 用户静态数据
        IsCurrentUserDataRole   = Qt::UserRole + 2, // 是否是当前用户
    };

    struct UserItemData {
        QString imagePath;
        QString displayName;
        QString name;
        QString userType;
        uid_t userId;
        bool isLogined = false;
    };

public:
    explicit UserItemDelegate(QObject *parent = nullptr);

protected:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const Q_DECL_OVERRIDE;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const Q_DECL_OVERRIDE;

private:
    void drawRoundImage(QPainter *painter, const QRect &rect, const QString &path) const;
    void drawLoginedState(QPainter *painter, const QRect &rect) const;
    void drawNameAndType(QPainter *painter, const UserItemData &userData, const QRect &rect) const;
    void drawCheckedState(QPainter *painter, const QRect &rect) const;

    int stringWidth(const QString &str, int fontSize, bool isBold = false) const;
    QString elidedText(const QString &originString, int width, int fontSize, bool isBold = false) const;
};

Q_DECLARE_METATYPE(UserItemDelegate::UserItemData)

#endif // USER_ITEM_DELEGATE_H
