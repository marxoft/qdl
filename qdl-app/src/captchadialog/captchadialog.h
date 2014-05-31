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

#ifndef CAPTCHADIALOG_H
#define CAPTCHADIALOG_H

#include <QDialog>

class QLabel;
class QLineEdit;
class QPushButton;
class QTimer;

class CaptchaDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CaptchaDialog(QWidget *parent = 0);
    ~CaptchaDialog();
    
public slots:
    void setCaptchaFileName(const QString &fileName);
    void setTimeout(int secs);

private slots:
    void onCaptchaTextChanged(const QString &text);
    void submitCaptchaResponse();
    void onTimeout();

signals:
    void captchaResponseReady(const QString &text);

private:
    QLabel *m_imageLabel;
    QLineEdit *m_responseEdit;
    QLabel *m_timeoutLabel;
    QPushButton *m_doneButton;
    QPushButton *m_cancelButton;
    QTimer *m_timer;
    int m_timeout;
};

#endif // CAPTCHADIALOG_H
