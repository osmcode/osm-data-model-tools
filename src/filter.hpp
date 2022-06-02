#pragma once

#include <osmium/tags/tags_filter.hpp>

osmium::TagsFilter load_filter_patterns(std::string const &file_name);
