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

#include "archiveextractor.h"
#include <QDebug>

ArchiveExtractor::ArchiveExtractor(QObject *parent) :
    QObject(parent),
    m_process(0),
    m_createSubfolder(false)
{
}

ArchiveExtractor::~ArchiveExtractor() {}

QString ArchiveExtractor::fileName() const {
    return m_fileName;
}

void ArchiveExtractor::setFileName(const QString &fileName) {
    m_fileName = fileName;
}

QString ArchiveExtractor::outputDirectory() const {
    return m_outputDirectory;
}

void ArchiveExtractor::setOutputDirectory(const QString &directory) {
    m_outputDirectory = directory.endsWith('/') ? directory : directory + '/';
}

bool ArchiveExtractor::createSubfolder() const {
    return m_createSubfolder;
}

void ArchiveExtractor::setCreateSubfolder(bool subfolder) {
    m_createSubfolder = subfolder;
}

QString ArchiveExtractor::password() const {
    return m_password;
}

void ArchiveExtractor::setPassword(const QString &password) {
    m_password = password;
}

QString ArchiveExtractor::errorString() const {
    return m_errorString;
}

void ArchiveExtractor::setErrorString(const QString &errorString) {
    m_errorString = errorString;
}

void ArchiveExtractor::start() {
    if ((this->fileName().isEmpty()) || (this->outputDirectory().isEmpty())) {
        this->setErrorString(tr("No input filename and/or output directory specified"));
        emit error();
        return;
    }

    QString command;

    QString fileSuffix = this->fileName().mid(this->fileName().lastIndexOf('.') + 1);
    QString subFolder = this->fileName().mid(this->fileName().lastIndexOf('/') + 1);
    subFolder = subFolder.left(subFolder.lastIndexOf(QRegExp("p(ar|)t\\d+\\.", Qt::CaseInsensitive)));
    subFolder = subFolder.left(subFolder.lastIndexOf('.'));

    if (fileSuffix == "rar") {
        command = "unrar x -or -p-";

        if (this->createSubfolder()) {
            command.append(" -ad");
        }

        if (!this->password().isEmpty()) {
            command.append(" -p" + this->password());
        }

        command.append(QString(" \"%1\" \"%2\"").arg(this->fileName()).arg(this->outputDirectory()));
    }
    else if (fileSuffix == "zip") {
        command = QString("unzip -n \"%1\"").arg(this->fileName());

        if (!this->password().isEmpty()) {
            command.append(" -P " + this->password());
        }

        command.append(QString(" -d \"%1%2\"").arg(this->outputDirectory()).arg(this->createSubfolder() ? subFolder : ""));
    }
    else if (fileSuffix == "gz") {
        command = QString("untar xvf \"%1\" -C \"%2%3\"")
                  .arg(this->fileName())
                  .arg(this->outputDirectory())
                  .arg(this->createSubfolder() ? subFolder : "");
    }

    qDebug() << "Extracting files with command:" << command;


    if (command.isEmpty()) {
        this->setErrorString(tr("No handler found for archive"));
        emit error();
    }
    else {
        if (!m_process) {
            m_process = new QProcess(this);
            this->connect(m_process, SIGNAL(finished(int,QProcess::ExitStatus)),
                          this, SLOT(onProcessFinished(int,QProcess::ExitStatus)));
        }

        if (m_process->state() != QProcess::Running) {
            m_process->start(command);
        }
    }
}

void ArchiveExtractor::start(const QString &fileName, const QString &outputDirectory, const QString &password, bool createSubfolder) {
    this->setFileName(fileName);
    this->setOutputDirectory(outputDirectory);
    this->setPassword(password);
    this->setCreateSubfolder(createSubfolder);
    this->start();
}

void ArchiveExtractor::onProcessFinished(int exitCode, QProcess::ExitStatus status) {
    if ((exitCode == 0) && (status == QProcess::NormalExit)) {
        emit finished();
    }
    else {
        this->setErrorString(m_process ? m_process->errorString() : tr("Cannot extract archive"));
        emit error();
    }
}
