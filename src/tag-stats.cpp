/*

OSM Data Model Tools

tag-stats

Copyright (C) 2018-2022  Jochen Topf <jochen@topf.org>

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

#include <osmium/io/any_input.hpp>

#include <lyra.hpp>

#include <algorithm>
#include <cstddef>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

int main(int argc, char* argv[]) {
    std::string input_filename;
    std::size_t max_tags = 10000; // essentially all
    std::size_t min_count = 100;
    bool with_values = false;
    bool help = false;

    const auto cli
        = lyra::opt(max_tags, "N")
            ["-m"]["--max-tags"]
            ("count tags only on objects with no more than this many tags (default: all)")
        | lyra::opt(min_count, "N")
            ["-c"]["--min-count"]
            ("min count to output (default: " + std::to_string(min_count) + ")")
        | lyra::opt(with_values)
            ["-v"]["--with-values"]
            ("also count values")
        | lyra::help(help)
        | lyra::arg(input_filename, "FILENAME")
            ("input file");

    const auto result = cli.parse(lyra::args(argc, argv));
    if (!result) {
        std::cerr << "Error in command line: " << result.message() << '\n';
        return 1;
    }

    if (help) {
        std::cout << cli
                  << "\nCreate tag statistics.\n";
        return 0;
    }

    if (input_filename.empty()) {
        std::cerr << "Missing input filename. Try '-h'.\n";
        return 1;
    }

    osmium::io::File input_file{input_filename};

    std::unordered_map<std::string, std::size_t> dict;

    osmium::io::Reader reader{input_file};
    while (const auto buffer = reader.read()) {
        for (const auto& object : buffer.select<osmium::OSMObject>()) {
            if (object.tags().size() <= max_tags) {
                for (const auto& tag : object.tags()) {
                    if (with_values) {
                        dict[tag.key() + std::string{"="} + tag.value()]++;
                    } else {
                        dict[tag.key()]++;
                    }
                }
            }
        }
    }
    reader.close();

    using si = std::pair<std::string, std::size_t>;
    std::vector<si> common_keys;
    for (const auto& p : dict) {
        if (p.second >= min_count) {
            common_keys.emplace_back(p);
        }
    }

    std::sort(common_keys.begin(), common_keys.end(), [](const si& a, const si& b) {
        return a.second > b.second;
    });

    for (const auto& p : common_keys) {
        std::cout << p.second << ' ' << p.first << '\n';
    }
}

