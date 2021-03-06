/*
 * Copyright © 2010, Intel Corporation.
 * Copyright © 2012, Jolla.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 */

#include "networkservice.h"

const QString NetworkService::Name("Name");
const QString NetworkService::State("State");
const QString NetworkService::Type("Type");
const QString NetworkService::Security("Security");
const QString NetworkService::Strength("Strength");
const QString NetworkService::Error("Error");
const QString NetworkService::Favorite("Favorite");
const QString NetworkService::IPv4("IPv4");
const QString NetworkService::IPv4Config("IPv4.Configuration");
const QString NetworkService::Nameservers("Nameservers");
const QString NetworkService::NameserversConfig("Nameservers.Configuration");
const QString NetworkService::Domains("Domains");
const QString NetworkService::DomainsConfig("Domains.Configuration");
const QString NetworkService::Proxy("Proxy");
const QString NetworkService::ProxyConfig("Proxy.Configuration");

NetworkService::NetworkService(const QString &path, const QVariantMap &properties, QObject* parent)
  : QObject(parent),
    m_service(NULL)
{
    Q_ASSERT(!path.isEmpty());
    m_service = new Service("net.connman", path, QDBusConnection::systemBus(), this);

    if (!m_service->isValid()) {
        qDebug() << "invalid service: " << path;
        throw -1; // FIXME
    }

    m_propertiesCache = properties;

    connect(m_service,
            SIGNAL(PropertyChanged(const QString&, const QDBusVariant&)),
            this,
            SLOT(propertyChanged(const QString&, const QDBusVariant&)));
}

NetworkService::~NetworkService() {}

const QString NetworkService::name() const {
    return m_propertiesCache.value(Name).toString();
}

const QString NetworkService::state() const {
    return m_propertiesCache.value(State).toString();
}

const QString NetworkService::type() const {
    return m_propertiesCache.value(Type).toString();
}

const QStringList NetworkService::security() const {
    return m_propertiesCache.value(Security).toStringList();
}

const uint NetworkService::strength() const {
    return m_propertiesCache.value(Strength).toUInt();
}

const bool NetworkService::favorite() const {
    return m_propertiesCache.value(Favorite).toBool();
}

const QVariantMap NetworkService::ipv4() const {
    return qdbus_cast<QVariantMap>(m_propertiesCache.value(IPv4));
}

const QVariantMap NetworkService::ipv4Config() const {
    return qdbus_cast<QVariantMap>(m_propertiesCache.value(IPv4Config));
}

const QStringList NetworkService::nameservers() const {
    return m_propertiesCache.value(Nameservers).toStringList();
}

const QStringList NetworkService::nameserversConfig() const {
    return m_propertiesCache.value(NameserversConfig).toStringList();
}

const QStringList NetworkService::domains() const {
    return m_propertiesCache.value(Domains).toStringList();
}

const QStringList NetworkService::domainsConfig() const {
    return m_propertiesCache.value(DomainsConfig).toStringList();
}

const QVariantMap NetworkService::proxy() const {
    return qdbus_cast<QVariantMap>(m_propertiesCache.value(Proxy));
}

const QVariantMap NetworkService::proxyConfig() const {
    return qdbus_cast<QVariantMap>(m_propertiesCache.value(ProxyConfig));
}

const QString NetworkService::dbusPath() const {
    return m_service->path();
}

void NetworkService::requestConnect()
{
    // increase reply timeout when connecting
    int timeout = CONNECT_TIMEOUT_FAVORITE;
    int old_timeout = m_service->timeout();
    if (!favorite()) {
        timeout = CONNECT_TIMEOUT;
    }
    m_service->setTimeout(timeout);
    QDBusPendingReply<> conn_reply = m_service->Connect();
    m_service->setTimeout(old_timeout);

    dbg_connectWatcher = new QDBusPendingCallWatcher(conn_reply, m_service);
    connect(dbg_connectWatcher,
            SIGNAL(finished(QDBusPendingCallWatcher*)),
            this,
            SLOT(dbg_connectReply(QDBusPendingCallWatcher*)));
}

void NetworkService::requestDisconnect()
{
    m_service->Disconnect();
}

void NetworkService::setIpv4Config(const QVariantMap &ipv4)
{
    // QDBusPendingReply<void> reply =
    m_service->SetProperty(IPv4Config, QDBusVariant(QVariant(ipv4)));
}

void NetworkService::setNameserversConfig(const QStringList &nameservers)
{
    // QDBusPendingReply<void> reply =
    m_service->SetProperty(NameserversConfig, QDBusVariant(QVariant(nameservers)));
}

void NetworkService::setDomainsConfig(const QStringList &domains)
{
    // QDBusPendingReply<void> reply =
    m_service->SetProperty(DomainsConfig, QDBusVariant(QVariant(domains)));
}

void NetworkService::setProxyConfig(const QVariantMap &proxy)
{
    // QDBusPendingReply<void> reply =
    m_service->SetProperty(ProxyConfig, QDBusVariant(QVariant(proxy)));
}

/* this slot is used for debugging */
void NetworkService::dbg_connectReply(QDBusPendingCallWatcher *call){
    qDebug() << "Got something from service.connect()";
    Q_ASSERT(call);
    QDBusPendingReply<> reply = *call;
    if (!reply.isFinished()) {
       qDebug() << "connect() not finished yet";
    }
    if (reply.isError()) {
        qDebug() << "Reply from service.connect(): " << reply.error().message();
    }
}

void NetworkService::propertyChanged(const QString &name, const QDBusVariant &value)
{
    qDebug() << "NetworkService: property " << name << " changed to " << value.variant();
    m_propertiesCache[name] = value.variant();

    if (name == Name) {
        emit nameChanged(m_propertiesCache[name].toString());
    } else if (name == State) {
        emit stateChanged(m_propertiesCache[name].toString());
    } else if (name == Security) {
        emit securityChanged(m_propertiesCache[name].toStringList());
    } else if (name == Strength) {
        emit strengthChanged(m_propertiesCache[name].toUInt());
    } else if (name == Favorite) {
        emit favoriteChanged(m_propertiesCache[name].toBool());
    } else if (name == IPv4) {
        emit ipv4Changed(qdbus_cast<QVariantMap>(m_propertiesCache.value(IPv4)));
    } else if (name == IPv4Config) {
        emit ipv4ConfigChanged(qdbus_cast<QVariantMap>(m_propertiesCache.value(IPv4Config)));
    } else if (name == Nameservers) {
        emit nameserversChanged(m_propertiesCache[name].toStringList());
    } else if (name == NameserversConfig) {
        emit nameserversConfigChanged(m_propertiesCache[name].toStringList());
    } else if (name == Domains) {
        emit domainsChanged(m_propertiesCache[name].toStringList());
    } else if (name == DomainsConfig) {
        emit domainsConfigChanged(m_propertiesCache[name].toStringList());
    } else if (name == Proxy) {
        emit proxyChanged(qdbus_cast<QVariantMap>(m_propertiesCache.value(Proxy)));
    } else if (name == ProxyConfig) {
        emit proxyConfigChanged(qdbus_cast<QVariantMap>(m_propertiesCache.value(ProxyConfig)));
    }
}
