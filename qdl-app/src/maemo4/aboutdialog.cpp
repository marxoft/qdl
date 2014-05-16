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

#include "aboutdialog.h"
#include "../shared/definitions.h"
#include <QDesktopServices>
#include <QUrl>
#include <QLabel>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QGridLayout>

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent)
{
    this->setWindowTitle(tr("About QDL"));
    this->setAttribute(Qt::WA_DeleteOnClose, true);

    QLabel *icon = new QLabel(this);
    icon->setPixmap(QPixmap(ICON_PATH + "qdl.png"));
    QLabel *title = new QLabel(QString("<b><font size='40'>QDL %1</font></b>").arg(VERSION_NUMBER), this);
    QLabel *description = new QLabel("A user-friendly download manager.<br><br>&copy; Stuart Howarth 2012-2013", this);
    description->setWordWrap(true);
    QDialogButtonBox *buttonBox = new QDialogButtonBox(Qt::Horizontal, this);
    QPushButton *donateButton = new QPushButton(tr("Donate"), this);
    QPushButton *bugButton = new QPushButton(tr("Report bug"), this);
    QPushButton *closeButton = new QPushButton(tr("Close"), this);
    buttonBox->addButton(donateButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(bugButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(closeButton, QDialogButtonBox::AcceptRole);

    QGridLayout *grid = new QGridLayout(this);
    grid->addWidget(icon, 0, 0);
    grid->addWidget(title, 0, 1);
    grid->addWidget(description, 1, 0, 1, 2);
    grid->addWidget(buttonBox, 2, 0, 1, 2);

    this->connect(donateButton, SIGNAL(clicked()), this, SLOT(donate()));
    this->connect(bugButton, SIGNAL(clicked()), this, SLOT(reportBug()));
    this->connect(closeButton, SIGNAL(clicked()), this, SLOT(accept()));
}

AboutDialog::~AboutDialog() {}

void AboutDialog::donate() {
    QDesktopServices::openUrl(QUrl("https://www.paypal.com/cgi-bin/webscr?cmd=_donations&business=showarth@marxoft.co.uk&lc=GB&item_name=QDL&currency_code=USD&bn=PP%2dDonationsBF%3abtn_donate_SM%2egif%3aNonHosted"));
    this->accept();
}

void AboutDialog::reportBug() {
    QDesktopServices::openUrl(QUrl(QString("mailto:showarth@marxoft.co.uk?subject=QDL %1 for Maemo4").arg(VERSION_NUMBER)));
    this->accept();
}
