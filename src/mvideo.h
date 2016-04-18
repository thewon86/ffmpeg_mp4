#ifndef MVIDEO_H
#define MVIDEO_H

#include <QDialog>

namespace Ui {
class mVideo;
}

class QImage;
class FFmpegVideo;
class QThread;

class mVideo : public QDialog
{
    Q_OBJECT

public:
    explicit mVideo(QWidget *parent = 0);
    ~mVideo();
    void initCamera();

protected:
Q_SIGNALS:

private slots:
    void refreshVideo(QByteArray *dat);

private:
    Ui::mVideo *ui;
    FFmpegVideo *m_fvideo;
    QImage *img_frame;
    QTimer *timer;
};

#endif // MVIDEO_H
