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

#include "opentelemetry/sdk/version/version.h"
/*
#include "opentelemetry/exporters/ostream/span_exporter.h"
#include "opentelemetry/exporters/ostream/span_exporter_factory.h"
#include "opentelemetry/exporters/ostream/log_record_exporter.h"
#include "opentelemetry/logs/provider.h"
#include "opentelemetry/sdk/logs/logger_provider_factory.h"
#include "opentelemetry/sdk/logs/simple_log_record_processor_factory.h"
*/
#include "cwrapper.h"
#include "storedspan.h"

static std::list<StoredSpan> spanList;

#ifdef __cplusplus
extern "C" {
#endif

/*
namespace trace     = opentelemetry::trace;
namespace trace_sdk = opentelemetry::sdk::trace;
namespace otlp      = opentelemetry::exporter::otlp;

namespace internal_log = opentelemetry::sdk::common::internal_log;

namespace logs_api      = opentelemetry::logs;
namespace logs_sdk      = opentelemetry::sdk::logs;
namespace logs_exporter = opentelemetry::exporter::logs;

namespace trace_api      = opentelemetry::trace;
namespace trace_exporter = opentelemetry::exporter::trace;
*/

/* Only one provider allowed simultaneously. Logs switched off to enable HTTP */

/*
void InitLogger()
{
  // Create ostream log exporter instance
  auto exporter =
      std::unique_ptr<opentelemetry::sdk::logs::LogRecordExporter>(new opentelemetry::exporter::logs::OStreamLogRecordExporter);
  auto processor = opentelemetry::sdk::logs::SimpleLogRecordProcessorFactory::Create(std::move(exporter));
  std::shared_ptr<opentelemetry::logs::LoggerProvider> provider(
      opentelemetry::sdk::logs::LoggerProviderFactory::Create(std::move(processor)));

  // Set the global logger provider
  opentelemetry::logs::Provider::SetLoggerProvider(provider);
}

void CleanupLogger()
{
  std::shared_ptr<opentelemetry::logs::LoggerProvider> none;
  opentelemetry::logs::Provider::SetLoggerProvider(none);
}
*/
/////////////
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
    // Create LOG exporter and provider
    /*
    auto log_exporter  = opentelemetry::exporter::trace::OStreamSpanExporterFactory::Create();
    auto log_processor = opentelemetry::sdk::trace::SimpleSpanProcessorFactory::Create(std::move(log_exporter));
    std::shared_ptr<opentelemetry::trace::TracerProvider> log_provider =
      opentelemetry::sdk::trace::TracerProviderFactory::Create(std::move(log_processor));
    opentelemetry::trace::Provider::SetTracerProvider(log_provider);
    */
    return ERRCODE_OK;
}

int cWrapper_StartSpan(const char *span_name) //*q_count, const char *q_level, const char *q_text)
{
  bool check_segfault = true;
  std::string spname = span_name;
/*
  std::string strname = q_count;
  std::string strlevel = q_level;
  std::string strtext = q_text;
*/

  opentelemetry::v1::nostd::string_view sname = opentelemetry::v1::nostd::string_view(span_name); // q_text;
  auto provider = opentelemetry::trace::Provider::GetTracerProvider();
  opentelemetry::v1::nostd::shared_ptr<opentelemetry::v1::trace::Span> t_span;
  StoredSpan s;

  t_span = provider->GetTracer("PostgreSQL", OPENTELEMETRY_SDK_VERSION)->StartSpan(sname);
  if(t_span)
  {
    opentelemetry::v1::trace::Scope t_scope{t_span};
    auto scope = provider->GetTracer("PostgreSQL", OPENTELEMETRY_SDK_VERSION)->WithActiveSpan(t_span);
  }
  s.setSpanName(spname);
  s.setSpanPtr(t_span);
/*
  s.setQueryId(strname);
  s.setQueryLevel(strlevel);
  s.setQueryText(strtext);
*/
  spanList.push_front(s);
  return ERRCODE_OK;
}

int cWrapper_EndSpan(const char *span_name)
{
  opentelemetry::v1::nostd::shared_ptr<opentelemetry::v1::trace::Span> span;
  std::string sname = span_name;
  int is_span_found = ERRCODE_ERR;
  for(std::list<StoredSpan>::iterator it = spanList.begin(); it != spanList.end(); it++)
  {
    if((it)->getSpanName() == sname)
    {
      span = (it)->getSpanPtr();
      span->End();
      spanList.erase(it);
      is_span_found = ERRCODE_OK;
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
  return ERRCODE_OK;
}


#ifdef __cplusplus
}
#endif
