#ifndef _STOREDSPAN
#define _STOREDSPAN
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

class StoredSpan
{
private:
  std::string span_name;
  std::string query_text;
  std::string query_level;
  std::string query_id;
  opentelemetry::v1::nostd::shared_ptr<opentelemetry::v1::trace::Span> span;

  public:
    StoredSpan();
    StoredSpan(std::string t_span_name, opentelemetry::v1::nostd::shared_ptr<opentelemetry::v1::trace::Span> t_span);
    friend bool operator == (const StoredSpan& obj1, const StoredSpan& obj2)
    {
      return obj1.span_name == obj2.span_name;
    }
    void setSpanName(std::string t_span_name);
    void setQueryText(std::string q_text);
    void setQueryLevel(std::string q_level);
    void setQueryId(std::string q_id);
    void setSpanPtr(opentelemetry::v1::nostd::shared_ptr<opentelemetry::v1::trace::Span> t_span);
    std::string getSpanName();
    std::string getQueryText();
    std::string getQueryLevel();
    std::string getQueryId();
    opentelemetry::v1::nostd::shared_ptr<opentelemetry::v1::trace::Span> getSpanPtr();
};
#endif