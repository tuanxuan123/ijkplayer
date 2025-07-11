#ifndef PIXUI_CURL_H
#define PIXUI_CURL_H

#ifdef _WIN32
#define EXPORT_API  __declspec(dllexport)
#else
#define EXPORT_API  __attribute__ ((visibility ("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif
    EXPORT_API void pixui_curl_slist_free_all(struct curl_slist * curl_slist);

    EXPORT_API CURLMcode pixui_curl_multi_add_handle(CURLM *multi,
                                CURL *data);

    EXPORT_API const char *pixui_curl_multi_strerror(CURLMcode error);

    EXPORT_API CURLMcode pixui_curl_multi_wait(CURLM *multi,
                          struct curl_waitfd extra_fds[],
                          unsigned int extra_nfds,
                          int timeout_ms,
                          int *ret);

    EXPORT_API const char *pixui_curl_easy_strerror(CURLcode error);

    EXPORT_API CURLMcode pixui_curl_multi_remove_handle(CURLM *multi,
                                   CURL *data);

    EXPORT_API CURLcode pixui_curl_easy_getinfo(CURL *data, CURLINFO info, ...);

    EXPORT_API CURLMcode pixui_curl_multi_perform(CURLM *multi, int *running_handles);

    EXPORT_API void pixui_curl_easy_cleanup(CURL *data);

    EXPORT_API struct curl_slist *pixui_curl_slist_append(struct curl_slist *list,
                                     const char *data);

    EXPORT_API CURLMcode pixui_curl_multi_cleanup(CURLM *multi);

    EXPORT_API void pixui_curl_global_cleanup(void);

    EXPORT_API CURLcode pixui_curl_global_init(long flags);

    EXPORT_API CURLM *pixui_curl_multi_init(void);

    EXPORT_API CURL *pixui_curl_easy_init(void);

    EXPORT_API CURLcode pixui_curl_easy_perform(CURL *curl);

    EXPORT_API CURLMsg *pixui_curl_multi_info_read(CURLM *multi, int *msgs_in_queue);

    EXPORT_API CURLcode pixui_curl_easy_setopt(CURL *data, CURLoption tag, ...);

    EXPORT_API CURLMcode pixui_curl_multi_setopt(CURLM *multi, CURLMoption option, ...);

    EXPORT_API char *pixui_curl_version(void);
#ifdef __cplusplus
}
#endif
#endif /* CURLINC_CURL_H */
