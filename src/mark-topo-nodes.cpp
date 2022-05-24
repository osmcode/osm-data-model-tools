/*

OSM Data Model Tools

mark-topo-nodes

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
#include <osmium/builder/osm_object_builder.hpp>

#include <lyra.hpp>

#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>

int main(int argc, char* argv[]) {
    std::string input_filename;
    std::string output_directory;
    bool help = false;

    const auto cli
        = lyra::opt(output_directory, "DIR")
            ["-o"]["--output-dir"]
            ("output directory")
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
                  << "\nAdd tags to nodes in multiple ways and relations.\n";
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

    osmium::io::Reader reader1{input_file, osmium::osm_entity_bits::way | osmium::osm_entity_bits::relation};
    while (const auto buffer = reader1.read()) {
        for (const auto& object : buffer.select<osmium::OSMObject>()) {
            if (object.type() == osmium::item_type::way) {
                const auto& way = static_cast<const osmium::Way&>(object);
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
            } else {
                for (const auto& member : static_cast<const osmium::Relation&>(object).members()) {
                    if (member.type() == osmium::item_type::node) {
                        in_relation.set(member.positive_ref());
                    }
                }
            }
        }
    }
    reader1.close();

    osmium::memory::Buffer outbuffer{1024};
    osmium::io::Writer writer{output_directory + "/with-marked-topo-nodes.osm.pbf"};

    osmium::io::Reader reader2{input_file};
    while (const auto buffer = reader2.read()) {
        for (const auto& object : buffer.select<osmium::OSMObject>()) {
            if (object.type() == osmium::item_type::node) {
                const bool in_mw = in_multiple_ways.get(object.positive_id());
                const bool in_rel = in_relation.get(object.positive_id());
                if (!object.tags().empty() || (!in_mw && !in_rel)) {
                    writer(object);
                } else {
                    {
                        osmium::builder::NodeBuilder builder{outbuffer};
                        builder.set_location(static_cast<const osmium::Node &>(object).location());
                        builder.set_id(object.id());
                        builder.set_version(object.version());
                        builder.set_timestamp(object.timestamp());
                        builder.set_changeset(object.changeset());
                        builder.set_uid(object.uid());
                        builder.set_user(object.user());
                        if (in_mw) {
                            builder.add_tags({std::make_pair("_in", "ways")});
                        } else if (in_rel) {
                            builder.add_tags({std::make_pair("_in", "rel")});
                        }
                    }
                    outbuffer.commit();
                    writer(*outbuffer.cbegin());
                    outbuffer.clear();
                }
            } else {
                writer(object);
            }
        }
    }

    writer.close();
    reader2.close();
}

