#include "track.h"

Track::Track(QUrl _path, QString _radioName){
    m_manager = new QNetworkAccessManager();
    path = _path;
    title = path.fileName();
    radioName = _radioName;
    //connect(m_manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(onPage_loaded(QNetworkReply*)));
}

Track::~Track()
{
    delete m_manager;
}

QString Track::captionFormated(QString format){
    QString fooString("{title}");
    format.replace(format.indexOf(fooString), fooString.size(), title);
    fooString = "{artist}";
    format.replace(format.indexOf(fooString), fooString.size(), artist);
    fooString = "{radioName}";
    format.replace(format.indexOf(fooString), fooString.size(), radioName);
    fooString = "{radioname}";
    format.replace(format.indexOf(fooString), fooString.size(), radioName);
    fooString = "{genre}";
    format.replace(format.indexOf(fooString), fooString.size(), genre);
    fooString = "{album}";
    format.replace(format.indexOf(fooString), fooString.size(), album);
    fooString = "{date}";
    format.replace(format.indexOf(fooString), fooString.size(), date);

    return format;
}

void Track::clear(){
    title.clear();
    artist.clear();
    album.clear();
    genre.clear();
    date.clear();
}

void Track::search_cover(QString search){
    last_search = search;
    //qDebug() << "Пробуем загрузиться с яндекса: "<<search;
    parse("https://yandex.ru/images/search?text="+search+" cover art");
}

void Track::parse(QString url) {
    QNetworkRequest request(QUrl::fromUserInput(url));
    request.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("text/plain"));
    request.setHeader(QNetworkRequest::UserAgentHeader, "RomanMatv Browser 1.0");
    //m_manager->get(request);

    reply = m_manager->get(request);

    //connect(m_manager, SIGNAL(finished(QNetworkReply *)), this, SLOT(onPage_loaded(QNetworkReply *)));

    //connect(reply, SIGNAL(readyRead()), this, SLOT(slotReadyRead()));
    connect(reply, SIGNAL(finished()), this, SLOT(slotReadyRead()));
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
            this, SLOT(slotError(QNetworkReply::NetworkError)));
    connect(reply, SIGNAL(sslErrors(QList<QSslError>)),
            this, SLOT(slotSslErrors(QList<QSslError>)));
}

void Track::slotReadyRead(){
    //qDebug() << "Сработал slotReadyRead";

    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    QString response;// = reply->readAll();
    if (reply){
        if (reply->error() == QNetworkReply::NoError)
        {
            QByteArray buffer = reply->readAll();
            response = QString::fromUtf8(buffer);
        }
        else
        {
            response =  tr("Error: %1 status: %2").arg(reply->errorString(), reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toString());
        }
        finding(response);
        //qDebug()<<"code: "<<reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toString();
        reply->deleteLater();
    }
}
void Track::slotError(QNetworkReply::NetworkError error){
    //qDebug() << "Сработал slotError";
    qDebug() << error;
}
void Track::slotSslErrors(QList<QSslError> errors){
    //qDebug() << "Сработал slotSslErrors";
    for (int i = 0; i< errors.count();i++)
        qDebug() << errors[i].errorString();
}

void Track::finding(QString response){
    //qDebug() << "===================RESPONSE====================";
    //qDebug() << response;
    //qDebug() << "===============================================";
    QRegExp iconsRegExp_yandex("\"origin\":{\"w\":\\d+,\"h\":\\d+,\"url\":\"([\\/:\\w\\d\\/\\?.;=\\-&%]+)\"},");
    iconsRegExp_yandex.setMinimal(true);

    QRegExp iconsRegExp_google("{\"clt\":\"n\",\"id\"([\\w:\",.]+)(\"ou\":\")([\\/:\\w\\d\\/\\?.;=\\-&%]+)(\",)\"");
    iconsRegExp_google.setMinimal(true);

    QStringList icons;

    //qDebug()<<"RegExp_yandex";
    int lastPos = 0;
    while ((lastPos = iconsRegExp_yandex.indexIn(response, lastPos)) != -1) {
        QStringList iconData;
        lastPos += iconsRegExp_yandex.matchedLength();

        icons.push_back("https:"+iconsRegExp_yandex.cap(1));
        //qDebug() << iconsRegExp_google.cap(1);
    }

    //qDebug()<<"RegExp_google";
    lastPos = 0;
    while ((lastPos = iconsRegExp_google.indexIn(response, lastPos)) != -1) {
        QStringList iconData;
        lastPos += iconsRegExp_google.matchedLength();

        icons.push_back("https:"+iconsRegExp_google.cap(3));
        //qDebug() << iconsRegExp_google.cap(3);
    }

    emit finished(icons);
    onFinish(icons);
}

void Track::onPage_loaded(QNetworkReply *reply) {
    QString response = reply->readAll();

    if (reply->error() != QNetworkReply::NoError){
        qDebug() << tr("Error: %1 status: %2").arg(reply->errorString(), reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toString());
    }

    //QString buff = reply->readAll();

    //qDebug() << reply->url();

    //qDebug() << "Страница загружена: " << response;

    QRegExp iconsRegExp_yandex("\"origin\":{\"w\":\\d+,\"h\":\\d+,\"url\":\"([\\/:\\w\\d\\/\\?.;=\\-&%]+)\"},");
    iconsRegExp_yandex.setMinimal(true);

    QRegExp iconsRegExp_google("{\"clt\":\"n\",\"id\"([\\w:\",.]+)(\"ou\":\")([\\/:\\w\\d\\/\\?.;=\\-&%]+)(\",)\"");
    iconsRegExp_google.setMinimal(true);

    QStringList icons;

    //qDebug()<<"RegExp_yandex";
    int lastPos = 0;
    while ((lastPos = iconsRegExp_yandex.indexIn(response, lastPos)) != -1) {
        QStringList iconData;
        lastPos += iconsRegExp_yandex.matchedLength();

        icons.push_back("https:"+iconsRegExp_yandex.cap(1));
        //qDebug() << iconsRegExp_google.cap(1);
    }

    //qDebug()<<"RegExp_google";
    lastPos = 0;
    while ((lastPos = iconsRegExp_google.indexIn(response, lastPos)) != -1) {
        QStringList iconData;
        lastPos += iconsRegExp_google.matchedLength();

        icons.push_back("https:"+iconsRegExp_google.cap(3));
        //qDebug() << iconsRegExp_google.cap(3);
    }

    emit finished(icons);
    onFinish(icons);
    reply->deleteLater();
}

void Track::onFinish(QStringList ics){
    //qDebug() << "загрузка иконки";
    if (ics.count()>0){
        m_pixmapLoader.load(ics[0]);
    }else{
        //qDebug() << "иконок нет, грузим с гугла";
        if (!last_search.isEmpty() && last_search!=""){
            parse("https://www.google.ru/search?q="+last_search+" cover art&tbm=isch");
            last_search = "";
        }
    }
}

void Track::onPixmap_load(QPixmap pixmap) {
    //qDebug() << "иконка загружена";
    cover = pixmap;
}

void Track::setLocalCover(QPixmap img){
    local_cover = img;
}

void Track::setLocalCover(QImage img){
    local_cover.convertFromImage(img);
}
