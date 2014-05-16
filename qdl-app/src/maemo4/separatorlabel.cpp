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

#include "separatorlabel.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QApplication>

SeparatorLabel::SeparatorLabel(const QString &text, QWidget *parent) :
    QWidget(parent),
    m_label(new QLabel(text, this))
{
    this->setStyleSheet(QString("color: %1").arg(QApplication::palette().color(QPalette::Mid).name()));

    QHBoxLayout *hbox = new QHBoxLayout(this);
    QFrame *line = new QFrame(this);

    line->setFrameShape(QFrame::HLine);
    line->setLineWidth(1);

    hbox->addWidget(line, 1);
    hbox->addWidget(m_label, 0, Qt::AlignVCenter);
}

SeparatorLabel::~SeparatorLabel() {}
