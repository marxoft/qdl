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

#ifndef CHECKURLSDIALOG_H
#define CHECKURLSDIALOG_H

#include <QDialog>

class QTreeView;
class QProgressBar;
class QLabel;
class QDialogButtonBox;

class CheckUrlsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CheckUrlsDialog(QWidget *parent = 0);
    ~CheckUrlsDialog();

protected:
    void hideEvent(QHideEvent *event);

private slots:
    void onProgressChanged(int progress);
    void onCanceled();
    void resetDialog();
    
private:
    QTreeView *m_view;
    QProgressBar *m_progressBar;
    QLabel *m_infoLabel;
    QDialogButtonBox *m_buttonBox;
    QPushButton *m_okButton;
    QPushButton *m_cancelButton;
};

#endif // CHECKURLSDIALOG_H
