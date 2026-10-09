// Binary save writer and reader, and the state hash. Spec: docs/tier0/specs/06-save-replay.md
// Little-endian, fixed-width fields, written in a fixed order; floats are stored as their bits.
#pragma once
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

namespace zs {

class ArchiveWriter {
 public:
  void u8(std::uint8_t v) { bytes_.push_back(v); }
  void u16(std::uint16_t v) { put(v, 2); }
  void u32(std::uint32_t v) { put(v, 4); }
  void u64(std::uint64_t v) { put(v, 8); }
  void i64(std::int64_t v) { put(static_cast<std::uint64_t>(v), 8); }
  void f64(double v);
  void boolean(bool v) { u8(v ? 1 : 0); }
  void str(const std::string& s);
  const std::vector<std::uint8_t>& bytes() const { return bytes_; }

 private:
  void put(std::uint64_t v, int n) {
    for (int i = 0; i < n; ++i) bytes_.push_back(static_cast<std::uint8_t>(v >> (8 * i)));
  }
  std::vector<std::uint8_t> bytes_;
};

class ArchiveReader {
 public:
  explicit ArchiveReader(const std::vector<std::uint8_t>& bytes) : bytes_(bytes) {}
  std::uint8_t u8() { need(1); return bytes_[pos_++]; }
  std::uint16_t u16() { return static_cast<std::uint16_t>(get(2)); }
  std::uint32_t u32() { return static_cast<std::uint32_t>(get(4)); }
  std::uint64_t u64() { return get(8); }
  std::int64_t i64() { return static_cast<std::int64_t>(get(8)); }
  double f64();
  bool boolean() { return u8() != 0; }
  std::string str();
  bool at_end() const { return pos_ == bytes_.size(); }

  // The save format version being read, set by World::load so a system can read an older layout; 0 = current.
  std::uint32_t format = 0;
  bool format_at_least(std::uint32_t v) const { return format == 0 || format >= v; }

 private:
  void need(std::size_t n) const {
    if (pos_ + n > bytes_.size()) throw std::runtime_error("save file is truncated");
  }
  std::uint64_t get(int n) {
    need(static_cast<std::size_t>(n));
    std::uint64_t v = 0;
    for (int i = 0; i < n; ++i) v |= static_cast<std::uint64_t>(bytes_[pos_++]) << (8 * i);
    return v;
  }
  const std::vector<std::uint8_t>& bytes_;
  std::size_t pos_ = 0;
};

// 64-bit FNV-1a.
std::uint64_t hash_bytes(const std::vector<std::uint8_t>& bytes);

std::vector<std::uint8_t> read_file(const std::string& path);
void write_file(const std::string& path, const std::vector<std::uint8_t>& bytes);

}  // namespace zs
