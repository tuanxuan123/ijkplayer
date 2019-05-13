
#include "ijkutils.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <inttypes.h>

void ijk_av_freep(void *arg)
{
    void *val;

    memcpy(&val, arg, sizeof(val));
    memcpy(arg, &(void *){ NULL }, sizeof(val));
    free(val);
}

int ijk_av_strstart(const char *str, const char *pfx, const char **ptr)
{
    while (*pfx && *pfx == *str) {
        pfx++;
        str++;
    }
    if (!*pfx && ptr)
        *ptr = str;
    return !*pfx;
}




#ifdef _WIN32
#include "libavformat/url.h"

#define IJK_FF_PROTOCOL(x)                                                                          \
extern URLProtocol ff_##x##_protocol;                                                               \
int ijkav_register_##x##_protocol(URLProtocol *protocol, int protocol_size);                        \
int ijkav_register_##x##_protocol(URLProtocol *protocol, int protocol_size)                         \
{                                                                                                   \
    if (protocol_size != sizeof(URLProtocol)) {                                                     \
        av_log(NULL, AV_LOG_ERROR, "ijkav_register_##x##_protocol: ABI mismatch.\n");               \
        return -1;                                                                                  \
    }                                                                                               \
    memcpy(&ff_##x##_protocol, protocol, protocol_size);                                            \
    return 0;                                                                                       \
}

#define IJK_DUMMY_PROTOCOL(x)                                       \
IJK_FF_PROTOCOL(x);                                                 \
static const AVClass ijk_##x##_context_class = {                    \
    .class_name = #x,                                               \
    .item_name  = av_default_item_name,                             \
    .version    = LIBAVUTIL_VERSION_INT,                            \
    };                                                              \
                                                                    \
URLProtocol ff_##x##_protocol = {                                   \
    .name                = #x,                                      \
    .url_open2           = ijkdummy_open,                           \
    .priv_data_size      = 1,                                       \
    .priv_data_class     = &ijk_##x##_context_class,                \
};

static int ijkdummy_open(URLContext *h, const char *arg, int flags, AVDictionary **options)
{
	return -1;
}


IJK_FF_PROTOCOL(async);
IJK_DUMMY_PROTOCOL(ijkhttphook);
IJK_DUMMY_PROTOCOL(ijklongurl);
IJK_DUMMY_PROTOCOL(ijksegment);
IJK_DUMMY_PROTOCOL(ijktcphook);
IJK_DUMMY_PROTOCOL(ijkio);

#endif // _WIN32

