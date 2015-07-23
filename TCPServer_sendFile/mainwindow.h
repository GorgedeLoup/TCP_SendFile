#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtNetwork/QtNetwork>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    QTcpSocket *m_tcpServer;
    QFile *m_localFile;    // File to send
    qint64 m_totalBytes;    // Total bytes of data
    qint64 m_bytesWritten;    // Bytes of data sent
    qint64 m_bytesToWrite;    // Bytes remains
    qint64 m_loadSize;    // Bytes of every step of sending
    QString m_fileName;    // Path
    QByteArray m_outBlock;    // Data buffer

private slots:
    void send();    // Connect to client
    void startTransfer();    // Start session
    void updateServerProgress(qint64);    // Sending, update the progress bar
    void displayError(QAbstractSocket::SocketError);    // Display error
    void openFile();    // Open file
    void on_u_openButton_clicked();
    void on_u_sendButton_clicked();
};

#endif // MAINWINDOW_H
