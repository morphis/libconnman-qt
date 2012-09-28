/*
 * Copyright © 2010, Intel Corporation.
 * Copyright © 2012, Jolla.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 */

#ifndef NETWORKINGMODEL_H
#define NETWORKINGMODEL_H

#include <QDBusAbstractAdaptor>
#include <networkmanager.h>
#include <networktechnology.h>
#include <networkservice.h>

struct ServiceReqData
{
    QVariantMap fields;
    QDBusMessage reply;
    QDBusMessage msg;
};

class NetworkingModel : public QObject
{
    Q_OBJECT;

    Q_PROPERTY(bool available READ isAvailable NOTIFY availabilityChanged);
    Q_PROPERTY(QList<NetworkService*> services READ services NOTIFY networksChanged);

public:
    NetworkingModel(QObject* parent=0);
    virtual ~NetworkingModel();

    bool isAvailable() const;

    QList<NetworkService*> services() const;
    void requestUserInput(ServiceReqData* data);
    void reportError(const QString &error);

public slots:
    void requestScan() const;
    void sendUserReply(const QVariantMap &input);

signals:
    void availabilityChanged(bool available);
    void networksChanged();
    void technologiesChanged();
    void userInputRequested(QVariantMap fields);
    void errorReported(const QString &error);

private:
    NetworkManager* m_manager;
    ServiceReqData* m_req_data;

private slots:
    void updateTechnologies(const QMap<QString, NetworkTechnology*> &added,
                            const QStringList &removed);

    void managerAvailabilityChanged(bool available);

private:
    Q_DISABLE_COPY(NetworkingModel);
};

class UserInputAgent : public QDBusAbstractAdaptor
{
    Q_OBJECT;
    Q_CLASSINFO("D-Bus Interface", "net.connman.Agent");

public:
    UserInputAgent(NetworkingModel* parent);
    virtual ~UserInputAgent();

public slots:
    void Release();
    void ReportError(const QDBusObjectPath &service_path, const QString &error);
    void RequestBrowser(const QDBusObjectPath &service_path, const QString &url);
    Q_NOREPLY void RequestInput(const QDBusObjectPath &service_path,
                                const QVariantMap &fields,
                                const QDBusMessage &message);
    void Cancel();

private:
    NetworkingModel* m_networkingmodel;
};

#endif //NETWORKINGMODEL_H
