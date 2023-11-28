#include <glm/fwd.hpp>
#include <glm/gtx/color_space.hpp>
#include <iostream>
#include <string>
#include <vector>

class Hue_ring {
public:
  Hue_ring(glm::vec3 base_hsv = {0, 0.7, 0.7}, float hue_step = 15)
      : base_(base_hsv), current_(base_), step_(hue_step) {}
  glm::vec3 next_rgb() {
    next();
    return glm::rgbColor(current_);
  }
  float step() const { return step_; }

private:
  void next() {
    current_.x += step_;
    if (current_.x >= 360) {
      current_.x -= 360;
    }
  }
  glm::vec3 base_;
  glm::vec3 current_;

  float step_;
};
#include <fmt/core.h>
#include <glm/glm.hpp>

// Custom formatter for glm::vec3
template <> struct fmt::formatter<glm::vec3> {
  // Parses format specifications of the form ['f' | 'e']
  constexpr auto parse(format_parse_context &ctx) -> decltype(ctx.begin()) {
    // [ctx.begin(), ctx.end()) is a character range that contains a part of
    // the format specification (excluding braces).
    auto it = ctx.begin(), end = ctx.end();

    // Check if reached the end of the range:
    if (it != end && (*it == 'f' || *it == 'e')) {
      // Advance a character, as we've found a valid format specifier
      it++;
    }

    // Return an iterator past the end of the parsed range:
    return it;
  }

  // Formats the point p using the parsed format specification (either 'f' or
  // 'e').
  template <typename FormatContext>
  auto format(const glm::vec3 &vec, FormatContext &ctx) -> decltype(ctx.out()) {
    // ctx.out() is an output iterator to write to.
    return format_to(ctx.out(), "[{:.2f}, {:.2f}, {:.2f}]", vec.x, vec.y,
                     vec.z);
  }
};
class Text_table {
public:
  std::vector<std::string> headers_;
  std::vector<std::vector<std::string>> table_;

  int column() { return headers_.size(); };

  void add(std::string str) {
    if (table_.empty()) {
      table_.push_back({str});
      return;
    }
    if (table_.back().size() == column()) {
      table_.push_back({});
    }
    table_.back().push_back(str);
  }
};
std::string render_to_html(const Text_table &table) {
  std::string html;
  html += "<table>\n";

  // Add table headers
  html += "  <tr>\n";
  for (const std::string &header : table.headers_) {
    html += "    <th>" + header + "</th>\n";
  }
  html += "  </tr>\n";

  // Add table rows
  for (const std::vector<std::string> &row : table.table_) {
    html += "  <tr>\n";
    for (const std::string &cell : row) {
      html += "    <td>" + cell + "</td>\n";
    }
    html += "  </tr>\n";
  }

  html += "</table>\n";
  return html;
}
std::string css = R"**(
      <style>
        /* Style for the table */
        table {
            border-collapse: collapse;
            width: 100%;
            margin-bottom: 20px;
            font-family: Arial, sans-serif;
        }

        /* Style for table headers */
        th {
            background-color: #f2f2f2;
            text-align: left;
            padding: 10px;
            border-top: 1px solid #ddd;
            border-bottom: 1px solid #ddd;
        }

        /* Style for table cells */
        td {
            padding: 10px;
            border-top: 1px solid #ddd;
            border-bottom: 1px solid #ddd;
        }

        /* Style for alternating rows */
        tr:nth-child(even) {
            background-color: #f2f2f2;
        }

        /* Hover effect for rows */
        tr:hover {
            background-color: #e0e0e0;
        }
    </style>
)**";
int main() {
  Hue_ring ring{};

  // Create a Text_table instance with sample data
  Text_table table;
  table.headers_ = {"Header1", "Header2", "Header3"};
  table.table_ = {{"Row1-1", "Row1-2", "Row1-3"},
                  {"Row2-1", "Row2-2", "Row2-3"},
                  {"Row3-1", "Row3-2", "Row3-3"}};

  // Render the table to HTML
  for (int i = 0; i < (360 / ring.step()); i++) {
    auto color = ring.next_rgb();
    auto str = fmt::format("<p style=\"color:rgb({},{},{});\">This is a "
                           "paragraph with {} color.</p>",
                           color.r * 255.99, color.g * 255.99, color.b * 255.99,
                           color);
    table.add(str);
  }
  std::string htmlTable = render_to_html(table);

  std::cout << css;
  // Print the generated HTML
  std::cout << htmlTable << std::endl;

  return 0;
}
