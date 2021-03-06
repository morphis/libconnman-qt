/*
 * Copyright © 2010, Intel Corporation.
 * Copyright © 2012, Jolla.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 */

#include "QDebug"
#include "networkingmodel.h"

#define AGENT_PATH "/WifiSettings"

NetworkingModel::NetworkingModel(QObject* parent)
  : QObject(parent),
    m_manager(NULL),
    m_wifi(NULL)
{
    m_manager = NetworkManagerFactory::createInstance();
    new UserInputAgent(this);

    m_wifi = m_manager->getTechnology("wifi"); // TODO: use constant literal
    if (m_wifi) {
        connect(m_wifi,
                SIGNAL(poweredChanged(bool)),
                this,
                SIGNAL(wifiPoweredChanged(bool)));
    }
    connect(m_manager, SIGNAL(availabilityChanged(bool)),
            this, SLOT(managerAvailabilityChanged(bool)));
    connect(m_manager,
            SIGNAL(technologiesChanged(QMap<QString, NetworkTechnology*>, QStringList)),
            this,
            SLOT(updateTechnologies(QMap<QString, NetworkTechnology*>, QStringList)));
    connect(m_manager,
            SIGNAL(servicesChanged()),
            this,
            SIGNAL(networksChanged()));
    QDBusConnection::systemBus().registerObject(AGENT_PATH, this);
    m_manager->registerAgent(QString(AGENT_PATH));
}

NetworkingModel::~NetworkingModel()
{
    m_manager->unregisterAgent(QString(AGENT_PATH));
}

bool NetworkingModel::isAvailable() const
{
    return m_manager->isAvailable();
}

QList<QObject*> NetworkingModel::networks() const
{
    const QString wifi("wifi");
    QList<QObject*> networks;
    // TODO: get rid of double looping, do filtering in NM::getServices()
    foreach (NetworkService* network, m_manager->getServices()) {
        if (network->type() == wifi) {
            networks.append(network);
        }
    }
    return networks;
}

bool NetworkingModel::isWifiPowered() const
{
    if (m_wifi) {
        return m_wifi->powered();
    } else {
        qWarning() << "Can't get: wifi technology is NULL";
        return false;
    }
}

void NetworkingModel::setWifiPowered(const bool &wifiPowered)
{
    if (m_wifi) {
        m_wifi->setPowered(wifiPowered);
    } else {
        qWarning() << "Can't set: wifi technology is NULL";
    }
}

void NetworkingModel::requestScan() const
{
    qDebug() << "scan requested for wifi";
    if (m_wifi) {
        m_wifi->scan();
    }
}

void NetworkingModel::updateTechnologies(const QMap<QString, NetworkTechnology*> &added,
                                         const QStringList &removed)
{
    QString wifi_str = QString("wifi");
    if (added.contains(wifi_str)) {
        m_wifi = added.value(wifi_str);
        connect(m_wifi,
                SIGNAL(poweredChanged(bool)),
                this,
                SIGNAL(wifiPoweredChanged(bool)));
    }
    if (removed.contains(wifi_str)) {
        m_wifi = NULL; // FIXME: is it needed?
    }
    emit technologiesChanged();
}

void NetworkingModel::managerAvailabilityChanged(bool available)
{
    if(available)
        m_manager->registerAgent(QString(AGENT_PATH));

    emit availabilityChanged(available);
}

void NetworkingModel::requestUserInput(ServiceReqData* data)
{
    m_req_data = data;
    emit userInputRequested(data->fields);
}

void NetworkingModel::reportError(const QString &error) {
    emit errorReported(error);
}

void NetworkingModel::sendUserReply(const QVariantMap &input) {
    if (!input.isEmpty()) {
        QDBusMessage &reply = m_req_data->reply;
        reply << input;
        QDBusConnection::systemBus().send(reply);
    } else {
        QDBusMessage error = m_req_data->msg.createErrorReply(
            QString("net.connman.Agent.Error.Canceled"),
            QString("canceled by user"));
        QDBusConnection::systemBus().send(error);
    }
    delete m_req_data;
}

// DBus-adaptor for NetworkingModel /////////////////////

UserInputAgent::UserInputAgent(NetworkingModel* parent)
  : QDBusAbstractAdaptor(parent),
    m_networkingmodel(parent)
{
    // TODO
}

UserInputAgent::~UserInputAgent() {}

void UserInputAgent::Release()
{
    // here do clean up
}

void UserInputAgent::ReportError(const QDBusObjectPath &service_path, const QString &error)
{
    qDebug() << "From " << service_path.path() << " got this error:\n" << error;
    m_networkingmodel->reportError(error);
}

void UserInputAgent::RequestBrowser(const QDBusObjectPath &service_path, const QString &url)
{
    qDebug() << "Service " << service_path.path() << " wants browser to open hotspot's url " << url;
}

void UserInputAgent::RequestInput(const QDBusObjectPath &service_path,
                                       const QVariantMap &fields,
                                       const QDBusMessage &message)
{
    QVariantMap json;
    qDebug() << "Service " << service_path.path() << " wants user input";
    foreach (const QString &key, fields.keys()){
        QVariantMap payload = qdbus_cast<QVariantMap>(fields[key]);
        json.insert(key, payload);
    }
    ServiceReqData *reqdata = new ServiceReqData;
    reqdata->fields = json;
    message.setDelayedReply(true);
    reqdata->reply = message.createReply();
    reqdata->msg = message;
    m_networkingmodel->requestUserInput(reqdata);
}

void UserInputAgent::Cancel()
{
    qDebug() << "WARNING: request to agent got canceled";
}
