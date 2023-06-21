#include <list>
#include <string>
#include <cstdlib>

#include "opentelemetry/exporters/otlp/otlp_http_exporter_factory.h"
#include "opentelemetry/exporters/otlp/otlp_http_exporter_options.h"
//# otlp_http_exporter_options.h"
#include "opentelemetry/sdk/common/global_log_handler.h"
#include "opentelemetry/sdk/trace/simple_processor_factory.h"
#include "opentelemetry/sdk/trace/tracer_provider_factory.h"
#include "opentelemetry/trace/provider.h"

// sdk::TracerProvider is just used to call ForceFlush and prevent to cancel running exportings when
// destroy and shutdown exporters.It's optional to users.
#include "opentelemetry/sdk/trace/tracer_provider.h"

#include "cwrapper.h"
#include "storedspan.h"

static std::list<StoredSpan> spanList;

#ifdef __cplusplus
extern "C" {
#endif

// namespace trace     = opentelemetry::trace;
// namespace trace_sdk = opentelemetry::sdk::trace;
// namespace otlp      = opentelemetry::exporter::otlp;

// namespace internal_log = opentelemetry::sdk::common::internal_log;

int cWrapper_InitTracer(const char *url, const char *bin_mode, const char *deb_mode)
{
//    std::list<span_node_type> slist;
    opentelemetry::exporter::otlp::OtlpHttpExporterOptions opts;
    opts.url = url;
    std::string debug = deb_mode;
    opts.console_debug = debug != "" && debug != "0" && debug != "no";
    std::string binary_mode = bin_mode;
    if(binary_mode.size() >= 3 && binary_mode.substr(0, 3) == "bin")
      opts.content_type = opentelemetry::exporter::otlp::HttpRequestContentType::kBinary;

    if(opts.console_debug)
      opentelemetry::sdk::common::internal_log::GlobalLogHandler::SetLogLevel(opentelemetry::sdk::common::internal_log::LogLevel::Debug);

    // Create OTLP exporter instance
    auto exporter  = opentelemetry::exporter::otlp::OtlpHttpExporterFactory::Create(opts);
    auto processor = opentelemetry::sdk::trace::SimpleSpanProcessorFactory::Create(std::move(exporter));
    std::shared_ptr<opentelemetry::trace::TracerProvider> provider =
      opentelemetry::sdk::trace::TracerProviderFactory::Create(std::move(processor));
    // Set the global trace provider
    opentelemetry::trace::Provider::SetTracerProvider(provider);
    return 0;
}

int cWrapper_StartSpan(const char *q_count, const char *q_level, const char *q_text)
{
  opentelemetry::v1::nostd::string_view sname = q_text;
  std::string strname = q_count;
  auto provider = opentelemetry::trace::Provider::GetTracerProvider();
  opentelemetry::v1::nostd::shared_ptr<opentelemetry::v1::trace::Span> t_span;
  opentelemetry::v1::trace::Scope t_scope{t_span};
  StoredSpan s;

  t_span = provider->GetTracer("PostgreSQL", OPENTELEMETRY_SDK_VERSION)->StartSpan(sname);
  s.setSpanName(strname);
  s.setSpanPtr(t_span);
  s.setQueryId(q_count);
  s.setQueryLevel(q_level);
  s.setQueryText(q_text);
  auto scope = provider->GetTracer("PostgreSQL", OPENTELEMETRY_SDK_VERSION)->WithActiveSpan(t_span);
  spanList.push_front(s);
  return 0;
}

int cWrapper_EndSpan(const char *span_name)
{
  opentelemetry::v1::nostd::shared_ptr<opentelemetry::v1::trace::Span> span;
  std::string sname = span_name;
  int is_span_found = 1;
  for(std::list<StoredSpan>::iterator it = spanList.begin(); it != spanList.end(); it++)
  {
    if((it)->getSpanName() == sname)
    {
      span = (it)->getSpanPtr();
      span->End();
      is_span_found = 0;
      break;
    }
  }
  return is_span_found;
}

int cWrapper_CleanupTracer(void)
{
  // We call ForceFlush to prevent to cancel running exportings, It's optional.
  opentelemetry::nostd::shared_ptr<opentelemetry::trace::TracerProvider> provider =
      opentelemetry::trace::Provider::GetTracerProvider();
  if (provider)
  {
    static_cast<opentelemetry::sdk::trace::TracerProvider *>(provider.get())->ForceFlush();
  }

  std::shared_ptr<opentelemetry::trace::TracerProvider> none;
  opentelemetry::trace::Provider::SetTracerProvider(none);
  return 0;
}


#ifdef __cplusplus
}
#endif