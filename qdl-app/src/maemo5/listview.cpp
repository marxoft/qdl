/*
 * Copyright (C) 2014 Stuart Howarth <showarth@marxoft.co.uk>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU Lesser General Public License,
 * version 3, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "listview.h"
#include <QScrollBar>
#include <QAbstractKineticScroller>

ListView::ListView(QWidget *parent) :
    QListView(parent),
    m_kineticScroller(property("kineticScroller").value<QAbstractKineticScroller *>()),
    m_minimum(0),
    m_maximum(1000000)
{
    this->setUniformItemSizes(true);
    this->setAutoScroll(false);
    this->setEditTriggers(QListView::NoEditTriggers);
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setContextMenuPolicy(Qt::CustomContextMenu);

    this->connect(verticalScrollBar(), SIGNAL(rangeChanged(int,int)), this, SLOT(setScrollRange(int,int)));
    this->connect(verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(onScrollPositionChanged(int)));
}

void ListView::setScrollRange(int minimum, int maximum) {
    m_minimum = minimum;
    m_maximum = maximum;
}

void ListView::onScrollPositionChanged(int position) {
    if (position == m_maximum) {
        emit atEnd();
    }
}

void ListView::positionAtBeginning() {
    this->scrollTo(model()->index(0, 0), QListView::PositionAtTop);
}

void ListView::positionAtEnd() {
    this->scrollTo(model()->index(model()->rowCount() - 1, 0), QListView::PositionAtBottom);
}

void ListView::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Down) {
        if (event->modifiers() == Qt::ShiftModifier) {
            this->positionAtEnd();
        }
        else {
            m_kineticScroller->scrollTo(QPoint(0, this->rectForIndex(indexAt(QPoint(0, height()))).y()));
        }

        event->accept();
    }
    else if (event->key() == Qt::Key_Up) {
        if (event->modifiers() == Qt::ShiftModifier) {
            this->positionAtBeginning();
        }
        else {
            m_kineticScroller->scrollTo(QPoint(0, this->rectForIndex(indexAt(QPoint(0, -height()))).y()));
        }

        event->accept();
    }
    else {
        event->ignore();
    }
}

void ListView::mousePressEvent(QMouseEvent *event) {
    this->setCurrentIndex(indexAt(event->pos()));
    QListView::mousePressEvent(event);
}

void ListView::mouseReleaseEvent(QMouseEvent *event) {
    QListView::mouseReleaseEvent(event);
    this->setCurrentIndex(QModelIndex());
}
