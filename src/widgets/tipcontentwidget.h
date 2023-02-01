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

#ifndef TIPCONTENTWIDGET_H
#define TIPCONTENTWIDGET_H

#include <QFrame>

class TipContentWidget : public QFrame
{
    Q_OBJECT

    enum ShowType {
        SingleLine,
        MultiLine
    };

public:
    explicit TipContentWidget(QWidget *parent = nullptr);

    const QString& text(){ return m_text; }
    const QStringList &textList() { return  m_textList; }

    void setText(const QString &text, bool multiLine = true);
    void setTextList(const QStringList &textList);

protected:
    void paintEvent(QPaintEvent *event) override;
    bool event(QEvent *event) override;

private:
    void showSingleLineText(const QString &text);

private:
    QString m_text;
    QStringList m_textList;
    ShowType m_type;
};

#endif // TIPCONTENTWIDGET_H
