#pragma once

#include <osmium/tags/tags_filter.hpp>

osmium::TagsFilter load_filter_patterns(const std::string& file_name);

