#pragma once

#include <QUrl>
#include <QTimer>
#include <QtWidgets/QMainWindow>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QNetworkCookieJar>
#include <QNetworkCookie>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <rapidjson/document.h>
#include "ui_RollCallAlarmer.h"

class RollCallAlarmer : public QMainWindow
{
    Q_OBJECT

public:
    RollCallAlarmer(QWidget *parent = nullptr);
    ~RollCallAlarmer();

signals:
    void have_roll_call(const rapidjson::GenericArray<false, rapidjson::Value>& rollcalls);

private slots:
    void do_start_listen();
    void do_stop_listen();
    void do_cookie_added(const QNetworkCookie &cookie);
    void do_load_finished(bool);
    void do_show_login_page();
    void do_roll_call_job(const rapidjson::GenericArray<false, rapidjson::Value>& rollcalls);
    void do_view_alarm_file();
    void do_change_alarm_type();
private:
    void do_command(const rapidjson::GenericArray<false, rapidjson::Value>& rollcalls);
    void do_read_string(const rapidjson::GenericArray<false, rapidjson::Value>& rollcalls);
    void do_alarm_roll_call(const rapidjson::GenericArray<false, rapidjson::Value>& rollcalls);
    QString session_id;
    QString command_str;
    QString default_alarm_path;
    QList<QString> session_ids;
    Ui::RollCallAlarmerClass ui;
    QNetworkAccessManager* net_mgr;
    QMediaPlayer* media_player;
    QAudioOutput* audio_out;
    QTimer* timer, *tm_cmd;
    rapidjson::Document document;
};
