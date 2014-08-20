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

#ifndef DECAPTCHAINTERFACE_H
#define DECAPTCHAINTERFACE_H

#include <QtPlugin>

class DecaptchaPlugin;

class DecaptchaInterface
{

public:
    virtual ~DecaptchaInterface() {}
    virtual DecaptchaPlugin* getDecaptchaPlugin() = 0;
    virtual DecaptchaPlugin* createDecaptchaPlugin() = 0;
};

Q_DECLARE_INTERFACE(DecaptchaInterface, "com.marxoft.QDL.DecaptchaInterface/1.0")

#endif // SERVICEINTERFACE_H
