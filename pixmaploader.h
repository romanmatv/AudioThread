#ifndef PIXMAPLOADER_H
#define PIXMAPLOADER_H

//#include <QNetworkAccessManager>
#include <QtNetwork/qnetworkaccessmanager.h>
#include <QPixmap>

class PixmapLoader : public QObject {
    Q_OBJECT
public:
    PixmapLoader(QObject *parent = nullptr);

    struct NetworReplyException : public std::invalid_argument{
        NetworReplyException(std::string what) : std::invalid_argument(what) {
        }
    };
public slots:
    void load(QString imageUrl);
private slots:
    void on_load(QNetworkReply* reply);
signals:
    void loaded(QPixmap);
private:
    QNetworkAccessManager m_manager;
};

#endif // PIXMAPLOADER_H
