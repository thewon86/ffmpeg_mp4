#ifndef BASEDIALOG_H
#define BASEDIALOG_H

#include <QDialog>

class mVideo;

namespace Ui {
class BaseDialog;
}

class BaseDialog : public QDialog
{
    Q_OBJECT

public:
    explicit BaseDialog(QWidget *parent = 0);
    ~BaseDialog();

private slots:
    void refleshtime();

private:
    Ui::BaseDialog *ui;
    QTimer *m_timer_dt;
    mVideo *m_video;
};

#endif // BASEDIALOG_H
