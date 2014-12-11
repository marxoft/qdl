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

#include "audioconverter.h"

AudioConverter::AudioConverter(QObject *parent) :
    QObject(parent),
    m_process(0)
{
}

AudioConverter::~AudioConverter() {}

QString AudioConverter::fileName() const {
    return m_fileName;
}

void AudioConverter::setFileName(const QString &fileName) {
    m_fileName = fileName;
}

QString AudioConverter::outputDirectory() const {
    return m_outputDirectory;
}

void AudioConverter::setOutputDirectory(const QString &directory) {
    m_outputDirectory = directory.endsWith('/') ? directory : directory + '/';
}

QString AudioConverter::errorString() const {
    return m_errorString;
}

void AudioConverter::setErrorString(const QString &errorString) {
    m_errorString = errorString;
}

void AudioConverter::start() {
    if ((this->fileName().isEmpty()) || (this->outputDirectory().isEmpty())) {
        this->setErrorString(tr("No input filename and/or output directory specified"));
        emit error();
        return;
    }

    if (!m_process) {
        m_process = new QProcess(this);
        this->connect(m_process, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(onProcessFinished(int,QProcess::ExitStatus)));
    }

    if (m_process->state() != QProcess::Running) {
        m_process->start(QString("ffmpeg -i \"%1\" -acodec copy -y -vn \"%2\"")
                         .arg(this->fileName())
                         .arg(this->outputDirectory()
                         + this->fileName().section('/', -1).section('.', 0, -2) + ".m4a"));
    }
}

void AudioConverter::start(const QString &fileName, const QString &outputDirectory) {
    this->setFileName(fileName);
    this->setOutputDirectory(outputDirectory);
    this->start();
}

void AudioConverter::onProcessFinished(int exitCode, QProcess::ExitStatus status) {
    if ((exitCode == 0) && (status == QProcess::NormalExit)) {
        emit finished();
    }
    else {
        this->setErrorString(m_process ? m_process->errorString() : tr("Cannot convert input file to audio"));
        emit error();
    }
}
