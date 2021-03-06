/*

OSM Data Model Tools

line-or-polygon

Copyright (C) 2018  Jochen Topf <jochen@topf.org>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.

*/

#include "filter.hpp"

#include <osmium/io/any_input.hpp>
#include <osmium/io/any_output.hpp>
#include <osmium/tags/tags_filter.hpp>

#include <clara.hpp>

#include <cassert>
#include <cstring>
#include <iostream>
#include <ostream>
#include <string>
#include <unordered_map>

static bool debug = false;

enum class lptype {
    unclassified,
    unknown,
    linestring,
    polygon,
    both,
    neutral,
    error
};

template <typename TChar, typename TTraits>
std::basic_ostream<TChar, TTraits>& operator<<(std::basic_ostream<TChar, TTraits>& out, const lptype lpt) {
    switch (lpt) {
        case lptype::unclassified:
            out << "unclassified";
            break;
        case lptype::unknown:
            out << "unknown";
            break;
        case lptype::linestring:
            out << "linestring";
            break;
        case lptype::polygon:
            out << "polygon";
            break;
        case lptype::both:
            out << "both";
            break;
        case lptype::neutral:
            out << "neutral";
            break;
        case lptype::error:
            out << "error";
            break;
    }
    return out;
}

osmium::TagsFilter filter_linestring;
osmium::TagsFilter filter_polygon;
osmium::TagsFilter filter_meta;
osmium::TagsFilter filter_neutral;
osmium::TagsFilter filter_import;

lptype check_tag(const osmium::Tag& tag) noexcept {
    if (filter_polygon(tag)) {
        return lptype::polygon;
    }

    if (filter_linestring(tag)) {
        return lptype::linestring;
    }

    if (filter_meta(tag) || filter_neutral(tag) || filter_import(tag)) {
        return lptype::neutral;
    }

    return lptype::unknown;
}

lptype get_type(const osmium::TagList& tags, std::vector<std::string>* unknown_keys) {
    auto type = lptype::unclassified;

    for (const auto& tag : tags) {
        if (debug) {
            std::cerr << "  " << type << " -> " << tag.key() << '=' << tag.value();
        }

        if (!std::strcmp(tag.key(), "area")) {
            if (!std::strcmp(tag.value(), "yes")) {
                if (debug) {
                    std::cerr << " area=yes\n";
                }
                return lptype::polygon;
            }
            if (!std::strcmp(tag.value(), "no")) {
                if (debug) {
                    std::cerr << " area=no\n";
                }
                return lptype::linestring;
            }
            if (debug) {
                std::cerr << " area=INVALID\n  -> error";
            }
            return lptype::error;
        }

        const auto t = check_tag(tag);
        if (debug) {
            std::cerr << " [" << t << "]\n";
        }

        if (t == lptype::unknown) {
            unknown_keys->emplace_back(tag.key());
        }

        if (t == lptype::neutral) {
            continue;
        }

        switch (type) {
            case lptype::unclassified:
                if (t == lptype::linestring || t == lptype::polygon) {
                    type = t;
                } else {
                    type = lptype::unknown;
                }
                break;
            case lptype::linestring:
                if (t == lptype::polygon) {
                    return lptype::both;
                } else if (t != lptype::linestring) {
                    type = lptype::unknown;
                }
                break;
            case lptype::polygon:
                if (t == lptype::linestring) {
                    return lptype::both;
                } else if (t != lptype::polygon) {
                    type = lptype::unknown;
                }
                break;
            case lptype::unknown:
                break;
            default:
                assert(false);
        }
    }

    if (debug) {
        std::cerr << "  -> " << type << '\n';
    }
    return type;
}

std::unordered_map<std::string, uint64_t> keys;

void count_keys(const std::vector<std::string>& unknown_keys) {
    for (const auto& key : unknown_keys) {
        ++keys[key];
    }
}

int main(int argc, char* argv[]) {
    std::string input_filename;
    std::string expressions_directory{"."};
    std::string output_directory{"."};
    bool help = false;

    const auto cli
        = clara::Opt(output_directory, "DIR")
            ["-o"]["--output-dir"]
            ("output directory")
        | clara::Opt(expressions_directory, "DIR")
            ["-e"]["--expressions-dir"]
            ("directory with expression files")
        | clara::Opt(debug)
            ["-d"]["--debug"]
            ("enable debug mode")
        | clara::Help(help)
        | clara::Arg(input_filename, "FILENAME")
            ("input file");

    const auto result = cli.parse(clara::Args(argc, argv));
    if (!result) {
        std::cerr << "Error in command line: " << result.errorMessage() << '\n';
        return 1;
    }

    if (help) {
        std::cout << cli
                  << "\nClassify ways into linestrings and/or polygons.\n";
        return 0;
    }

    if (input_filename.empty()) {
        std::cerr << "Missing input filename. Try '-h'.\n";
        return 1;
    }

    filter_linestring = load_filter_patterns(expressions_directory + "/linestring-tags");
    filter_polygon    = load_filter_patterns(expressions_directory + "/polygon-tags");
    filter_meta       = load_filter_patterns(expressions_directory + "/meta-tags");
    filter_neutral    = load_filter_patterns(expressions_directory + "/neutral-tags");
    filter_import     = load_filter_patterns(expressions_directory + "/import-tags");

    osmium::io::File input_file{input_filename};

    osmium::io::Reader reader{input_file, osmium::osm_entity_bits::way};

    osmium::io::Writer writer_unknown{output_directory + "/lp-unknown.osm.pbf", osmium::io::overwrite::allow};
    osmium::io::Writer writer_linestring{output_directory + "/lp-linestring.osm.pbf", osmium::io::overwrite::allow};
    osmium::io::Writer writer_polygon{output_directory + "/lp-polygon.osm.pbf", osmium::io::overwrite::allow};
    osmium::io::Writer writer_both{output_directory + "/lp-both.osm.pbf", osmium::io::overwrite::allow};
    osmium::io::Writer writer_no_tags{output_directory + "/lp-no-tags.osm.pbf", osmium::io::overwrite::allow};
    osmium::io::Writer writer_error{output_directory + "/lp-error.osm.pbf", osmium::io::overwrite::allow};

    std::uint64_t count_closed = 0;
    std::uint64_t count_nonclosed = 0;
    std::uint64_t count_unknown = 0;
    std::uint64_t count_linestring = 0;
    std::uint64_t count_polygon = 0;
    std::uint64_t count_both = 0;
    std::uint64_t count_error = 0;
    std::uint64_t count_no_tags = 0;

    while (const auto buffer = reader.read()) {
        for (const auto& way : buffer.select<osmium::Way>()) {
            if (way.is_closed()) {
                ++count_closed;
                if (way.tags().empty()) {
                    ++count_no_tags;
                    writer_no_tags(way);
                } else {
                    if (debug) {
                        std::cerr << "WAY " << way.id() << '\n';
                    }
                    std::vector<std::string> unknown_keys;
                    auto type = get_type(way.tags(), &unknown_keys);
                    switch (type) {
                        case lptype::unclassified:
                            ++count_no_tags;
                            writer_no_tags(way);
                            break;
                        case lptype::unknown:
                            ++count_unknown;
                            writer_unknown(way);
                            break;
                        case lptype::linestring:
                            ++count_linestring;
                            writer_linestring(way);
                            break;
                        case lptype::polygon:
                            ++count_polygon;
                            writer_polygon(way);
                            break;
                        case lptype::neutral:
                            break;
                        case lptype::both:
                            ++count_both;
                            writer_both(way);
                            count_keys(unknown_keys);
                            break;
                        case lptype::error:
                            ++count_error;
                            writer_error(way);
                            break;
                    }
                }
            } else {
                ++count_nonclosed;
            }
        }
    }

    reader.close();

    std::cout << "Statistics:"
              << "\n  non-closed: "  << count_nonclosed
              << "\n  closed:     "  << count_closed      << " (100%)"
              << "\n    unknown:    " << count_unknown    << " (" << (count_unknown    * 100 / count_closed)
              << "%)\n    linestring: " << count_linestring << " (" << (count_linestring * 100 / count_closed)
              << "%)\n    polygon:    " << count_polygon    << " (" << (count_polygon    * 100 / count_closed)
              << "%)\n    both:       " << count_both       << " (" << (count_both       * 100 / count_closed)
              << "%)\n    no tags:    " << count_no_tags    << " (" << (count_no_tags    * 100 / count_closed)
              << "%)\n    error:      " << count_error      << " (" << (count_error      * 100 / count_closed)
              << "%)\n";

    std::cout << "Keys:\n";

    using si = std::pair<std::string, uint64_t>;
    std::vector<si> common_keys;
    for (const auto& p : keys) {
        if (p.second > 10000) {
            common_keys.emplace_back(p);
        }
    }

    std::sort(common_keys.begin(), common_keys.end(), [](const si& a, const si& b) {
        return a.second > b.second;
    });

    for (const auto& p : common_keys) {
        std::cout << p.first << ' ' << p.second << '\n';
    }
}

