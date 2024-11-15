#include "RollCallAlarmer.h"
#include <QWebEngineProfile>
#include <QWebEngineCookieStore>
#include <QStandardPaths>
#include <QFileDialog>

RollCallAlarmer::RollCallAlarmer(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    net_mgr = new QNetworkAccessManager(this);
    net_mgr->setCookieJar(new QNetworkCookieJar(this));
    timer = new QTimer(this);
    tm_cmd = new QTimer(this);
    media_player = new QMediaPlayer(this);
    audio_out = new QAudioOutput(this);
    default_alarm_path = QString(R"(C:\Windows\Media\)");
    media_player->setAudioOutput(audio_out);
    media_player->setSource(QUrl::fromLocalFile(default_alarm_path + ui.sys_alarm_file->text()));
    

    auto web_profile = ui.login_webpage->page()->profile();
    QString storage_path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/web_profile";
    web_profile->setPersistentStoragePath(storage_path);
    web_profile->setPersistentCookiesPolicy(QWebEngineProfile::ForcePersistentCookies);

    connect(ui.start_listen, &QPushButton::clicked, this, &RollCallAlarmer::do_start_listen);
    connect(ui.stop_listen, &QPushButton::clicked, this, &RollCallAlarmer::do_stop_listen);
    connect(ui.show_loggin_page, &QPushButton::clicked, this, &RollCallAlarmer::do_show_login_page);
    connect(ui.login_webpage->page()->profile()->cookieStore(), &QWebEngineCookieStore::cookieAdded, this, &RollCallAlarmer::do_cookie_added);
    connect(ui.login_webpage, &QWebEngineView::loadFinished, this, &RollCallAlarmer::do_load_finished);
    connect(ui.alarm_file_view_but, &QPushButton::clicked, this, &RollCallAlarmer::do_view_alarm_file);
    connect(ui.is_system_default_alarm, &QCheckBox::checkStateChanged, this, &RollCallAlarmer::do_change_alarm_type);
    connect(ui.test_play_media, &QPushButton::clicked, this, [this] {media_player->play(); });
    connect(ui.stop_play, &QPushButton::clicked, this, [this] {media_player->stop(); });

    connect(this, &RollCallAlarmer::have_roll_call, this, &RollCallAlarmer::do_roll_call_job);
    
    connect(timer, &QTimer::timeout, 
        [this]() {
            int gap = ui.gap_sec->text().toInt();
            if (gap != 0) {
                ui.login_webpage->load(QUrl("https://courses.zju.edu.cn/api/radar/rollcalls?api_version=1.1.0/"));
                timer->start(gap * 1000);
            }
        }
    );
    connect(tm_cmd, &QTimer::timeout,
        [this]() {
            emit have_roll_call(document["rollcalls"].GetArray());
        }
    );
}

RollCallAlarmer::~RollCallAlarmer()
{}

void RollCallAlarmer::do_stop_listen()
{
    timer->stop();
    ui.statusBar->showMessage(tr("监听已终止。"));
}

void RollCallAlarmer::do_cookie_added(const QNetworkCookie& cookie)
{
    net_mgr->cookieJar()->insertCookie(cookie);
    if (cookie.name() == "session") {
        auto domain_name = cookie.domain();
        auto path = cookie.path();
        session_id = cookie.value();
        session_ids.append(session_id);
        ui.tab_pages->setCurrentIndex(1);
        if (!timer->isActive()) {
            ui.statusBar->showMessage(tr("登录完成"));
        }
    }
    else {
        auto domain_name = cookie.domain();
        auto path = cookie.path();
        auto name = cookie.name();
        auto value = cookie.value();
        ui.statusBar->showMessage(tr("尚未检测到登录会话信息"));
    }
}

void RollCallAlarmer::do_load_finished(bool finished)
{
    auto url = ui.login_webpage->url();
    auto str = url.toString().toStdString();
    ui.statusBar->showMessage(tr("开始分析回复"));
    if (url == QUrl("https://courses.zju.edu.cn/api/radar/rollcalls?api_version=1.1.0/")) {
        ui.login_webpage->page()->toPlainText(
            [this](const QString& qcontent) {
                std::string json = qcontent.toStdString();
                if (!document.Parse(json.data()).HasParseError()) {
                    if (document.IsObject() && document.HasMember("rollcalls")) {
                        if (document["rollcalls"].IsArray()) {
                            auto rollcalls = document["rollcalls"].GetArray();
                            if (rollcalls.Size()) {
                                emit have_roll_call(rollcalls);
                                ui.statusBar->showMessage(tr("有签到！！！"));
                            }
                            else {
                                ui.statusBar->showMessage(tr("暂无签到"));
                            }
                        }
                    }
                }
            }
        );
    }
    else {
        return;
    }
}

void RollCallAlarmer::do_show_login_page()
{
    ui.login_webpage->load(QUrl("https://mcourses.zju.edu.cn/"));
}

void RollCallAlarmer::do_roll_call_job(const rapidjson::GenericArray<false, rapidjson::Value>& rollcalls)
{
    do_alarm_roll_call(rollcalls);
    do_command(rollcalls);
    do_read_string(rollcalls);

    int gap = ui.cmd_gap->text().toInt();
    if (gap != -1) {
        timer->start(gap * 1000);
    }
}

void RollCallAlarmer::do_command(const rapidjson::GenericArray<false, rapidjson::Value>& rollcalls)
{
    if (ui.command_set->isChecked()) {
        
    }
}

void RollCallAlarmer::do_read_string(const rapidjson::GenericArray<false, rapidjson::Value>& rollcalls)
{
    if (ui.read_text_set->isChecked()) {

    }
}

void RollCallAlarmer::do_alarm_roll_call(const rapidjson::GenericArray<false, rapidjson::Value>& rollcalls)
{
	if (ui.alarm_set->isChecked()) {
		int times = ui.repeat_times->text().toInt();
        media_player->setLoops(times);
        media_player->play();
	}
}

void RollCallAlarmer::do_view_alarm_file()
{
    QString Qfile_path = QFileDialog::getOpenFileName(this, tr("选择音频文件"), QDir::currentPath());
    ui.alarm_wav_path->setText(Qfile_path);
    media_player->setSource(QUrl::fromLocalFile(ui.alarm_wav_path->text()));
}

void RollCallAlarmer::do_change_alarm_type()
{
    if (ui.is_system_default_alarm->isChecked()) {
#ifdef WIN32
        media_player->setSource(QUrl::fromLocalFile(default_alarm_path + ui.sys_alarm_file->text()));
#endif // WIN32
    }
    else {
        media_player->setSource(QUrl::fromLocalFile(ui.alarm_wav_path->text()));
    }
}

void RollCallAlarmer::do_start_listen() {
    int gap = ui.gap_sec->text().toInt();
    if (gap != 0) {
        ui.login_webpage->load(QUrl("https://courses.zju.edu.cn/api/radar/rollcalls?api_version=1.1.0/"));
        timer->start(gap * 1000);
    }
}
