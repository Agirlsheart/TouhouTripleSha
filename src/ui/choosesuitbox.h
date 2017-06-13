/********************************************************************
    Copyright (c) 2013-2015 - Mogara

    This file is part of QSanguosha-Hegemony.

    This game is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 3.0
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    See the LICENSE file for more details.

    Mogara
    *********************************************************************/

#ifndef CHOOSESUITBOX_H
#define CHOOSESUITBOX_H

#include "graphicsbox.h"

class Button;
class QGraphicsProxyWidget;
class QSanCommandProgressBar;

class SuitOptionButton : public QGraphicsObject
{
    Q_OBJECT

    friend class ChooseSuitBox;

public:
    static QFont defaultFont();

signals:
    void clicked();
    void hovered(bool entering);

protected:
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *);
    virtual QRectF boundingRect() const;
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);

private:
    SuitOptionButton(QGraphicsObject *parent, const QString &suit, const int width);

    QString m_suit;
    int width;
};

class ChooseSuitBox : public GraphicsBox
{
    Q_OBJECT

public:
    ChooseSuitBox();

    QRectF boundingRect() const;

    void chooseSuit(const QStringList &suits);
    void clear();

private:
    int suitNumber;
    QStringList m_suits;
    QList<SuitOptionButton *> buttons;

    static const int outerBlankWidth;
    static const int buttonWidth;
    static const int buttonHeight;
    static const int interval;
    static const int topBlankWidth;
    static const int bottomBlankWidth;

    QGraphicsProxyWidget *progressBarItem;
    QSanCommandProgressBar *progressBar;

    void reply();
};

#endif // CHOOSESUITBOX_H
