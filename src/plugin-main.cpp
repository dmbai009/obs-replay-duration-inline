#include <obs-module.h>
#include <obs-frontend-api.h>
#include <util/config-file.h>
#include <QDockWidget>
#include <QHBoxLayout>
#include <QIntValidator>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QPointer>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSizePolicy>
#include <QTimer>
#include <QWidget>
#include <algorithm>
#include <cstdint>
#include <cstring>
OBS_DECLARE_MODULE()
const char *obs_module_description(void) { return "Inline replay buffer duration control"; }
namespace {
class Inline : public QObject {
    QTimer timer;
    QPointer<QWidget> holder;
    QPointer<QLineEdit> field;
    QPointer<QPushButton> replayButton;
    QPointer<QPushButton> saveButton;
    QPointer<QHBoxLayout> row;
    QSizePolicy originalReplayPolicy;
    QSizePolicy originalSavePolicy;
    int originalReplayStretch = 0;
    int originalSaveStretch = 0;
    bool transitioning = false;
    static config_t *config() { return obs_frontend_get_profile_config(); }
    static const char *section(config_t *cfg) {
        const char *mode = config_get_string(cfg, "Output", "Mode");
        return mode && strcmp(mode, "Advanced") == 0 ? "AdvOut" : "SimpleOutput";
    }
    static int seconds() {
        auto *c = config();
        return c ? (int)std::clamp<int64_t>(config_get_int(c, section(c), "RecRBTime"), 5, 21600) : 20;
    }
    bool locked() const { return transitioning || obs_frontend_replay_buffer_active(); }
    void attach() {
        if (field) return;
        auto *main = static_cast<QMainWindow *>(obs_frontend_get_main_window());
        if (!main) return;
        auto *dock = main->findChild<QDockWidget *>("controlsDock");
        if (!dock || !dock->widget()) return;
        auto *root = dock->widget();
        auto *layout = root->findChild<QHBoxLayout *>("replayBufferLayout");
        auto *button = root->findChild<QPushButton *>("replayBufferButton");
        auto *save = root->findChild<QPushButton *>("saveReplayButton");
        if (!layout || !button || !save) return;
        auto *widget = new QWidget(root);
        widget->setObjectName("replayDurationInline");
        // Keep the seconds input compact; distribute ALL spare width to the replay button.
        widget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        auto *line = new QHBoxLayout(widget);
        line->setContentsMargins(0, 0, 0, 0);
        line->setSpacing(2);
        auto *edit = new QLineEdit(widget);
        edit->setObjectName("replayDurationInlineSeconds");
        edit->setFixedWidth(54);
        edit->setFixedHeight(button->sizeHint().height());
        edit->setMaxLength(5);
        edit->setAlignment(Qt::AlignCenter);
        edit->setValidator(new QIntValidator(5, 21600, edit));
        edit->setAccessibleName("Replay Buffer duration in seconds");
        edit->setToolTip("Set maximum replay length in seconds");
        edit->setText(QString::number(seconds()));
        line->addWidget(edit);
        line->addWidget(new QLabel("s", widget));

        // OBS controls: [replay button (expanding)] [seconds] [save replay (fixed)].
        // The native saveReplayButton is initially hidden and becomes visible
        // while buffering. Its own size should never consume the spare width.
        originalReplayPolicy = button->sizePolicy();
        originalSavePolicy = save->sizePolicy();
        originalReplayStretch = layout->stretch(0);
        originalSaveStretch = layout->stretch(1);
        button->setSizePolicy(QSizePolicy::Expanding, originalReplayPolicy.verticalPolicy());
        save->setSizePolicy(QSizePolicy::Fixed, originalSavePolicy.verticalPolicy());
        layout->insertWidget(1, widget);
        layout->setStretch(0, 1);
        layout->setStretch(1, 0);
        layout->setStretch(2, 0);

        holder = widget;
        field = edit;
        replayButton = button;
        saveButton = save;
        row = layout;
        connect(edit, &QLineEdit::editingFinished, this, [this] { save(); });
        blog(LOG_INFO, "[replay-duration-inline] Attached duration field");
    }
    void save() {
        if (!field) return;
        if (locked()) { field->setText(QString::number(seconds())); return; }
        bool ok;
        int value = field->text().toInt(&ok);
        if (!ok || value < 5 || value > 21600) { field->setText(QString::number(seconds())); return; }
        auto *cfg = config();
        if (!cfg) return;
        const char *group = section(cfg);
        auto old = config_get_int(cfg, group, "RecRBTime");
        if (old == value) return;
        config_set_int(cfg, group, "RecRBTime", value);
        if (config_save_safe(cfg, "tmp", nullptr) != 0) {
            config_set_int(cfg, group, "RecRBTime", old);
            field->setText(QString::number(seconds()));
            blog(LOG_WARNING, "[replay-duration-inline] Unable to save duration");
            return;
        }
        obs_output_t *out = obs_frontend_get_replay_buffer_output();
        if (out) {
            if (!obs_output_active(out)) {
                obs_data_t *data = obs_data_create();
                obs_data_set_int(data, "max_time_sec", value);
                obs_output_update(out, data);
                obs_data_release(data);
            }
            obs_output_release(out);
        }
        blog(LOG_INFO, "[replay-duration-inline] Replay duration: %d seconds", value);
    }
    void refresh() {
        if (!field) attach();
        if (!field) return;
        bool show = replayButton && replayButton->isVisible();
        if (holder && holder->isVisible() != show) holder->setVisible(show);
        const bool isLocked = locked();
        field->setEnabled(show && !isLocked);
        if (isLocked || !field->hasFocus()) {
            auto text = QString::number(seconds());
            if (field->text() != text) { QSignalBlocker block(field); field->setText(text); }
        }
    }
    void detach() {
        if (!holder) return;
        auto *widget = holder.data();
        holder.clear(); field.clear();
        if (row) {
            row->removeWidget(widget);
            row->setStretch(0, originalReplayStretch);
            row->setStretch(1, originalSaveStretch);
        }
        if (replayButton) replayButton->setSizePolicy(originalReplayPolicy);
        if (saveButton) saveButton->setSizePolicy(originalSavePolicy);
        delete widget;
        row.clear(); replayButton.clear(); saveButton.clear();
    }
public:
    Inline() {
        connect(&timer, &QTimer::timeout, this, [this] { refresh(); });
        timer.start(500);
        QTimer::singleShot(0, this, [this] { refresh(); });
    }
    ~Inline() override { timer.stop(); detach(); }
    void onEvent(obs_frontend_event event) {
        if (event == OBS_FRONTEND_EVENT_REPLAY_BUFFER_STARTING ||
            event == OBS_FRONTEND_EVENT_REPLAY_BUFFER_STARTED ||
            event == OBS_FRONTEND_EVENT_REPLAY_BUFFER_STOPPING) transitioning = true;
        if (event == OBS_FRONTEND_EVENT_REPLAY_BUFFER_STOPPED) transitioning = false;
        if (event == OBS_FRONTEND_EVENT_EXIT) { timer.stop(); detach(); }
        else if (event == OBS_FRONTEND_EVENT_FINISHED_LOADING ||
                 event == OBS_FRONTEND_EVENT_PROFILE_CHANGED ||
                 event == OBS_FRONTEND_EVENT_REPLAY_BUFFER_STOPPED ||
                 event == OBS_FRONTEND_EVENT_REPLAY_BUFFER_STARTING)
            QTimer::singleShot(0, this, [this] { refresh(); });
    }
};
Inline *instance = nullptr;
void eventCb(obs_frontend_event e, void *data) { static_cast<Inline *>(data)->onEvent(e); }
}
bool obs_module_load(void) {
    blog(LOG_INFO, "[replay-duration-inline] Loaded with Qt %s", qVersion());
    instance = new Inline;
    obs_frontend_add_event_callback(eventCb, instance);
    return true;
}
void obs_module_unload(void) {
    if (!instance) return;
    obs_frontend_remove_event_callback(eventCb, instance);
    delete instance;
    instance = nullptr;
}
