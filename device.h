typedef struct device_t
{
        char name[50];
        uint32_t flags;
        void *(*init)();
        void (*close)(void *p);
        int  (*available)();
        void (*speed_changed)(void *p);
        void (*force_redraw)(void *p);
        int  (*add_status_info)(char *s, int max_len, void *p);
} device_t;

void device_init();
void device_add(device_t *d);
void device_close_all();
int device_available(device_t *d);
void device_speed_changed();
void device_force_redraw();
char *device_add_status_info(char *s, int max_len);

enum
{
        DEVICE_NOT_WORKING = 1 /*Device does not currently work correctly and will be disabled in a release build*/
};
