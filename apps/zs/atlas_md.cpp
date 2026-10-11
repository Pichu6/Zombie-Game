#include "atlas_md.h"

#include <imgui.h>

#include <algorithm>
#include <cctype>
#include <cstring>
#include <sstream>

namespace zs::atlas {

namespace {

bool starts_with(const std::string& s, const char* p) { return s.rfind(p, 0) == 0; }

std::string trim(const std::string& s) {
  std::size_t a = 0, b = s.size();
  while (a < b && std::isspace(static_cast<unsigned char>(s[a]))) ++a;
  while (b > a && std::isspace(static_cast<unsigned char>(s[b - 1]))) --b;
  return s.substr(a, b - a);
}

// "| a | b |" -> {"a", "b"}, with escaped pipes kept.
std::vector<std::string> split_row(const std::string& line) {
  std::vector<std::string> cells;
  std::string cur;
  const std::string t = trim(line);
  for (std::size_t i = 0; i < t.size(); ++i) {
    if (t[i] == '\\' && i + 1 < t.size() && t[i + 1] == '|') {
      cur += '|';
      ++i;
    } else if (t[i] == '|') {
      cells.push_back(trim(cur));
      cur.clear();
    } else cur += t[i];
  }
  cells.push_back(trim(cur));
  if (!cells.empty() && cells.front().empty()) cells.erase(cells.begin());
  if (!cells.empty() && cells.back().empty()) cells.pop_back();
  return cells;
}

bool separator_row(const std::string& line) {
  const std::string t = trim(line);
  if (t.size() < 3 || t[0] != '|') return false;
  for (char c : t)
    if (!(c == '|' || c == '-' || c == ':' || c == ' ')) return false;
  return true;
}

// Inline pieces: text, emphasis, code, links.
struct Piece {
  std::string text;
  std::string link;  // target, when the piece is a link
  bool code = false, bold = false;
};

std::vector<Piece> pieces_of(const std::string& s) {
  std::vector<Piece> out;
  Piece cur;
  auto flush = [&] {
    if (!cur.text.empty()) out.push_back(cur);
    cur = Piece{};
  };
  for (std::size_t i = 0; i < s.size();) {
    if (s[i] == '`') {
      const auto end = s.find('`', i + 1);
      if (end != std::string::npos) {
        flush();
        Piece p;
        p.text = s.substr(i + 1, end - i - 1);
        p.code = true;
        out.push_back(p);
        i = end + 1;
        continue;
      }
    }
    if (s[i] == '[') {
      const auto close = s.find(']', i);
      if (close != std::string::npos && close + 1 < s.size() && s[close + 1] == '(') {
        const auto paren = s.find(')', close + 2);
        if (paren != std::string::npos) {
          flush();
          Piece p;
          p.text = s.substr(i + 1, close - i - 1);
          p.link = s.substr(close + 2, paren - close - 2);
          out.push_back(p);
          i = paren + 1;
          continue;
        }
      }
    }
    if (s.compare(i, 2, "**") == 0) {
      flush();
      cur.bold = !cur.bold;
      // The next pieces carry the bold flag until the closing mark.
      const auto end = s.find("**", i + 2);
      if (end != std::string::npos) {
        Piece p;
        p.text = s.substr(i + 2, end - i - 2);
        p.bold = true;
        // Links inside bold are rare; keep them plain.
        out.push_back(p);
        i = end + 2;
        cur.bold = false;
        continue;
      }
      i += 2;
      continue;
    }
    if (s[i] == '*' || s[i] == '_') {  // emphasis marks: dropped
      const bool word_char = i + 1 < s.size() && !std::isspace(static_cast<unsigned char>(s[i + 1]));
      const bool prev_word = i > 0 && !std::isspace(static_cast<unsigned char>(s[i - 1]));
      if (s[i] == '*' || (!prev_word || !word_char)) {
        ++i;
        continue;
      }
    }
    cur.text += s[i];
    ++i;
  }
  flush();
  return out;
}

void draw_word(const char* begin, const char* end, bool link, bool code, bool bold, const std::string& target,
               const std::function<void(const std::string&)>& on_link) {
  const ImVec2 size = ImGui::CalcTextSize(begin, end);
  const float avail = ImGui::GetContentRegionAvail().x;
  if (size.x > avail && ImGui::GetCursorPosX() > ImGui::GetWindowContentRegionMin().x + 1) ImGui::NewLine();
  if (code) {
    ImDrawList* d = ImGui::GetWindowDrawList();
    const ImVec2 p = ImGui::GetCursorScreenPos();
    d->AddRectFilled(ImVec2(p.x - 2, p.y), ImVec2(p.x + size.x + 2, p.y + size.y), IM_COL32(60, 60, 70, 255), 2.0f);
    ImGui::TextColored(ImVec4(0.85f, 0.9f, 0.75f, 1), "%.*s", static_cast<int>(end - begin), begin);
  } else if (link) {
    ImGui::TextColored(ImVec4(0.45f, 0.7f, 1.0f, 1), "%.*s", static_cast<int>(end - begin), begin);
    if (ImGui::IsItemHovered()) {
      ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
      ImGui::SetTooltip("%s", target.c_str());
    }
    if (ImGui::IsItemClicked()) on_link(target);
  } else if (bold) {
    ImGui::TextColored(ImVec4(1, 1, 1, 1), "%.*s", static_cast<int>(end - begin), begin);
  } else {
    ImGui::TextUnformatted(begin, end);
  }
}

}  // namespace

std::string plain_text(const std::string& s) {
  std::string out;
  for (const Piece& p : pieces_of(s)) out += p.text;
  return out;
}

void draw_inline(const std::string& text, const std::function<void(const std::string&)>& on_link) {
  const float space = ImGui::CalcTextSize(" ").x;
  bool first = true, gap_before = false;  // gap_before: the previous piece ended in a space
  for (const Piece& p : pieces_of(text)) {
    // Words, so the line wraps at the available width; a link or code span is drawn word by word too and keeps its
    // behaviour on every word. Two pieces with no space between them ("(spec", a link, ")") stay glued.
    const char* s = p.text.c_str();
    const char* e = s + p.text.size();
    const char* w = s;
    bool piece_first = true;
    while (w < e) {
      const bool leading_space = *w == ' ';
      while (w < e && *w == ' ') ++w;
      const char* we = w;
      while (we < e && *we != ' ') ++we;
      if (we == w) break;
      if (!first) ImGui::SameLine(0, (piece_first && !leading_space && !gap_before) ? 0.0f : space);
      draw_word(w, we, !p.link.empty(), p.code, p.bold, p.link, on_link);
      first = false;
      piece_first = false;
      w = we;
    }
    gap_before = !p.text.empty() && p.text.back() == ' ';
  }
  if (first) ImGui::NewLine();
}

std::vector<MdBlock> parse_markdown(const std::string& text) {
  std::vector<MdBlock> blocks;
  std::istringstream in(text);
  std::string line, para;
  bool in_code = false;
  MdBlock code;
  auto flush_para = [&] {
    if (!para.empty()) {
      MdBlock b;
      b.kind = MdBlock::Paragraph;
      b.text = para;
      blocks.push_back(b);
      para.clear();
    }
  };
  std::vector<std::string> raw_lines;
  while (std::getline(in, line)) {
    if (!line.empty() && line.back() == '\r') line.pop_back();
    raw_lines.push_back(line);
  }
  for (std::size_t i = 0; i < raw_lines.size(); ++i) {
    line = raw_lines[i];
    if (in_code) {
      if (starts_with(trim(line), "```")) {
        blocks.push_back(code);
        in_code = false;
      } else code.text += line + "\n";
      continue;
    }
    const std::string t = trim(line);
    if (starts_with(t, "```")) {
      flush_para();
      in_code = true;
      code = MdBlock{};
      code.kind = MdBlock::Code;
      continue;
    }
    if (t.empty()) {
      flush_para();
      continue;
    }
    if (t[0] == '#') {
      flush_para();
      MdBlock b;
      b.kind = MdBlock::Heading;
      while (b.level < static_cast<int>(t.size()) && t[static_cast<std::size_t>(b.level)] == '#') ++b.level;
      b.text = trim(t.substr(static_cast<std::size_t>(b.level)));
      blocks.push_back(b);
      continue;
    }
    if (t == "---" || t == "***") {
      flush_para();
      MdBlock b;
      b.kind = MdBlock::Rule;
      blocks.push_back(b);
      continue;
    }
    if (t[0] == '|') {
      flush_para();
      MdBlock b;
      b.kind = MdBlock::Table;
      while (i < raw_lines.size() && !trim(raw_lines[i]).empty() && trim(raw_lines[i])[0] == '|') {
        if (!separator_row(raw_lines[i])) b.rows.push_back(split_row(raw_lines[i]));
        ++i;
      }
      --i;
      blocks.push_back(b);
      continue;
    }
    // Lists: "- ", "* ", "1. ", with two-space indents.
    std::size_t indent = 0;
    while (indent < line.size() && line[indent] == ' ') ++indent;
    const bool bullet = starts_with(t, "- ") || starts_with(t, "* ");
    std::size_t digits = 0;
    while (digits < t.size() && std::isdigit(static_cast<unsigned char>(t[digits]))) ++digits;
    const bool numbered = digits > 0 && digits + 1 < t.size() && t[digits] == '.' && t[digits + 1] == ' ';
    if (bullet || numbered) {
      flush_para();
      MdBlock b;
      b.kind = bullet ? MdBlock::Bullet : MdBlock::Numbered;
      b.level = static_cast<int>(indent / 2);
      b.text = bullet ? trim(t.substr(2)) : t.substr(0, digits) + ". " + trim(t.substr(digits + 2));
      blocks.push_back(b);
      continue;
    }
    // A continuation of the previous list item (indented text under it).
    if (indent >= 2 && !blocks.empty() && (blocks.back().kind == MdBlock::Bullet || blocks.back().kind == MdBlock::Numbered) &&
        para.empty()) {
      blocks.back().text += " " + t;
      continue;
    }
    para += (para.empty() ? "" : " ") + t;
  }
  if (in_code) blocks.push_back(code);
  flush_para();
  return blocks;
}

void draw_markdown(const std::vector<MdBlock>& blocks, const std::function<void(const std::string&)>& on_link) {
  int table_id = 0;
  for (const MdBlock& b : blocks) {
    switch (b.kind) {
      case MdBlock::Heading: {
        ImGui::Spacing();
        if (b.level <= 1) ImGui::PushFont(nullptr, ImGui::GetFontSize() * 1.5f);
        else if (b.level == 2) ImGui::PushFont(nullptr, ImGui::GetFontSize() * 1.25f);
        else ImGui::PushFont(nullptr, ImGui::GetFontSize() * 1.1f);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 0.95f, 0.8f, 1));
        draw_inline(b.text, on_link);
        ImGui::PopStyleColor();
        ImGui::PopFont();
        if (b.level <= 2) ImGui::Separator();
        ImGui::Spacing();
        break;
      }
      case MdBlock::Paragraph:
        draw_inline(b.text, on_link);
        ImGui::Spacing();
        break;
      case MdBlock::Bullet:
      case MdBlock::Numbered: {
        ImGui::Indent(12.0f * (b.level + 1));
        if (b.kind == MdBlock::Bullet) {
          ImGui::Bullet();
          ImGui::SameLine();
        }
        draw_inline(b.text, on_link);
        ImGui::Unindent(12.0f * (b.level + 1));
        break;
      }
      case MdBlock::Code: {
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.12f, 0.12f, 0.15f, 1));
        const float h = ImGui::GetTextLineHeightWithSpacing() * (1 + std::count(b.text.begin(), b.text.end(), '\n')) + 12;
        ImGui::BeginChild(("code" + std::to_string(table_id++)).c_str(), ImVec2(0, h), ImGuiChildFlags_Borders,
                          ImGuiWindowFlags_HorizontalScrollbar);
        ImGui::TextUnformatted(b.text.c_str());
        ImGui::EndChild();
        ImGui::PopStyleColor();
        ImGui::Spacing();
        break;
      }
      case MdBlock::Table: {
        if (b.rows.empty()) break;
        std::size_t cols = 0;
        for (const auto& r : b.rows) cols = std::max(cols, r.size());
        if (cols == 0) break;
        const ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp;
        if (ImGui::BeginTable(("t" + std::to_string(table_id++)).c_str(), static_cast<int>(cols), flags)) {
          for (std::size_t r = 0; r < b.rows.size(); ++r) {
            ImGui::TableNextRow(r == 0 ? ImGuiTableRowFlags_Headers : 0);
            for (std::size_t c = 0; c < cols; ++c) {
              ImGui::TableSetColumnIndex(static_cast<int>(c));
              if (c < b.rows[r].size()) {
                ImGui::PushID(static_cast<int>(r * cols + c));
                draw_inline(b.rows[r][c], on_link);
                ImGui::PopID();
              }
            }
          }
          ImGui::EndTable();
        }
        ImGui::Spacing();
        break;
      }
      case MdBlock::Rule:
        ImGui::Separator();
        break;
    }
  }
}

}  // namespace zs::atlas
