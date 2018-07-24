/*

OSM Data Model Tools

way-nodes

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

#include <osmium/index/id_set.hpp>
#include <osmium/io/any_input.hpp>
#include <osmium/io/any_output.hpp>

#include <clara.hpp>

#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>

static std::string percent(std::uint64_t fraction, std::uint64_t all, const char* text) noexcept {
    const auto p = fraction * 100 / all;
    return " (" + std::to_string(p) + "% of " + text + ")";
}

int main(int argc, char* argv[]) {
    std::string input_filename;
    std::string output_directory;
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
                  << "\nCreate statistics on way nodes.\n";
        return 0;
    }

    if (input_filename.empty()) {
        std::cerr << "Missing input filename. Try '-h'.\n";
        return 1;
    }

    osmium::index::IdSetDense<osmium::unsigned_object_id_type> in_way;
    osmium::index::IdSetDense<osmium::unsigned_object_id_type> in_multiple_ways;
    osmium::index::IdSetDense<osmium::unsigned_object_id_type> in_relation;

    osmium::io::File input_file{input_filename};

    std::uint64_t count_ways = 0;
    std::uint64_t count_relations = 0;

    osmium::io::Reader reader1{input_file, osmium::osm_entity_bits::way | osmium::osm_entity_bits::relation};
    while (const auto buffer = reader1.read()) {
        for (const auto& object : buffer.select<osmium::OSMObject>()) {
            if (object.type() == osmium::item_type::way) {
                ++count_ways;
                const auto& way = static_cast<const osmium::Way&>(object);
                if (way.nodes().empty()) {
                    continue;
                }
                auto it = way.nodes().begin();
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
            } else {
                ++count_relations;
                for (const auto& member : static_cast<const osmium::Relation&>(object).members()) {
                    if (member.type() == osmium::item_type::node) {
                        in_relation.set(member.positive_ref());
                    }
                }
            }
        }
    }
    reader1.close();

    std::uint64_t count_nodes = 0;
    std::uint64_t count_nodes_with_tags = 0;
    std::uint64_t count_nodes_in_way = 0;
    std::uint64_t count_nodes_with_tags_in_way = 0;
    std::uint64_t count_nodes_in_multiple_ways = 0;
    std::uint64_t count_nodes_in_relation = 0;

    std::unique_ptr<osmium::io::Writer> writer_nodes_with_tags_in_way;
    if (!output_directory.empty()) {
        writer_nodes_with_tags_in_way = std::make_unique<osmium::io::Writer>(output_directory + "/nodes_with_tags_in_way.osm.pbf");
    }

    osmium::io::Reader reader2{input_file, osmium::osm_entity_bits::node};
    while (const auto buffer = reader2.read()) {
        for (const auto& node : buffer.select<osmium::Node>()) {
            ++count_nodes;
            if (!node.tags().empty()) {
                ++count_nodes_with_tags;
            }
            if (in_way.get(node.positive_id())) {
                ++count_nodes_in_way;
                if (!node.tags().empty()) {
                    ++count_nodes_with_tags_in_way;
                    if (writer_nodes_with_tags_in_way) {
                        (*writer_nodes_with_tags_in_way)(node);
                    }
                }
                if (in_multiple_ways.get(node.positive_id())) {
                    ++count_nodes_in_multiple_ways;
                }
            }
            if (in_relation.get(node.positive_id())) {
                ++count_nodes_in_relation;
            }
        }
    }

    if (writer_nodes_with_tags_in_way) {
        writer_nodes_with_tags_in_way->close();
    }
    reader2.close();

    std::cout << "nodes: " << count_nodes
              << "\nways: " << count_ways
              << "\nrelations: " << count_relations
              << "\nnodes with tags: " << count_nodes_with_tags << percent(count_nodes_with_tags, count_nodes, "all nodes")
              << "\nnodes in way: " << count_nodes_in_way << percent(count_nodes_in_way, count_nodes, "all nodes")
              << "\nnodes with tags in way: " << count_nodes_with_tags_in_way << percent(count_nodes_with_tags_in_way, count_nodes, "all nodes") << percent(count_nodes_with_tags_in_way, count_nodes_with_tags, "tagged nodes")
              << "\nnodes in multiple ways: " << count_nodes_in_multiple_ways << percent(count_nodes_in_multiple_ways, count_nodes, "all nodes")
              << "\nnodes in relation: " << count_nodes_in_relation << percent(count_nodes_in_relation, count_nodes, "all nodes")
              << '\n';
}

