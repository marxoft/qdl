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

#include "captchadialog.h"
#include "../shared/utils.h"
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QTimer>

CaptchaDialog::CaptchaDialog(QWidget *parent) :
    QDialog(parent),
    m_imageLabel(new QLabel(this)),
    m_responseEdit(new QLineEdit(this)),
    m_timeoutLabel(new QLabel(this)),
    m_doneButton(0),
    m_cancelButton(0),
    m_timer(0)
{
    this->setWindowTitle(tr("Please complete captcha"));
    this->setAttribute(Qt::WA_DeleteOnClose, true);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(this);
    QGridLayout *grid = new QGridLayout(this);

    m_doneButton = buttonBox->addButton(QDialogButtonBox::Ok);
    m_cancelButton = buttonBox->addButton(QDialogButtonBox::Cancel);

#ifdef Q_WS_MAEMO_5
    buttonBox->setOrientation(Qt::Vertical);
    grid->addWidget(m_imageLabel, 0, 0);
    grid->addWidget(m_timeoutLabel, 0, 1, 1, 1, Qt::AlignTop);
    grid->addWidget(m_responseEdit, 1, 0);
    grid->addWidget(buttonBox, 1, 1);

    m_imageLabel->setFixedHeight(200);
    m_imageLabel->setScaledContents(true);
#elif defined MAEMO4_OS
    buttonBox->setOrientation(Qt::Horizontal);
    grid->addWidget(m_imageLabel, 0, 0, 1, 2);
    grid->addWidget(m_responseEdit, 1, 0, 1, 2);
    grid->addWidget(m_timeoutLabel, 2, 0);
    grid->addWidget(buttonBox, 2, 1);
#else
    m_doneButton->setIcon(QIcon::fromTheme("dialog-ok"));
    m_cancelButton->setIcon(QIcon::fromTheme("dialog-cancel"));

    buttonBox->setOrientation(Qt::Horizontal);
    grid->addWidget(m_imageLabel, 0, 0, 1, 2);
    grid->addWidget(m_responseEdit, 1, 0, 1, 2);
    grid->addWidget(m_timeoutLabel, 2, 0);
    grid->addWidget(buttonBox, 2, 1);
#endif
    
    m_imageLabel->setFrameStyle(QFrame::StyledPanel);
    m_doneButton->setEnabled(false);
    m_timeoutLabel->hide();    

    this->connect(m_responseEdit, SIGNAL(textChanged(QString)), this, SLOT(onCaptchaTextChanged(QString)));
    this->connect(m_doneButton, SIGNAL(clicked()), this, SLOT(submitCaptchaResponse()));
    this->connect(m_cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
}

CaptchaDialog::~CaptchaDialog() {}

void CaptchaDialog::setCaptchaFileName(const QString &fileName) {
    m_imageLabel->setPixmap(QPixmap(fileName));
}

void CaptchaDialog::setTimeout(int secs) {
    if (secs > 0) {
        m_timeout = secs;

        if (!m_timer) {
            m_timer = new QTimer(this);
            this->connect(m_timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
        }

        m_timeoutLabel->setText(Utils::durationFromSecs(m_timeout));
        m_timeoutLabel->show();
        m_timer->start(1000);
    }
    else {
        m_timeoutLabel->hide();

        if (m_timer) {
            m_timer->stop();
        }
    }
}

void CaptchaDialog::onTimeout() {
    m_timeout--;
    m_timeoutLabel->setText(Utils::durationFromSecs(m_timeout));

    if (m_timeout <= 0) {
        this->reject();
    }
}

void CaptchaDialog::onCaptchaTextChanged(const QString &text) {
    m_doneButton->setEnabled(!text.isEmpty());
}

void CaptchaDialog::submitCaptchaResponse() {
    emit captchaResponseReady(m_responseEdit->text());
    this->accept();
}
