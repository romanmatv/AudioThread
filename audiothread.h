#ifndef AUDIOTHREAD_H
#define AUDIOTHREAD_H

#include "audiothread_global.h"

#include <QThread>
#include <QTimer>
#include "bass.h"
#include "bass_fx.h"
#include "track.h"
#include "tags.h"

// HLS definitions (copied from BASSHLS.H)
#define BASS_SYNC_HLS_SEGMENT	0x10300
#define BASS_TAG_HLS_EXTINF		0x14000
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#define min(a,b)            (((a) < (b)) ? (a) : (b))

void __stdcall syncFunc(HSYNC handle, DWORD channel, DWORD data, void *user);
void __stdcall syncMeta(HSYNC handle, DWORD channel, DWORD data, void *user);

class AUDIOTHREADSHARED_EXPORT AudioThread : public QThread
{
    Q_OBJECT
public:
    explicit AudioThread(QObject *parent = 0, int freq=0);
    ~AudioThread();
    void free();
    bool playing;
    void run();
    unsigned long getChanel(){return chan;}
    void setFreq(int);
    float eqs[18] = {31.0f, 63.0f, 87.0f, 125.0f, 175.0f, 250.0f, 350.0f, 500.0f, 700.0f, 1000.0f, 1400.0f, 2000.0f, 2800.0f, 4000.0f, 5600.0f, 8000.0f, 11200.0f, 16000.0f,};
private:
    unsigned long chan;
    float lLeft, lRight;
    QTimer *tmr;
    QList<DWORD> freqs = {44100, 48000, 96000, 192000};
    HFX fxEQ, fxCompressor;			// dsp fx handles
    void UpdateFX(int b, float v);
    float chan_levels_old[2];
    int dblevel_1, dblevel_2;

    //Arduino функции
    template <typename T>
    static T map(T x, T in_min, T in_max, T out_min, T out_max)
    {
      return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    }
    //Найденная на просторах функция (работает лучше)
    template <typename T>
    T convert(T value, T From1, T From2, T To1, T To2)
    {
          return (value-From1)/(From2-From1)*(To2-To1)+To1;
    }
    template <typename T>
    static T constrain(T x, T a, T b){
        if (x<a) return a;
        if (x>b) return b;
        return x;
    }
signals:
    void startOfPlayback(double total);
    void endOfPlayback();
    void pauseOfPlayback();
    void curPos(double position, double total);
    void metaChanged(Track*);
    void leftLevelChanged(int);
    void rightLevelChanged(int);
public slots:
    //Plaing
    void play(QString filepath);
    void play(Track*);
    void playOrPause(QString filepath);
    void playOrPause(Track*t=nullptr);
    bool isPlaying();
    bool isPausing();
    void pause();
    void resume();
    void stop();
    void setVolume(int vol);
    void changePosition(int position);
    //Information
    void getFFT(float *, unsigned long length = BASS_DATA_FFT2048);
    void getLevels(float *, float length = 0.02, DWORD flags = BASS_LEVEL_STEREO);
    BASS_CHANNELINFO getChanelInfo();
    QString getDuration(QString path);
    QString formattedTime(double t);
    static Track *getMeta(unsigned long chan);
    static Track *getMeta(QString);
    Track *getMeta();
    float getCPU(){return BASS_GetCPU();}
    //DSP
    void setDSP_EQ(QSettings *sets = nullptr);
    void unsetDSP_EQ();
    void updateDSP_EQ(QSettings *sets = nullptr);
    void setCompressor(float fGain, float fThreshold, float fRatio, float fAttack, float fRelease);
    void unsetCompressor();
private slots:
    void signalUpdate();

};

#endif // AUDIOTHREAD_H
