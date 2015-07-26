#include <QFileDialog>
#include <QDebug>
#include "mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_loadSize = 4*1024;
    m_totalBytes = 0;
    m_bytesWritten = 0;
    m_bytesToWrite = 0;
    m_tcpServer = new QTcpSocket(this);

    connect(m_tcpServer, SIGNAL(connected()), this, SLOT(startTransfer()));    // If connection is built, send connected signal, start session
    connect(m_tcpServer, SIGNAL(bytesWritten(qint64)), this, SLOT(updateServerProgress(qint64)));    // Update the progress bar

    connect(m_tcpServer, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(displayError(QAbstractSocket::SocketError)));

    ui->u_sendButton->setEnabled(false);
}


MainWindow::~MainWindow()
{
    delete ui;
}


// Open file
void MainWindow::openFile()
{
    m_fileName = QFileDialog::getOpenFileName(this);
    if(!m_fileName.isEmpty())
    {
        ui->u_sendButton->setEnabled(true);
        ui->u_serverStatusLabel->setText(tr("Open file %1 success !")
                                       .arg(m_fileName));
    }
}


// Connect to client and start session
void MainWindow::send()
{
    ui->u_sendButton->setEnabled(false);
    m_bytesWritten = 0;
    // Initialize the sent data as 0
    ui->u_serverStatusLabel->setText(tr("Connecting..."));
    m_tcpServer->connectToHost(ui->u_hostLineEdit->text(),
                             ui->u_portLineEdit->text().toInt());    // Connect
}


// Send file information
void MainWindow::startTransfer()
{
    m_localFile = new QFile(m_fileName);
    if(!m_localFile->open(QFile::ReadOnly))
    {
        qDebug() << "Open file error !";
        return;
    }
    qDebug() << "m_fileName:" << m_fileName;
    m_totalBytes = m_localFile->size();    // Total bytes of the file

    QDataStream m_sendOut(&m_outBlock, QIODevice::WriteOnly);
    m_sendOut.setVersion(QDataStream::Qt_4_6);
    QString currentFileName = m_fileName.right(m_fileName.size() - m_fileName.lastIndexOf('/')-1);
    m_sendOut << qint64(0) << qint64(0) << currentFileName;
    qDebug() << "currentFileName:" << currentFileName;
// Write total bytes space, filename space, filename in order
    m_totalBytes += m_outBlock.size();
// m_totalBytes is the bytes information of filename space and the actual bytes of the file
    m_sendOut.device()->seek(0);
    m_sendOut << m_totalBytes << qint64((m_outBlock.size() - sizeof(qint64)*2));
// Return the start of m_outBlock, replace two qint64 spaces by actual length information
    m_bytesToWrite = m_totalBytes - m_tcpServer->write(m_outBlock);
// The rest data length after the head information
    ui->u_serverStatusLabel->setText(tr("Connected"));
    m_outBlock.resize(0);
}


// Update the progress bar, transfer the file
void MainWindow::updateServerProgress(qint64 numBytes)
{
    m_bytesWritten += (int)numBytes;
// Length of sent data
    if(m_bytesToWrite > 0) // If any data has already been sent
    {
        m_outBlock = m_localFile->read(qMin(m_bytesToWrite, m_loadSize));
// The data length of every send progress, here 4KB as default, if less than it, send the rest
        m_bytesToWrite -= (int)m_tcpServer->write(m_outBlock);
// Length of remaining data
        m_outBlock.resize(0);
// Clear the data buffer
    }
    else
    {
        m_localFile->close();    // If no data is sent, close file
    }
    ui->u_serverProgressBar->setMaximum(m_totalBytes);
    ui->u_serverProgressBar->setValue(m_bytesWritten);
// Update the progress bar
    if(m_bytesWritten == m_totalBytes)    // Send over
    {
        ui->u_serverStatusLabel->setText(tr("Send file %1 finished")
                                            .arg(m_fileName));
        m_localFile->close();
        m_tcpServer->close();
    }
}


void MainWindow::displayError(QAbstractSocket::SocketError)    // Display error report
{
    qDebug() << m_tcpServer->errorString();
    m_tcpServer->close();
    ui->u_serverProgressBar->reset();
    ui->u_serverStatusLabel->setText(tr("Server is ready"));
    ui->u_sendButton->setEnabled(true);
}


void MainWindow::on_u_openButton_clicked()    // Enable the open button
{
    openFile();
}

void MainWindow::on_u_sendButton_clicked()    // Enable the send button
{
    send();
}
