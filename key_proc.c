#include "lvgl.h"
#include "input_key.h"
#include "nav_launcher.h"
#include "page_events.h"

#define ALKAID_KEY_COUNT 10
#define ALKAID_KEY_TASK_CYCLE 25
#define ALKAID_KEY_DEBOUNCE_DELAY 50
#define ALKAID_KEY_SHORT_CLICK_MAX_DURATION 500
#define ALKAID_KEY_MULTI_CLICK_DELAY 200
#define ALKAID_KEY_LONG_DURATION 1000
#define ALKAID_KEY_LONG_PRESSED_REPEAT_COUNT_MAX 9

#define ALKAID_KEY_MAKE(key, action) (((uint32_t)key << 16) | action)

typedef struct
{
    uint8_t key;
    bool is_down;
    uint32_t ms_key_down;
    bool is_report_down;
    uint16_t cnt_long;
} alkaid_key_info_t;

static alkaid_key_info_t m_alkaid_keys[ALKAID_KEY_COUNT] = {0};

static void _report_alkaid_key(uint32_t key_info)
{
    // printf("report alkaid key: key:%d, action:%d\n", key_info >> 16, key_info & 0xFFFF);
    lv_event_send(nav_launcher_get_runtime()->joint_screen, PAGE_EVENT_KEY, (void *)key_info);
}

#if 1
static void _alkaid_key_clear(alkaid_key_info_t *info)
{
    info->is_down = false;
    info->ms_key_down = 0;
    info->is_report_down = false;
    info->cnt_long = 0;
}

static bool m_alkaid_work_running = false;

lv_timer_t * m_alkaid_key_timer = NULL;

static void _alkaid_key_timer_handler(lv_timer_t * timer) {
    bool is_any_down = false;
    for (int i = 0; i < ALKAID_KEY_COUNT; i++)
    {
        alkaid_key_info_t *info = &m_alkaid_keys[i];
        if (info->is_down)
        {
            is_any_down = true;
            uint32_t ms_key_duration = lv_tick_elaps(info->ms_key_down);
            if (ms_key_duration >= ALKAID_KEY_DEBOUNCE_DELAY && !info->is_report_down)
            {
                _report_alkaid_key(ALKAID_KEY_MAKE(info->key, KEY_ACTION_PRESSED));
                info->is_report_down = true;
            }
            uint16_t cnt_long = ms_key_duration / ALKAID_KEY_LONG_DURATION;
            if (cnt_long > 0 && cnt_long != info->cnt_long)
            {
                if (cnt_long == 1)
                {
                    _report_alkaid_key(ALKAID_KEY_MAKE(info->key, KEY_ACTION_LONG_PRESSED));
                }
                else
                {
                    _report_alkaid_key(ALKAID_KEY_MAKE(info->key, KEY_ACTION_LONG_PRESSED_REPEAT));
                }
                info->cnt_long = cnt_long;
                if (cnt_long == ALKAID_KEY_LONG_PRESSED_REPEAT_COUNT_MAX + 1)
                {
                    m_alkaid_work_running = false;
                }
            }
            break;
        }
    }

    if (!is_any_down)
    {
        m_alkaid_work_running = false;
    }
    if (!m_alkaid_work_running) {
        lv_timer_del(timer);
        m_alkaid_key_timer = NULL;
    }
}

static bool m_alkaid_key_init = false;
#endif

//////// Map keyboard key to INPUT_KEY //////////


static void _handle_sdl_key(lv_event_t *e) {
    // down
    char c = *(char *)(e->param);

    // printf("_handle_sdl_key, key: %c\n", c);
    if (c == 'j')
    {
        printf(" power key down\n");
        uint8_t alkaid_key = INPUT_KEY_POWER;
        for (int i = 0; i < ALKAID_KEY_COUNT; i++)
        {
            alkaid_key_info_t *info = &m_alkaid_keys[i];
            if (info->key != alkaid_key)
            {
                continue;
            }
            if (info->is_down)
            {
                return;
            }
            info->is_down = true;
            info->ms_key_down = lv_tick_get();
            printf("  ts: %d\n", info->ms_key_down);
            if (!m_alkaid_work_running)
            {
                m_alkaid_work_running = true;
                lv_timer_create(_alkaid_key_timer_handler, ALKAID_KEY_TASK_CYCLE, NULL);
            }
        }
    }
    // up
    if (c == 'k')
    {
        printf(" power key up\n");
        uint8_t alkaid_key = INPUT_KEY_POWER;
        for (int i = 0; i < ALKAID_KEY_COUNT; i++)
        {
            alkaid_key_info_t *info = &m_alkaid_keys[i];
            if (info->key != alkaid_key)
            {
                continue;
            }
            if (!info->is_down)
            {
                return;
            }
            uint32_t ms_key_duration = lv_tick_elaps(info->ms_key_down);
            printf("  key duration: %d\n", ms_key_duration);
            // if (ms_key_duration >= ALKAID_KEY_DEBOUNCE_DELAY)
            {
                if (ms_key_duration < ALKAID_KEY_LONG_DURATION)
                {
                    _report_alkaid_key(ALKAID_KEY_MAKE(alkaid_key, KEY_ACTION_RELEASED));
                }
                if (ms_key_duration < ALKAID_KEY_SHORT_CLICK_MAX_DURATION)
                {
                    _report_alkaid_key(ALKAID_KEY_MAKE(alkaid_key, KEY_ACTION_SHORT_CLICKED));
                }
                if (ms_key_duration >= ALKAID_KEY_LONG_DURATION)
                {
                    _report_alkaid_key(ALKAID_KEY_MAKE(alkaid_key, KEY_ACTION_LONG_UP));
                }
            }
            _alkaid_key_clear(info);
            break;
        }
    }
}

void key_proc_init(void) {
#if 1
    if (!m_alkaid_key_init)
    {
        m_alkaid_key_init = true;
        memset(m_alkaid_keys, 0, sizeof(m_alkaid_keys));
        int cnt = 0;
        m_alkaid_keys[cnt++].key = INPUT_KEY_POWER;
        m_alkaid_keys[cnt++].key = INPUT_KEY_ENCODER_DOWN;
        m_alkaid_keys[cnt++].key = INPUT_KEY_ENCODER_UP;
        m_alkaid_keys[cnt++].key = INPUT_KEY_MENU;
        m_alkaid_keys[cnt++].key = INPUT_KEY_OPTION;
        m_alkaid_keys[cnt++].key = INPUT_KEY_SELECT;
        m_alkaid_keys[cnt++].key = INPUT_KEY_BACK;
    }
#endif
    lv_obj_add_event_cb(nav_launcher_get_runtime()->joint_screen, _handle_sdl_key, LV_EVENT_KEY, NULL);
}
