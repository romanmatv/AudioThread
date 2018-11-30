#include "audiothread.h"

bool endOfMusic;
Track *curTrack;
bool changedMeta;

void __stdcall syncFunc(HSYNC handle, DWORD channel, DWORD data, void *user)
{
    Q_UNUSED(data);
    Q_UNUSED(user);
    BASS_ChannelRemoveSync(channel, handle);
    BASS_ChannelStop(channel);
    endOfMusic = true;
}

void __stdcall syncMeta(HSYNC handle, DWORD channel, DWORD data, void *user){
    Q_UNUSED(handle);
    Q_UNUSED(data);
    Q_UNUSED(user);
    curTrack = AudioThread::getMeta(channel);
    changedMeta = true;
}

void __stdcall StatusProc(const void *buffer, DWORD length, void *user)
{
    Q_UNUSED(buffer);
    Q_UNUSED(length);
    Q_UNUSED(user);
}

// update dsp eq
void AudioThread::UpdateFX(int b, float gain)
{
    BASS_BFX_PEAKEQ eq;

    eq.lBand = b;	// get values of the selected band
    BASS_FXGetParameters(fxEQ, &eq);
        eq.fGain = (float)(10 - gain);
    BASS_FXSetParameters(fxEQ, &eq);
}

//0.0f,  2.5f,       0.0f, 125.0f,    	 1000.0f, 	  8000.0f
void AudioThread::setDSP_EQ(QSettings *sets)
{
    float fGain = 0.0f;
    float fBandwidth = 2.5f;
    float fQ = 0.0f;

    float gain = 0.0;

    BASS_BFX_PEAKEQ eq;
    //Частоты для изменений
    //31; 63; 87; 125; 175; 250; 350; 500; 700; 1k; 1,4k; 2k; 2,8k; 4k; 5,6k; 8k; 11,2k; 16k;

    // set peaking equalizer effect with no bands
    fxEQ = BASS_ChannelSetFX(chan, BASS_FX_BFX_PEAKEQ,0);

    eq.fGain = fGain;
    eq.fQ = fQ;
    eq.fBandwidth = fBandwidth;
    eq.lChannel = BASS_BFX_CHANALL;

    for (int i=0;i<18;i++){
        // create n band for bass
        //if (sets){
            //gain = sets->value("eq/bands/"+QString::number(i), 0).toFloat();
        //}else gain = 0;
        eq.lBand = i;
        eq.fCenter = eqs[i];
        //eq.fGain = (float)(10 - gain);
        BASS_FXSetParameters(fxEQ, &eq);
    }

    updateDSP_EQ(sets);
}

void AudioThread::updateDSP_EQ(QSettings *sets){
    //qDebug() << "update DSP EQ";
    int gain = 0;
    for (int i=0;i<18;i++){
        if (sets){
            gain = sets->value("eq/bands/"+QString::number(i), 0).toInt();
        }else gain = 0;

        UpdateFX(i, gain);
    }
}

void AudioThread::unsetDSP_EQ(){
    BASS_ChannelRemoveFX(chan, fxEQ);
}

void AudioThread::setCompressor(float fGain, float fThreshold, float fRatio, float fAttack, float fRelease){
    BASS_BFX_COMPRESSOR2 cmpr;

    cmpr.fGain = fGain;
    cmpr.fThreshold = fThreshold;
    cmpr.fRatio = fRatio;
    cmpr.fAttack = fAttack;
    cmpr.fRelease = fRelease;

    fxCompressor = BASS_ChannelSetFX(chan, BASS_FX_BFX_COMPRESSOR2,0);
    BASS_FXSetParameters(fxCompressor, &cmpr);
}

void AudioThread::unsetCompressor(){
    BASS_ChannelRemoveFX(chan, fxCompressor);
}

AudioThread::AudioThread(QObject *parent, int freq) :
    QThread(parent)
{
    DWORD _freq;
    if (freq>=0 && freq<4){
        _freq = freqs[freq];
    }else{
        _freq = freq;
    }
    BASS_Free();

    if (!BASS_Init(-1, _freq, 0, nullptr, nullptr)){
        //qDebug() << "Cannot initialize device";
    }else{
        BASS_SetConfig(BASS_CONFIG_NET_PLAYLIST, 1); // enable playlist processing
        BASS_SetConfig(BASS_CONFIG_NET_PREBUF_WAIT, 0); // disable BASS_StreamCreateURL pre-buffering
        BASS_SetConfig(BASS_CONFIG_FLOATDSP, true);

        BASS_PluginLoad("bass_aac.dll",0); // load BASS_AAC (if present) for AAC support on older Windows
        BASS_PluginLoad("basshls.dll",0); // load BASSHLS (if present) for HLS support
        BASS_PluginLoad("bass_ape.dll",0);
        BASS_PluginLoad("bass_ac3.dll",0);
        BASS_PluginLoad("bass_mpc.dll",0);
        BASS_PluginLoad("bassalac.dll",0);
        BASS_PluginLoad("bassflac.dll",0);
        BASS_PluginLoad("bassopus.dll",0);
        BASS_PluginLoad("basswasapi.dll",0);
        BASS_PluginLoad("basswma.dll",0);
        BASS_PluginLoad("basswv.dll",0);
        BASS_PluginLoad("tags.dll",0);
    }
    tmr = new QTimer(this);
    tmr->setInterval(50);
    connect(tmr, SIGNAL(timeout()), this, SLOT(signalUpdate()));
    endOfMusic = true;
    changedMeta = false;
}

void AudioThread::setFreq(int freq){
    tmr->stop();
    BASS_Free();

    DWORD _freq;
    if (freq>=0 && freq<4){
        _freq = freqs[freq];
    }else{
        _freq = freq;
    }

    if (!BASS_Init(-1, _freq, 0, nullptr, nullptr)){
        //qDebug() << "Cannot initialize device";
    }else{
        BASS_SetConfig(BASS_CONFIG_NET_PLAYLIST,1); // enable playlist processing
        BASS_SetConfig(BASS_CONFIG_NET_PREBUF_WAIT,0); // disable BASS_StreamCreateURL pre-buffering

        BASS_PluginLoad("bass_aac.dll",0); // load BASS_AAC (if present) for AAC support on older Windows
        BASS_PluginLoad("basshls.dll",0); // load BASSHLS (if present) for HLS support
        BASS_PluginLoad("bass_ape.dll",0);
        BASS_PluginLoad("bass_fx.dll",0);
        BASS_PluginLoad("bass_ac3.dll",0);
        BASS_PluginLoad("bass_mpc.dll",0);
        BASS_PluginLoad("bassalac.dll",0);
        BASS_PluginLoad("bassflac.dll",0);
        BASS_PluginLoad("bassopus.dll",0);
        BASS_PluginLoad("basswasapi.dll",0);
        BASS_PluginLoad("basswma.dll",0);
        BASS_PluginLoad("basswv.dll",0);
        BASS_PluginLoad("tags.dll",0);
    }
}

AudioThread::~AudioThread(){
    tmr->stop();
    delete tmr;
    BASS_Free();
}

void AudioThread::free(){
    tmr->stop();
    BASS_Free();
}

void AudioThread::play(Track *track){
    if (track){
        curTrack = track;
        play(track->path.toString());
    }
}

void AudioThread::play(QString filename)
{
    BASS_ChannelStop(chan);

    if (!(chan = BASS_StreamCreateURL(filename.toLocal8Bit(),0,BASS_STREAM_BLOCK|BASS_STREAM_STATUS|BASS_STREAM_AUTOFREE,&StatusProc,nullptr))){
        QUrl url = QUrl::fromUserInput(filename);
        if (!(chan = BASS_StreamCreateFile(false, (url.toLocalFile().toLocal8Bit()).data(), 0, 0, 0))){
            return;
        }
    }

    endOfMusic = false;
    BASS_ChannelPlay(chan, true);
    emit startOfPlayback(BASS_ChannelBytes2Seconds(chan, BASS_ChannelGetLength(chan, BASS_POS_BYTE)));
    playing = true;
    tmr->start();
    BASS_ChannelSetSync(chan,BASS_SYNC_META,0,syncMeta, nullptr); // Shoutcast
    BASS_ChannelSetSync(chan,BASS_SYNC_OGG_CHANGE,0,syncMeta, nullptr); // Icecast/OGG
    BASS_ChannelSetSync(chan,BASS_SYNC_HLS_SEGMENT,0,syncMeta, nullptr); // HLS
    BASS_ChannelSetSync(chan, BASS_SYNC_END, 0, &syncFunc, nullptr); //End of play
    curTrack = AudioThread::getMeta(chan);
    changedMeta = true;
}

void AudioThread::pause()
{
    BASS_ChannelPause(chan);
    playing = false;
    emit pauseOfPlayback();
}

void AudioThread::resume()
{
    if (!BASS_ChannelPlay(chan, false)){
        //qDebug() << "Error resuming";
    }else
    {
        tmr->start();
        emit startOfPlayback(BASS_ChannelBytes2Seconds(chan, BASS_ChannelGetLength(chan, BASS_POS_BYTE)));
        playing = true;
    }
}

void AudioThread::playOrPause(QString filename) {
    if (BASS_ChannelIsActive(chan)==1) {
        //Playing
        pause();
    }
    else {
        if (BASS_ChannelIsActive(chan)==3) {//paused
            resume();
        }
        else {
            play(filename);
        }
    }
}

void AudioThread::playOrPause(Track*t){
    curTrack = t;
    playOrPause(t->path.toString());
}

bool AudioThread::isPlaying(){
    if (BASS_ChannelIsActive(chan)==1) return true; else return false;
}

bool AudioThread::isPausing(){
    if (BASS_ChannelIsActive(chan)==3) return true; else return false;
}

void AudioThread::stop()
{
    BASS_ChannelStop(chan);
    playing = false;
}

void AudioThread::signalUpdate()
{
    if (endOfMusic == false)
    {
        playing = true;
        emit curPos(BASS_ChannelBytes2Seconds(chan, BASS_ChannelGetPosition(chan, BASS_POS_BYTE)),
                    BASS_ChannelBytes2Seconds(chan, BASS_ChannelGetLength(chan, BASS_POS_BYTE)));
    }
    else
    {
        if (playing){
            emit endOfPlayback();
            playing = false;
        }
    }

    if (changedMeta){
        if (curTrack){
            emit metaChanged(curTrack);
            changedMeta = false;
        }
    }

    //Получаем лэвэлы каналов
    float levels[2];
    BASS_ChannelGetLevelEx(chan, levels, 0.02, BASS_LEVEL_STEREO);
    bool to_minus = false;
    if (chan_levels_old[0]==levels[0] && chan_levels_old[1]==levels[1]){
        to_minus = true;
    }

    if (levels[0]<0 || levels[0]>1 || levels[1]<0 || levels[1]>1){
        to_minus = true;
    }

    if (to_minus){
        if (dblevel_1>0 || dblevel_2>0){
            if (dblevel_1>0){
                dblevel_1 = dblevel_1 / 1.2;
                dblevel_1 = constrain(dblevel_1, 0, 1000);
                emit leftLevelChanged(dblevel_1);
            }

            if (dblevel_2>0){
                dblevel_2 = dblevel_2 / 1.2;
                dblevel_2 = constrain(dblevel_2, 0, 1000);
                emit rightLevelChanged(dblevel_2);
            }
        }
    }else{
        chan_levels_old[0] = levels[0];
        chan_levels_old[1] = levels[1];

        dblevel_1 = convert((double)levels[0], 0.0, 1.0, 0.0, 1000.0);
        dblevel_2 = convert((double)levels[1], 0.0, 1.0, 0.0, 1000.0);

        dblevel_1 = constrain(dblevel_1, 0, 1000);
        dblevel_2 = constrain(dblevel_2, 0, 1000);

        emit leftLevelChanged(dblevel_1);
        emit rightLevelChanged(dblevel_2);
    }
}

void AudioThread::changePosition(int position)
{
    BASS_ChannelSetPosition(chan, BASS_ChannelSeconds2Bytes(chan, position), BASS_POS_BYTE);
}

QString AudioThread::getDuration(QString path) {
    DWORD lcl_chan;
    lcl_chan = BASS_StreamCreateFile(false,path.toLocal8Bit(), 0, 0, 0);
    QString tm = formattedTime(BASS_ChannelBytes2Seconds(lcl_chan, BASS_ChannelGetLength(lcl_chan, BASS_POS_BYTE)));
    BASS_StreamFree(lcl_chan);
    return tm;
}

QString AudioThread::formattedTime(double t) {
    int position = (int) (0.5+t);
    int min = (int) position / 60 % 60;
    int sec = (int) position % 60;
    QString s;
    s.sprintf("%02d:%02d",min,sec);
    return s;
}

void AudioThread::run()
{
    while (1);
}

void AudioThread::getFFT(float *fft, unsigned long length){
    BASS_ChannelGetData(chan, fft, length);
}

BASS_CHANNELINFO AudioThread::getChanelInfo(){
    BASS_CHANNELINFO inf;
    BASS_ChannelGetInfo(chan, &inf);
    return inf;
}

void AudioThread::getLevels(float *levels, float length, DWORD flags){
    BASS_ChannelGetLevelEx(chan, levels, length, flags);
}

void AudioThread::setVolume(int vol){
    BASS_SetConfig(BASS_CONFIG_GVOL_STREAM, (vol*100));
}

Track *AudioThread::getMeta(QString file){
    DWORD lcl_chan;
    lcl_chan = BASS_StreamCreateFile(false,file.toLocal8Bit(), 0, 0, 0);
    Track *tr = getMeta(lcl_chan);
    BASS_StreamFree(lcl_chan);
    return tr;
}

Track *AudioThread::getMeta(){
    return getMeta(chan);
}

Track *AudioThread::getMeta(unsigned long chan){
    Track *track = new Track();

    track->title = QString::fromLocal8Bit(TAGS_Read(chan, "%TITL" ));
    track->artist = QString::fromLocal8Bit(TAGS_Read(chan, "%ARTI" ));
    track->album = QString::fromLocal8Bit(TAGS_Read(chan, "%ALBM" ));
    track->genre = QString::fromLocal8Bit(TAGS_Read(chan, "%GNRE" ));
    track->date = QString::fromLocal8Bit(TAGS_Read(chan, "%YEAR" ));

    const char *icy=BASS_ChannelGetTags(chan,BASS_TAG_ICY);
    if (!icy) icy=BASS_ChannelGetTags(chan,BASS_TAG_HTTP); // no ICY tags, try HTTP
    if (icy) {
        //qDebug() << "icy: " << icy;
        for (;*icy;icy+=strlen(icy)+1) {
            if (!strnicmp(icy,"icy-name:",9)){
                if (track->radioName.isEmpty()){
                    track->radioName = QString(icy+9);
                }
            }
        }
    }

    const char *meta = BASS_ChannelGetTags(chan,BASS_TAG_META);
    if (meta) { // got Shoutcast metadata
        //qDebug() << "Shoutcast: "<<meta;
        const char *p=strstr(meta,"StreamTitle='"); // locate the title
        if (p) {
            const char *p2=strstr(p,"';"); // locate the end of it
            if (p2) {
                char *t=strdup(p+13);
                t[p2-(p+13)]=0;
                track->title = t;
                //delete t;
            }
            //delete p2;
        }
        //delete p;
    } else {
        meta = BASS_ChannelGetTags(chan,BASS_TAG_OGG);
        if (meta) { // got Icecast/OGG tags
            //qDebug() << "OGG: "<<meta;
            const char *artist=NULL,*title=NULL,*p=meta;
            for (;*p;p+=strlen(p)+1) {
                if (!strnicmp(p,"artist=",7)) // found the artist
                    artist=p+7;
                if (!strnicmp(p,"title=",6)) // found the title
                    title=p+6;
            }
            if (title) {
                if (artist) {
                    track->title = title;
                    track->artist = artist;
                } else {
                    track->title = title;
                }
            }
        } else {
            meta=BASS_ChannelGetTags(chan,BASS_TAG_HLS_EXTINF);
            //qDebug() << "HLS: "<<meta;
            if (meta) { // got HLS segment info
                const char *p=strchr(meta,',');
                if (p)
                    track->title = QString::fromLocal8Bit(p+1);
            }
        }
    }

    if (!track->artist.isEmpty() && !track->album.isEmpty()){
        track->search_cover(track->artist+" "+track->album);
    }else if (!track->title.isEmpty()){
        track->search_cover((!track->artist.isEmpty()?track->artist+" ":"")+track->title);
    }else if (!track->radioName.isEmpty()){
        track->search_cover(track->radioName);
    }

    return track;
}
