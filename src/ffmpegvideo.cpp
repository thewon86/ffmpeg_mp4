#include "ffmpegvideo.h"
#include <QByteArray>
#include <QTimer>

int linesize[4]  = {3 * 320, 0, 0, 0};
FFmpegVideo::FFmpegVideo(QObject *parent) :
    QObject(parent)
{
    av_register_all();
    avdevice_register_all();
    avformat_network_init();
    m_videodata = new QByteArray;
    m_timer = new QTimer;

    rgb = new unsigned char[320 * 240 * 3];
    rgb_sws_ctx = 0;
}

void FFmpegVideo::init(const QString &dev, const QString &oFile)
{
    openCamera(dev);
    initOutput(oFile);
    writeVideoHeader();

    connect(m_timer, SIGNAL(timeout()), this, SLOT(readFrame()));
    m_timer->start(100);

}

bool FFmpegVideo::openCamera(const QString &dev)
{
    AVCodec *pCodec;
    int	i;

    pFrame = av_frame_alloc();
    pFrameYUV=av_frame_alloc();
    pFormatCtx = avformat_alloc_context();

    AVDictionary* options = NULL;
//	Set some options
    av_dict_set(&options,"framerate","15",0);
//	Video frame size. The default is to capture the full screen
    av_dict_set(&options,"video_size","320x240",0);
    AVInputFormat *ifmt=av_find_input_format("video4linux2");
    if(avformat_open_input(&pFormatCtx,dev.toAscii().data()/*"/dev/video0"*/,ifmt,&options)!=0){
        printf("Couldn't open input stream.\n");
        return false;
    }

//获取视频流信息
    if(avformat_find_stream_info(pFormatCtx,NULL)<0)
    {
        printf("Couldn't find stream information.\n");
        return false;
    }
//获取视频流索引
    videoindex=-1;
    for(i=0; i<pFormatCtx->nb_streams; i++){
        if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO)
        {
            videoindex=i;
            break;
        }
    }
    if(videoindex==-1)
    {
        printf("Couldn't find a video stream.\n");
        return false;
    }
//获取视频流的分辨率大小
    pCodecCtx=pFormatCtx->streams[videoindex]->codec;
    printf("pCodecCtx->pix_fmt:%d, pCodecCtx->codec_id:%d, pCodecCtx->width:%d, pCodecCtx->height:%d\n", pCodecCtx->pix_fmt, pCodecCtx->codec_id, pCodecCtx->width, pCodecCtx->height);
    pCodec=avcodec_find_decoder(pCodecCtx->codec_id);
    if(pCodec==NULL)
    {
        printf("Codec not found.\n");
        return false;
    }
    if(avcodec_open2(pCodecCtx, pCodec,NULL)<0)
    {
        printf("Could not open codec.\n");
        return false;
    }

    return true;
}

void FFmpegVideo::closeCamera()
{
    sws_freeContext(rgb_sws_ctx);
    delete rgb;
    avcodec_close(pCodecCtx);
    avformat_close_input(&pFormatCtx);
    av_free(pFrame);

}

bool FFmpegVideo::initOutput(const QString &oFile)
{
    AVOutputFormat* fmtout;
    AVCodec* pCodecout;

    const char* out_file = oFile.toAscii().data();

    pFormatCtxout = avformat_alloc_context();
    //Guess Format
    fmtout = av_guess_format(NULL, out_file, NULL);
    pFormatCtxout->oformat = fmtout;
//Open output URL
    if (avio_open(&pFormatCtxout->pb,out_file, AVIO_FLAG_READ_WRITE) < 0){
        printf("Failed to open output file! \n");
        return false;
    }
    video_st = avformat_new_stream(pFormatCtxout, 0);
    video_st->time_base.num = 1;
    video_st->time_base.den = 15;

    if (video_st==NULL){
        return false;
    }

//Param that must set
    pCodecCtxout = video_st->codec;
    pCodecCtxout->codec_id = fmtout->video_codec;
    pCodecCtxout->codec_type = AVMEDIA_TYPE_VIDEO;
    pCodecCtxout->pix_fmt = AV_PIX_FMT_YUV420P;
    pCodecCtxout->width = pCodecCtx->width;
    pCodecCtxout->height = pCodecCtx->height;
    pCodecCtxout->time_base.num = video_st->time_base.num;
    pCodecCtxout->time_base.den = video_st->time_base.den;
    pCodecCtxout->bit_rate = 1800000;
    pCodecCtxout->gop_size=300;
    //H264
    pCodecCtxout->qmin = 10;
    pCodecCtxout->qmax = 51;
    //Optional Param
    pCodecCtxout->max_b_frames=3;

    // Set Option
    AVDictionary *param = 0;
    //H.264
    if(pCodecCtxout->codec_id == AV_CODEC_ID_H264) {
        av_dict_set(&param, "preset", "slow", 0);
        av_dict_set(&param, "tune", "zerolatency", 0);
    }
    //H.265
    if(pCodecCtxout->codec_id == AV_CODEC_ID_H265){
        av_dict_set(&param, "preset", "ultrafast", 0);
        av_dict_set(&param, "tune", "zero-latency", 0);
    }

//Show some Information
    av_dump_format(pFormatCtxout, 0, out_file, 1);

    printf("pCodecCtxout->codec_id:%d\n", pCodecCtxout->codec_id);
    pCodecout = avcodec_find_encoder(pCodecCtxout->codec_id);
    if (!pCodecout){
        printf("Can not find encoder! \n");
        return false;
    }
    if (avcodec_open2(pCodecCtxout, pCodecout,&param) < 0){
        printf("Failed to open encoder! \n");
        return false;
    }
    pFrameYUV->width = pCodecCtx->width;
    pFrameYUV->height = pCodecCtx->height;
    pFrameYUV->format = pCodecCtxout->pix_fmt;

    m_write_mp4 = true;

    return true;
// end open output mp4 file
}

bool FFmpegVideo::writeVideoHeader()
{
    int picture_size;

    picture_size = av_image_get_buffer_size(pCodecCtxout->pix_fmt, pCodecCtxout->width, pCodecCtxout->height, 1);
    uint8_t *out_buffer;
    out_buffer= (uint8_t *)malloc(picture_size);
    av_image_fill_arrays(pFrameYUV->data, pFrameYUV->linesize, out_buffer, pCodecCtxout->pix_fmt, pCodecCtxout->width, pCodecCtxout->height, 1);

    img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height,
                                     pCodecCtx->pix_fmt,
                                     pCodecCtxout->width,
                                     pCodecCtxout->height,
                                     pCodecCtxout->pix_fmt, SWS_BICUBIC, NULL, NULL, NULL);

    rgb_sws_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height,
                                 pCodecCtx->pix_fmt,
                                 320,
                                 240,
                                 AV_PIX_FMT_RGB24,
                                 SWS_BICUBIC, 0, 0, 0);
    //Write File Header
    avformat_write_header(pFormatCtxout,NULL);

    av_new_packet(&pkt_write,picture_size);

    return true;
}

void FFmpegVideo::readFrame()
{
    AVPacket *packet=(AVPacket *)av_malloc(sizeof(AVPacket));
    int ret, got_picture;
    static int framecnt = 0;

    if(av_read_frame(pFormatCtx, packet)>=0){
        if(packet->stream_index==videoindex){
            ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);
            if(ret < 0){
                printf("Decode Error.\n");
                return;
            }

            if(got_picture){
                if(rgb_sws_ctx != NULL){
                    sws_scale(rgb_sws_ctx, (const uint8_t* const*)pFrame->data, pFrame->linesize, 0, 240,
                              (uint8_t**)&rgb, linesize);
                    m_videodata = new QByteArray;
                    *m_videodata = QByteArray::fromRawData((const char*)rgb, 320 * 240 * 3);

                    emit newFrame(m_videodata);
                }

                if(framecnt < 0) return;
                if (framecnt < 500){
                sws_scale(img_convert_ctx, (const uint8_t* const*)pFrame->data, pFrame->linesize, 0, pCodecCtxout->height, pFrameYUV->data, pFrameYUV->linesize);
                pFrameYUV->pts=framecnt*0.030;
                pFrameYUV->pts=0x0400*framecnt*3;
                framecnt++;

                //Encode
                int ret = avcodec_encode_video2(pCodecCtxout, &pkt_write,pFrameYUV, &got_picture);
                if(ret < 0){
                    printf("Failed to encode! \n");
                    return;
                }
                if (got_picture==1){
                    pkt_write.stream_index = video_st->index;
                    ret = av_write_frame(pFormatCtxout, &pkt_write);
                    av_packet_unref(&pkt_write);
                }
                }
                else if(m_write_mp4){
                    writeVideoTail();
                }
            }
        }
        av_packet_unref(packet);
    }
}

bool FFmpegVideo::writeVideoTail()
{
    int ret;
    //Flush Encoder
    ret = flush_encoder(pFormatCtxout,0);
    if (ret < 0) {
        printf("Flushing encoder failed\n");
        return false;
    }
    //Write file trailer
    av_write_trailer(pFormatCtxout);

    //Clean
    if (video_st){
        avcodec_close(video_st->codec);
        av_free(pFrameYUV);
    }
    avio_close(pFormatCtxout->pb);
    avformat_free_context(pFormatCtxout);

    sws_freeContext(img_convert_ctx);

    m_write_mp4 = false;

    return true;
}

int FFmpegVideo::flush_encoder(AVFormatContext *fmt_ctx,unsigned int stream_index){
    int ret;
    int got_frame;
    AVPacket enc_pkt;
    if (!(fmt_ctx->streams[stream_index]->codec->codec->capabilities &
        CODEC_CAP_DELAY))
        return 0;
    while (1) {
        enc_pkt.data = NULL;
        enc_pkt.size = 0;
        av_init_packet(&enc_pkt);
        ret = avcodec_encode_video2 (fmt_ctx->streams[stream_index]->codec, &enc_pkt,
            NULL, &got_frame);
        av_frame_free(NULL);
        if (ret < 0)
            break;
        if (!got_frame){
            ret=0;
            break;
        }
        printf("Flush Encoder: Succeed to encode 1 frame!\tsize:%5d\n",enc_pkt.size);
        /* mux encoded frame */
        ret = av_write_frame(fmt_ctx, &enc_pkt);
        if (ret < 0)
            break;
    }
    return ret;
}
