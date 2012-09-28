/*
 * Copyright © 2010, Intel Corporation.
 * Copyright © 2012, Jolla.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 */

#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include "commondbustypes.h"
#include "manager.h"
#include "networktechnology.h"
#include "networkservice.h"
#include <QtDBus>

class NetworkManager;

class NetworkManagerFactory
{
public:
    NetworkManagerFactory() {}

    static NetworkManager* createInstance();
};

class NetworkManager : public QObject
{
    Q_OBJECT;

    Q_PROPERTY(bool available READ isAvailable NOTIFY availabilityChanged);
    Q_PROPERTY(QString state READ state NOTIFY stateChanged);
    Q_PROPERTY(bool offlineMode READ offlineMode WRITE setOfflineMode NOTIFY offlineModeChanged);
    Q_PROPERTY(NetworkService* defaultRoute READ defaultRoute NOTIFY defaultRouteChanged);

public:
    NetworkManager(QObject* parent=0);
    virtual ~NetworkManager();

    bool isAvailable() const;

    NetworkTechnology* getTechnology(const QString &type) const;
    QList<NetworkTechnology*> listTechnologies() const;
    const QVector<NetworkService*> getServices() const;

    const QString state() const;
    bool offlineMode() const;
    NetworkService* defaultRoute() const;

public slots:
    void setOfflineMode(const bool &offlineMode);
    void registerAgent(const QString &path);
    void unregisterAgent(const QString &path);

signals:
    void availabilityChanged(bool available);

    void stateChanged(const QString &state);
    void offlineModeChanged(bool offlineMode);
    void technologiesChanged(const QMap<QString, NetworkTechnology*> &added,
                             const QStringList &removed);
    void servicesChanged();
    void defaultRouteChanged(NetworkService* defaultRoute);

private:
    Manager *m_manager;

    QDBusPendingCallWatcher *m_getPropertiesWatcher;
    QDBusPendingCallWatcher *m_getTechnologiesWatcher;
    QDBusPendingCallWatcher *m_getServicesWatcher;
    QVariantMap m_propertiesCache;
    QMap<QString, NetworkTechnology *> m_technologiesCache;
    QMap<QString, NetworkService *> m_servicesCache;
    QMap<QString, uint> m_servicesOrder;
    NetworkService* m_defaultRoute;

    QDBusServiceWatcher *watcher;

    static const QString State;
    static const QString OfflineMode;

    bool m_available;

private slots:
    void connectToConnman(QString = "");
    void disconnectFromConnman(QString = "");
    void connmanUnregistered(QString = "");
    void getPropertiesReply(QDBusPendingCallWatcher *call);
    void getTechnologiesReply(QDBusPendingCallWatcher *call);
    void getServicesReply(QDBusPendingCallWatcher *call);
    void propertyChanged(const QString &name, const QDBusVariant &value);
    void updateServices(const ConnmanObjectList &changed, const QList<QDBusObjectPath> &removed);
    void updateDefaultRoute(NetworkService* defaultRoute);

private:
    Q_DISABLE_COPY(NetworkManager);
};

#endif //NETWORKMANAGER_H
