// A small Markdown reader for Dear ImGui (spec 55 rule 5): headings, paragraphs, lists, tables, code and links.
// Enough for the repo's docs; anything it does not know is shown as plain text.
#pragma once
#include <functional>
#include <string>
#include <vector>

namespace zs::atlas {

struct MdBlock {
  enum Kind { Heading, Paragraph, Bullet, Numbered, Code, Table, Rule } kind = Paragraph;
  int level = 0;                                   // heading level, or list depth
  std::string text;                                // inline markdown (links kept)
  std::vector<std::vector<std::string>> rows;      // tables: rows[0] is the header
};

std::vector<MdBlock> parse_markdown(const std::string& text);

// Draws the blocks. `on_link` gets the target of a clicked link (as written in the doc).
void draw_markdown(const std::vector<MdBlock>& blocks, const std::function<void(const std::string&)>& on_link);
// One line of inline markdown, wrapped to the available width, links clickable.
void draw_inline(const std::string& text, const std::function<void(const std::string&)>& on_link);
// The text with inline marks removed (for search and titles).
std::string plain_text(const std::string& inline_md);

}  // namespace zs::atlas
