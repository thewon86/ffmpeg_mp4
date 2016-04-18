#include "mvideo.h"
#include "ui_mvideo.h"
#include "ffmpegvideo.h"
#include <QByteArray>
#include <QImage>
#include <QPixmap>
#include <QTimer>

mVideo::mVideo(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::mVideo),
    m_fvideo(new FFmpegVideo)
{
    img_frame = new QImage(320, 240, QImage::Format_RGB888);
    ui->setupUi(this);
    ui->video->setPixmap(QPixmap("image.bmp"));
    connect(m_fvideo, SIGNAL(newFrame(QByteArray*)), this, SLOT(refreshVideo(QByteArray*)));
    m_fvideo->init("/dev/video0", "mvideo3.mp4");

    timer = new QTimer;
}

mVideo::~mVideo()
{
    delete ui;
}

void mVideo::refreshVideo(QByteArray *dat)
{
    int i,j;

    for(i = 0; i < 320; i++){
        for(j = 0; j < 240; j++){
            img_frame->setPixel(i, j, qRgb(dat->at(j * 320*3 + i*3),
                                           dat->at(j * 320*3 + i*3 + 1),
                                           dat->at(j * 320*3 + i*3 + 2)));
        }
    }
    delete dat;
    ui->video->setPixmap(QPixmap::fromImage(*img_frame, Qt::AutoColor));

}

