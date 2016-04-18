#include "basedialog.h"
#include "ui_basedialog.h"
#include <QDateTime>
#include <QTimer>
#include <QDebug>
#include "mvideo.h"


QString getCurrentTime()
{
    return QDateTime::currentDateTime().toString(QString("yyyy-MM-dd hh:mm:ss"));
}

BaseDialog::BaseDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BaseDialog),
    m_video(new mVideo(this))
{
    ui->setupUi(this);
    ui->base_stackedWidget->addWidget(m_video);
    ui->base_stackedWidget->setCurrentIndex(0);
    ui->cur_datetime->setText(getCurrentTime());
    m_timer_dt = new QTimer;
    m_timer_dt->start(500);

    connect(m_timer_dt, SIGNAL(timeout()), this, SLOT(refleshtime()));

    QFont font("simsun", 14, 50);
//    font.setBold(true);
    ui->cur_datetime->setFont(font);
}

BaseDialog::~BaseDialog()
{
    delete ui;
}

void BaseDialog::refleshtime()
{
    ui->cur_datetime->setText(getCurrentTime());
}

