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

    QTcpServer m_tcpClient;
    QTcpSocket *m_tcpClientConnection;
    qint64 m_totalBytes;    // Total bytes of data
    qint64 m_bytesReceived;    // Length of received data
    qint64 m_fileNameSize;    // Information of filename length
    QString m_fileName;    // Save filename
    QFile *m_localFile;    // Local file
    QByteArray m_inBlock;    // Data buffer
    QString m_ipAddress;

private slots:
    void start();    // Start to listen port
    void acceptConnection();    // Build connection
    void updateClientProgress();    //Update progress bar, receive the data
    void displayError(QAbstractSocket::SocketError socketError);    //Display the error
    QString getLocalIP();
    void on_u_startButton_clicked();
};

#endif // MAINWINDOW_H
