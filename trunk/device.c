#include "ibm.h"
#include "device.h"

static void *device_priv[256];
static device_t *devices[256];

void device_init()
{
        memset(devices, 0, sizeof(devices));
}

void device_add(device_t *d)
{
        int c = 0;
        void *priv;
        
        while (devices[c] != NULL && c < 256)
                c++;
        
        if (c >= 256)
                fatal("device_add : too many devices\n");
        
        priv = d->init();
        if (priv == NULL)
                fatal("device_add : device init failed\n");
        
        devices[c] = d;
        device_priv[c] = priv;        
}

void device_close_all()
{
        int c;
        
        for (c = 0; c < 256; c++)
        {
                if (devices[c] != NULL)
                {
                        devices[c]->close(device_priv[c]);
                        devices[c] = device_priv[c] = NULL;
                }
        }
}

int device_available(device_t *d)
{
#ifdef RELEASE_BUILD
        if (d->flags & DEVICE_NOT_WORKING)
                return 0;
#endif
        if (d->available)
                return d->available();
                
        return 1;        
}

void device_speed_changed()
{
        int c;
        
        for (c = 0; c < 256; c++)
        {
                if (devices[c] != NULL)
                {
                        if (devices[c]->speed_changed != NULL)
                        {
                                devices[c]->speed_changed(device_priv[c]);
                        }
                }
        }
}

void device_force_redraw()
{
        int c;
        
        for (c = 0; c < 256; c++)
        {
                if (devices[c] != NULL)
                {
                        if (devices[c]->force_redraw != NULL)
                        {
                                devices[c]->force_redraw(device_priv[c]);
                        }
                }
        }
}

char *device_add_status_info(char *s, int max_len)
{
        int c;
        
        s[0] = 0;
        
        for (c = 0; c < 256; c++)
        {
                if (devices[c] != NULL)
                {
                        if (devices[c]->add_status_info != NULL)
                        {
                                int len = devices[c]->add_status_info(s, max_len, device_priv[c]);
                                s += len;
                                max_len -= len;
                        }
                }
        }
}
