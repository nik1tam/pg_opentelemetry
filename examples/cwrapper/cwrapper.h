#ifndef OTEL_CWRAPPER_H
#define OTEL_CWRAPPER_H

#define ERRCODE_OK 0
#define ERRCODE_ERR 1

#ifdef __cplusplus
extern "C" {
#endif    

int cWrapper_InitTracer(const char *url, const char *bin_mode, const char *deb_mode);

int cWrapper_StartSpan(const char *span_name); // *q_id, const char *q_lvl, const char *q_txt);

int cWrapper_EndSpan(const char *span_name);

int cWrapper_CleanupTracer(void);

#ifdef __cplusplus
}
#endif    

#endif