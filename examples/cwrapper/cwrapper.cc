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

typedef struct span_node_type {
    std::string span_name;
    opentelemetry::nostd::shared_ptr<opentelemetry::trace::Span> span;
    opentelemetry::v1::trace::Scope scope{span};
    span_node_type *next;
} span_node_type;

//static std::list<span_node_type> span_list;
static span_node_type *span_list = NULL;

static void span_list_add(span_node_type *el)
{
  el->next = span_list;
  span_list = el;
}

static void span_list_remove(span_node_type *el)
{
  span_node_type *node = span_list;
  span_node_type *prev_node = span_list;
  std::string s = el->span_name;
  while(node != NULL)
  {
    if(node->span_name == s)
    {
      if(node == span_list)
        span_list = node->next;
      else
        prev_node->next = node->next;

      free(node);
    }
//      break;
    
    prev_node = node;
    node = node->next;
  }

/*
  if(node != NULL )
  {
    if(node == span_list)
      span_list = node->next;
    else
      prev_node->next = node->next;

    free(node);
  }
*/
}

static span_node_type * span_list_search(std::string span_name)
{
  span_node_type *node = span_list;
  while(node != NULL)
  {
    if(node->span_name == span_name)
      break;
    
    node = node->next;
  }
  return node;
}

static void span_list_cleanup()
{
  span_node_type *node = span_list;
  span_node_type *next_node = span_list;
  while(node != NULL)
  {
    next_node = node->next;
    free(node);
    node = next_node;
  }
  span_list = NULL;
}

#ifdef __cplusplus
extern "C" {
#endif

// namespace trace     = opentelemetry::trace;
// namespace trace_sdk = opentelemetry::sdk::trace;
// namespace otlp      = opentelemetry::exporter::otlp;

// namespace internal_log = opentelemetry::sdk::common::internal_log;

void cWrapper_InitTracer(const char *url, const char *bin_mode, const char *deb_mode)
{
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
}

void cWrapper_StartSpan(const char *span_name)
{
  span_node_type *span_node = new(span_node_type);
  opentelemetry::v1::nostd::string_view sname = span_name;
  auto provider = opentelemetry::trace::Provider::GetTracerProvider();
  span_node->span = provider->GetTracer("PostgreSQL", OPENTELEMETRY_SDK_VERSION)->StartSpan(sname);
  span_node->scope = provider->GetTracer("PostgreSQL", OPENTELEMETRY_SDK_VERSION)->WithActiveSpan(span_node->span);
  span_node->span_name = span_name;

  span_list_add(span_node);
  //span_list.push_front(span_node);
}

void cWrapper_EndSpan(const char *span_name)
{
  opentelemetry::nostd::shared_ptr<opentelemetry::trace::Span> span;
  std::string sname = span_name;
  span_node_type *t_span = span_list_search(sname);

  if(t_span != NULL)
  {
    t_span->span->End();
    span_list_remove(t_span);
  }
/*
  for(std::list<span_node_type>::iterator it = span_list.begin(); it != span_list.end(); it++)
  {
    if((it)->span_name == sname)
    {
      (it)->span->End();
      span_list.remove_if(*it);
      break;
    }
  }
*/
}

void cWrapper_CleanupTracer(void)
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
  span_list_cleanup();
}


#ifdef __cplusplus
}
#endif