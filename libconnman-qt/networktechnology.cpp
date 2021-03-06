/*
 * Copyright © 2010, Intel Corporation.
 * Copyright © 2012, Jolla.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 */

#include "networktechnology.h"

const QString NetworkTechnology::Name("Name");
const QString NetworkTechnology::Type("Type");
const QString NetworkTechnology::Powered("Powered");
const QString NetworkTechnology::Connected("Connected");

NetworkTechnology::NetworkTechnology(const QString &path, const QVariantMap &properties, QObject* parent)
  : QObject(parent),
    m_technology(NULL)
{
    Q_ASSERT(!path.isEmpty());
    m_technology = new Technology("net.connman", path, QDBusConnection::systemBus(), this);

    if (!m_technology->isValid()) {
        qDebug() << "invalid technology: " << path;
        throw -1; // FIXME
    }

    m_propertiesCache = properties;

    connect(m_technology,
            SIGNAL(PropertyChanged(const QString&, const QDBusVariant&)),
            this,
            SLOT(propertyChanged(const QString&, const QDBusVariant&)));
}

NetworkTechnology::~NetworkTechnology() {}

// Public API

// Getters

const QString NetworkTechnology::name() const
{
    return m_propertiesCache[NetworkTechnology::Name].toString();
}

const QString NetworkTechnology::type() const
{
    return m_propertiesCache[NetworkTechnology::Type].toString();
}

const bool NetworkTechnology::powered() const
{
    return m_propertiesCache[NetworkTechnology::Powered].toBool();
}

const bool NetworkTechnology::connected() const
{
    return m_propertiesCache[NetworkTechnology::Connected].toBool();
}

// Setters

void NetworkTechnology::setPowered(const bool &powered)
{
    Q_ASSERT(m_technology);
    m_technology->SetProperty(Powered, QDBusVariant(QVariant(powered)));
}

void NetworkTechnology::scan()
{
    Q_ASSERT(m_technology);
    m_technology->Scan();
}

// Private

void NetworkTechnology::propertyChanged(const QString &name, const QDBusVariant &value)
{
    qDebug() << "NetworkTechnology: property " << name << " changed to " << value.variant();
    m_propertiesCache[name] = value.variant();

    if (name == Powered) {
        emit poweredChanged(m_propertiesCache[name].toBool());
    } else if (name == Connected) {
        emit connectedChanged(m_propertiesCache[name].toBool());
    }
}
