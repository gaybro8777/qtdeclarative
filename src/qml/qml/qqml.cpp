/****************************************************************************
**
** Copyright (C) 2019 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtQml module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qqml.h"

#include <QtQml/qqmlprivate.h>

#include <private/qqmlengine_p.h>
#include <private/qqmlmetatype_p.h>
#include <private/qqmlmetatypedata_p.h>
#include <private/qqmltype_p_p.h>
#include <private/qqmltypemodule_p_p.h>

#include <QtCore/qmutex.h>

QT_BEGIN_NAMESPACE

void qmlClearTypeRegistrations() // Declared in qqml.h
{
    QQmlMetaType::clearTypeRegistrations();
    QQmlEnginePrivate::baseModulesUninitialized = true; //So the engine re-registers its types
    qmlClearEnginePlugins();
}

//From qqml.h
bool qmlProtectModule(const char *uri, int majVersion)
{
    return QQmlMetaType::protectModule(uri, majVersion);
}

//From qqml.h
void qmlRegisterModule(const char *uri, int versionMajor, int versionMinor)
{
    QQmlMetaType::registerModule(uri, versionMajor, versionMinor);
}

//From qqml.h
int qmlTypeId(const char *uri, int versionMajor, int versionMinor, const char *qmlName)
{
    return QQmlMetaType::typeId(uri, versionMajor, versionMinor, qmlName);
}

/*
This method is "over generalized" to allow us to (potentially) register more types of things in
the future without adding exported symbols.
*/
int QQmlPrivate::qmlregister(RegistrationType type, void *data)
{
    if (type == AutoParentRegistration) {
        return QQmlMetaType::registerAutoParentFunction(
                *reinterpret_cast<RegisterAutoParent *>(data));
    } else if (type == QmlUnitCacheHookRegistration) {
        return QQmlMetaType::registerUnitCacheHook(
                *reinterpret_cast<RegisterQmlUnitCacheHook *>(data));
    }

    QQmlType dtype;
    if (type == TypeRegistration)
        dtype = QQmlMetaType::registerType(*reinterpret_cast<RegisterType *>(data));
    else if (type == InterfaceRegistration)
        dtype = QQmlMetaType::registerInterface(*reinterpret_cast<RegisterInterface *>(data));
    else if (type == SingletonRegistration)
        dtype = QQmlMetaType::registerSingletonType(*reinterpret_cast<RegisterSingletonType *>(data));
    else if (type == CompositeRegistration)
        dtype = QQmlMetaType::registerCompositeType(*reinterpret_cast<RegisterCompositeType *>(data));
    else if (type == CompositeSingletonRegistration)
        dtype = QQmlMetaType::registerCompositeSingletonType(*reinterpret_cast<RegisterCompositeSingletonType *>(data));
    else
        return -1;

    if (!dtype.isValid())
        return -1;

    QQmlMetaType::registerUndeletableType(dtype);
    return dtype.index();
}

QT_END_NAMESPACE