#include <algorithm>
#include <array>
#include <deque>
#include <exception>
#include <format>
#include <ios>
#include <iostream>
#include <iterator>
#include <optional>
#include <print>
#include <ranges>
#include <span>
#include <stack>
#include <stdexcept>
#include <string>
#include <string_view>

#include <strings.h>
#include <vector>

#include <cctype>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "libmedia/mpeg4.hh"
#include "libmedia/mpeg4/dump.hh"

#include "file_view.hh"

struct BoxWalker
{
    BoxWalker(std::span<const std::byte> data) : m_data(data) {};

    bool has_box() const
    {
        if (m_data.empty()) {
            return false;
        }

        auto output = Mpeg4::BoxView(m_data);

        auto header = output.get_header();

        if (!header.has_value()) {
            return false;
        }

        auto data = output.get_content_data();
        if (!data.has_value()) {
            return false;
        }

        return true;
    }

    std::optional<Mpeg4::BoxView> current_box() const
    {
        if (!has_box()) {
            return std::nullopt;
        }
        return Mpeg4::BoxView(m_data);
    }

    void select_next_box()
    {
        auto current = current_box();
        if (!current) {
            return;
        }
        auto header = current.value().get_header();
        if (!header) {
            m_data = {};
            return;
        }

        auto box_size = header.value().box_content_size;

        size_t offset = 0;
        offset += header.value().header_size;
        offset += box_size.value_or(0);
        if (m_data.size() <= offset) {
            m_data = {};
            return;
        }

        m_data = m_data.subspan(offset, m_data.size() - offset);
    }

  private:
    std::span<const std::byte> m_data;
};

int main(int argc, char **argv)
try {
    if (argc < 2) {
        std::cerr << "No args!\n";
        return EXIT_FAILURE;
    }

    FileView f{argv[1]};
    auto boxes_data =
        std::span(reinterpret_cast<const std::byte *>(f.data()), f.size());

    std::stack<BoxWalker> walker_stack;
    walker_stack.push(BoxWalker(boxes_data));

    std::vector<bool> is_last_stack;

    while (!walker_stack.empty()) {

        BoxWalker &top = walker_stack.top();

        auto current_box_op = top.current_box();
        if (!current_box_op) {
            walker_stack.pop();
            continue;
        }
        auto current_box = current_box_op.value();
        top.select_next_box();

        auto log_box_indented =
            [&is_last_stack](size_t indent, Mpeg4::BoxView box, bool is_last) {
                std::string indent_str;

                if (is_last_stack.size() < indent + 1) {
                    is_last_stack.resize(indent + 1);
                }

                is_last_stack[indent] = is_last;

                if (indent != 0) {
                    for (size_t indent_level = 1; indent_level < indent;
                         indent_level++) {
                        if (is_last_stack[indent_level]) {
                            indent_str += " ";
                        } else {
                            indent_str += "│";
                        }
                    }
                    if (is_last) {
                        indent_str += "└";
                    } else {
                        indent_str += "├";
                    }
                }

                auto a = box.get_header();
                auto t = a.value().type;

                std::string t_str;

                std::ranges::for_each(t.data, [&t_str](char c) {
                    if (std::isprint(c)) {
                        t_str.push_back(c);
                        return;
                    }
                    t_str.push_back('?');
                });

                std::cout << std::format("{}{:s}\n", indent_str, t_str);
            };

        log_box_indented(walker_stack.size(), current_box, !top.has_box());

        auto data = current_box.get_content_data();

        if (data) {
            BoxWalker child_walker{data.value()};
            if (child_walker.has_box()) {
                walker_stack.push(child_walker);
            }
        }
    }

} catch (std::exception &e) {
    std::cout << std::format("Exception \"{}\"\n", e.what());
    return EXIT_FAILURE;
}
