#include "storedspan.h"

StoredSpan::StoredSpan()
{
   return;
}

StoredSpan::StoredSpan(std::string t_span_name, opentelemetry::v1::nostd::shared_ptr<opentelemetry::v1::trace::Span> t_span)
{
   span_name = t_span_name;
   span = t_span;
}

void StoredSpan::setSpanName(std::string t_span_name)
{
   span_name = t_span_name;
}

void StoredSpan::setSpanPtr(opentelemetry::v1::nostd::shared_ptr<opentelemetry::v1::trace::Span> t_span)
{
   span = t_span;
}

std::string StoredSpan::getSpanName()
{
   std::string s_name = span_name;
   return s_name;
}

opentelemetry::v1::nostd::shared_ptr<opentelemetry::v1::trace::Span> StoredSpan::getSpanPtr()
{
   return span;
}

void StoredSpan::setQueryText(std::string q_text)
{
   query_text = q_text;
}

void StoredSpan::setQueryLevel(std::string q_level)
{
   query_level = q_level;
}

void StoredSpan::setQueryId(std::string q_id)
{
   query_id = q_id;
}

std::string StoredSpan::getQueryText()
{
   return query_text;
}

std::string StoredSpan::getQueryLevel()
{
   return query_level;
}

std::string StoredSpan::getQueryId()
{
   return query_id;
}
