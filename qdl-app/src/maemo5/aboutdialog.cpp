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
    this->setWindowTitle(tr("About"));
    this->setAttribute(Qt::WA_DeleteOnClose, true);

    QLabel *imageLabel = new QLabel(this);
    imageLabel->setPixmap(QPixmap(ICON_PATH + "qdl.png"));
    imageLabel->setFixedWidth(64);
    QLabel *titleLabel = new QLabel(QString("<b><font size='20'>QDL %1</font></b>").arg(VERSION_NUMBER), this);
    titleLabel->setAlignment(Qt::AlignVCenter);
    QLabel *descriptionLabel = new QLabel("A user-friendly download manager.<br><br>&copy; Stuart Howarth 2012-2013", this);
    descriptionLabel->setWordWrap(true);
    QDialogButtonBox *buttonBox = new QDialogButtonBox(Qt::Vertical, this);
    QPushButton *donateButton = new QPushButton(tr("Donate"), this);
    QPushButton *bugButton = new QPushButton(tr("Report bug"), this);
    buttonBox->addButton(donateButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(bugButton, QDialogButtonBox::ActionRole);

    QGridLayout *grid = new QGridLayout(this);
    grid->addWidget(imageLabel, 0, 0);
    grid->addWidget(titleLabel, 0, 1);
    grid->addWidget(descriptionLabel, 1, 0, 1, 2);
    grid->addWidget(buttonBox, 1, 2);

    this->connect(donateButton, SIGNAL(clicked()), this, SLOT(donate()));
    this->connect(bugButton, SIGNAL(clicked()), this, SLOT(reportBug()));
}

AboutDialog::~AboutDialog() {}

void AboutDialog::donate() {
    QDesktopServices::openUrl(QUrl("https://www.paypal.com/cgi-bin/webscr?cmd=_donations&business=showarth@marxoft.co.uk&lc=GB&item_name=QDL&currency_code=USD&bn=PP%2dDonationsBF%3abtn_donate_SM%2egif%3aNonHosted"));
    this->accept();
}

void AboutDialog::reportBug() {
    QDesktopServices::openUrl(QUrl(QString("mailto:showarth@marxoft.co.uk?subject=QDL %1 for Maemo5").arg(VERSION_NUMBER)));
    this->accept();
}
