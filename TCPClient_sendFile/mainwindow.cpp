#include <QFileDialog>
#include <QDebug>
#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_totalBytes = 0;
    m_bytesReceived = 0;
    m_fileNameSize = 0;

    m_ipAddress = getLocalIP();
    qDebug() << "IP Address:" << m_ipAddress;

    connect(&m_tcpClient, SIGNAL(newConnection()) ,this, SLOT(acceptConnection()));    // Send newConnection() signal when a new connection is detected
}


MainWindow::~MainWindow()
{
    delete ui;
}


// Start to listen
void MainWindow::start()
{
    ui->u_startButton->setEnabled(false);
    m_bytesReceived = 0;

    QHostAddress ipAddress(m_ipAddress.toInt());
    if(!m_tcpClient.listen(ipAddress, 6666))
    {
        qDebug() << m_tcpClient.errorString();
        close();
        return;
    }
    ui->u_clientStatusLabel->setText(tr("Listen"));
}


// Accept new connection
void MainWindow::acceptConnection()
{
    m_tcpClientConnection = m_tcpClient.nextPendingConnection();
    connect(m_tcpClientConnection, SIGNAL(readyRead()), this, SLOT(updateClientProgress()));
    connect(m_tcpClientConnection, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(displayError(QAbstractSocket::SocketError)));

    ui->u_clientStatusLabel->setText(tr("Accept new connection"));
    m_tcpClient.close();
}


// Update progress bar, receive data
void MainWindow::updateClientProgress()
{
    QDataStream in(m_tcpClientConnection);
    in.setVersion(QDataStream::Qt_4_6);
    if(m_bytesReceived <= sizeof(qint64)*2)
    {
        // If received data length is less than 16 bytes, then it has just started, save incoming head information
        if((m_tcpClientConnection->bytesAvailable() >= sizeof(qint64)*2) && (m_fileNameSize == 0))
        {
            // Receive the total data length and the length of filename
            in >> m_totalBytes >> m_fileNameSize;
            m_bytesReceived += sizeof(qint64) * 2;
        }
        if((m_tcpClientConnection->bytesAvailable() >= m_fileNameSize) && (m_fileNameSize != 0))
        {
            // Receive the filename, and build the file
            in >> m_fileName;
            ui->u_clientStatusLabel->setText(tr("Receiving file %1 ...").arg(m_fileName));

            m_bytesReceived += m_fileNameSize;
            m_localFile = new QFile(m_fileName);
            if(!m_localFile->open(QFile::WriteOnly))
            {
                qDebug() << "Open file error !";
                return;
            }
        }
        else return;
    }
    if(m_bytesReceived < m_totalBytes)
    {
        // If received data length is less than total length, then write the file
        m_bytesReceived += m_tcpClientConnection->bytesAvailable();
        m_inBlock = m_tcpClientConnection->readAll();
        m_localFile->write(m_inBlock);
        m_inBlock.resize(0);
    }

    ui->u_clientProgressBar->setMaximum(m_totalBytes);
    ui->u_clientProgressBar->setValue(m_bytesReceived);

// Update progress bar
    if(m_bytesReceived == m_totalBytes)
    {
        // When receiving process is don
        m_tcpClientConnection->close();
        m_localFile->close();
        ui->u_startButton->setEnabled(true);
        ui->u_clientStatusLabel->setText(tr("Receive file %1 finished !").arg(m_fileName));
    }
}


// Error display
void MainWindow::displayError(QAbstractSocket::SocketError)
{
    qDebug() << m_tcpClientConnection->errorString();
    m_tcpClientConnection->close();

    ui->u_clientProgressBar->reset();
    ui->u_clientStatusLabel->setText(tr("Client is ready"));
    ui->u_startButton->setEnabled(true);
}


// Get the local IP address
QString MainWindow::getLocalIP()
{
QString ipAddress;
    QList<QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();
    for (int i = 0; i < ipAddressesList.size(); ++i) {
        if (ipAddressesList.at(i) != QHostAddress::LocalHost &&
            ipAddressesList.at(i).toIPv4Address()) {
            ipAddress = ipAddressesList.at(i).toString();
            break;
        }
    }
    if (ipAddress.isEmpty())
        ipAddress = QHostAddress(QHostAddress::LocalHost).toString();
    return ipAddress;
}



void MainWindow::on_u_startButton_clicked()
{
    start();
}
