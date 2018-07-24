/*

OSM Data Model Tools

remove-tags

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

#include <osmium/builder/osm_object_builder.hpp>
#include <osmium/io/any_input.hpp>
#include <osmium/io/any_output.hpp>
#include <osmium/tags/tags_filter.hpp>
#include <osmium/visitor.hpp>

#include <clara.hpp>

#include <iostream>
#include <string>
#include <utility>

class RewriteHandler : public osmium::handler::Handler {

    osmium::memory::Buffer* m_buffer;
    const osmium::TagsFilter& m_filter;

    template <typename T>
    void copy_attributes(T& builder, const osmium::OSMObject& object) {
        builder.set_id(object.id())
            .set_version(object.version())
            .set_changeset(object.changeset())
            .set_timestamp(object.timestamp())
            .set_uid(object.uid())
            .set_user(object.user());
    }

    void copy_tags(osmium::builder::Builder* parent, const osmium::TagList& tags) {
        osmium::builder::TagListBuilder builder{*parent};

        for (const auto& tag : tags) {
            if (!m_filter(tag)) {
                builder.add_tag(tag);
            }
        }
    }

public:

    explicit RewriteHandler(osmium::memory::Buffer* buffer, const osmium::TagsFilter& filter) :
        m_buffer(buffer),
        m_filter(filter) {
        assert(buffer);
    }

    void node(const osmium::Node& node) {
        {
            osmium::builder::NodeBuilder builder{*m_buffer};
            copy_attributes(builder, node);
            builder.set_location(node.location());
            copy_tags(&builder, node.tags());
        }
        m_buffer->commit();
    }

    void way(const osmium::Way& way) {
        {
            osmium::builder::WayBuilder builder{*m_buffer};
            copy_attributes(builder, way);
            copy_tags(&builder, way.tags());
            builder.add_item(way.nodes());
        }
        m_buffer->commit();
    }

    void relation(const osmium::Relation& relation) {
        {
            osmium::builder::RelationBuilder builder{*m_buffer};
            copy_attributes(builder, relation);
            copy_tags(&builder, relation.tags());
            builder.add_item(relation.members());
        }
        m_buffer->commit();
    }

}; // class RewriteHandler

int main(int argc, char* argv[]) {
    std::string input_filename;
    std::string output_filename;
    std::string filter_filename;
    bool help = false;

    const auto cli
        = clara::Opt(output_filename, "OUTPUT-FILE")
            ["-o"]["--output"]
            ("output file")
        | clara::Opt(filter_filename, "FILTER-FILE")
            ["-e"]["--expressions"]
            ("filter expressions file")
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
                  << "\nRemove tags matching any expressions in filter file.\n";
        return 0;
    }

    if (input_filename.empty()) {
        std::cerr << "Missing input filename. Try '-h'.\n";
        return 1;
    }

    if (output_filename.empty()) {
        std::cerr << "Missing output filename. Try '-h'.\n";
        return 1;
    }

    if (filter_filename.empty()) {
        std::cerr << "Missing filter filename. Try '-h'.\n";
        return 1;
    }

    const auto filter = load_filter_patterns(filter_filename);

    osmium::io::File input_file{input_filename};
    osmium::io::File output_file{output_filename};

    osmium::io::Reader reader{input_file};
    osmium::io::Writer writer{output_file, osmium::io::overwrite::allow};

    while (const auto buffer = reader.read()) {
        osmium::memory::Buffer output_buffer{buffer.committed(), osmium::memory::Buffer::auto_grow::yes};
        RewriteHandler handler{&output_buffer, filter};
        osmium::apply(buffer, handler);
        writer(std::move(output_buffer));
    }

    writer.close();
    reader.close();

    return 0;
}

