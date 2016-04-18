#ifndef FFMPEGVIDEO_H
#define FFMPEGVIDEO_H

#include <QObject>

#ifdef __cplusplus
extern "C"
{
#endif
#define __STDC_CONSTANT_MACROS
#ifdef _STDINT_H
#undef _STDINT_H
#endif
#include <stdint.h>

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavdevice/avdevice.h>
#include <libavutil/imgutils.h>

#ifdef __cplusplus
}
#endif

class QByteArray;
class QTimer;

class FFmpegVideo : public QObject
{
    Q_OBJECT
public:
    explicit FFmpegVideo(QObject *parent = 0);
    void init(const QString &dev, const QString &oFile);

//private:
    bool openCamera(const QString &dev);
    void closeCamera();
    bool initOutput(const QString &oFile);
    bool writeVideoHeader();
    bool writeVideoTail();
    int flush_encoder(AVFormatContext *fmt_ctx, unsigned int stream_index);

signals:
    void newFrame(QByteArray *dat);

private slots:
    void readFrame();

private:
    AVFormatContext	*pFormatCtx;
    AVFormatContext *pFormatCtxout;
    AVCodecContext *pCodecCtx;
    AVCodecContext *pCodecCtxout;
    AVFrame	*pFrame,*pFrameYUV;
    struct SwsContext *img_convert_ctx;
    AVPacket pkt_write;
    AVStream* video_st;

    int	videoindex;
    QByteArray *m_videodata;
    QTimer *m_timer;
    bool m_write_mp4;

    unsigned char *rgb;
    struct SwsContext *rgb_sws_ctx;
};

#endif // FFMPEGVIDEO_H
