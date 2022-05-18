/*

OSM Data Model Tools

duplicate-segments

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

#include <osmium/index/id_set.hpp>
#include <osmium/io/any_input.hpp>
#include <osmium/io/any_output.hpp>
#include <osmium/util/verbose_output.hpp>

#include <clara.hpp>

#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <utility>
#include <vector>

struct node_pair : public std::pair<osmium::object_id_type, osmium::object_id_type> {

    node_pair(osmium::object_id_type a, osmium::object_id_type b) :
        pair(std::min(a, b), std::max(a, b)) {
    }

}; // struct node_pair

class counter {

    std::vector<std::size_t> m_counts;

public:

    void increment(std::size_t value) {
        if (value >= m_counts.size()) {
            m_counts.resize(value + 1);
        }
        ++m_counts[value];
    }

    using const_iterator = std::vector<std::size_t>::const_iterator;

    const_iterator begin() const noexcept {
        return m_counts.begin();
    }

    const_iterator end() const noexcept {
        return m_counts.end();
    }

}; // class counter

int main(int argc, char* argv[]) {
    std::string input_filename;
    std::string output_directory{"."};
    bool help = false;

    const auto cli
        = clara::Opt(output_directory, "DIR")
            ["-o"]["--output-dir"]
            ("output directory")
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
                  << "\nCreate statistics on duplicate segments.\n";
        return 0;
    }

    if (input_filename.empty()) {
        std::cerr << "Missing input filename. Try '-h'.\n";
        return 1;
    }

    osmium::io::File input_file{input_filename};

    osmium::VerboseOutput vout{true};

    osmium::index::IdSetDense<osmium::unsigned_object_id_type> in_multiple_ways;

    vout << "Reading nodes in ways...\n";

    {
        osmium::index::IdSetDense<osmium::unsigned_object_id_type> in_way;
        osmium::io::Reader reader1{input_file, osmium::osm_entity_bits::way};
        while (const auto buffer = reader1.read()) {
            for (const auto& way : buffer.select<osmium::Way>()) {
                if (way.nodes().empty()) {
                    continue;
                }
                const auto *it = way.nodes().begin();
                if (way.is_closed()) {
                    ++it;
                }
                for (; it != way.nodes().end(); ++it) {
                    if (in_way.get(it->positive_ref())) {
                        in_multiple_ways.set(it->positive_ref());
                    } else {
                        in_way.set(it->positive_ref());
                    }
                }
            }
        }
        reader1.close();
    }

    vout << "Reading segments...\n";

    std::vector<node_pair> segments;

    osmium::io::Reader reader2{input_file, osmium::osm_entity_bits::way};
    while (const auto buffer = reader2.read()) {
        for (const auto& way : buffer.select<osmium::Way>()) {
            if (way.nodes().size() > 1) {
                const auto *it = way.nodes().begin();
                for (++it; it != way.nodes().end(); ++it) {
                    const auto id1 = (it - 1)->ref();
                    const auto id2 = it->ref();
                    if (in_multiple_ways.get(id1) && in_multiple_ways.get(id2)) {
                        segments.emplace_back(id1, id2);
                    }
                }
            }
        }
    }
    reader2.close();

    vout << "Got " << segments.size() << " segments\n";

    vout << "Sorting segments...\n";
    std::sort(segments.begin(), segments.end());

    vout << "Counting segments...\n";
    std::ofstream ids{output_directory + "/ids"};
    counter counts;

    for (auto it = segments.begin(); it != segments.end();) {
        const auto a = std::adjacent_find(it, segments.end());
        if (a == segments.end()) {
            break;
        }
        std::size_t count = 0;
        it = a;
        while (*a == *it) {
            ++count;
            ++it;
            if (it == segments.end()) {
                break;
            }
        }
        counts.increment(count - 1);

        if (count > 10) {
            ids << count << ' ' << it->first << ' ' << it->second << '\n';
        }

        it = a + count;
    }

    int n = 0;
    for (auto cc : counts) {
        std::cout << ++n << ' ' << cc << '\n';
    }

    vout << "Done.\n";
}

