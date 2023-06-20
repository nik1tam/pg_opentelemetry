#ifndef OTEL_CWRAPPER_H
#define OTEL_CWRAPPER_H

#ifdef __cplusplus
extern "C" {
#endif    

void cWrapper_InitTracer(const char *url, const char *bin_mode, const char *deb_mode);

void cWrapper_StartSpan(const char *span_name);

void cWrapper_EndSpan(const char *span_name);

void cWrapper_CleanupTracer(void);

#ifdef __cplusplus
}
#endif    

#endif