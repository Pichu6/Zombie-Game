#include "zs/archive.h"

#include <bit>
#include <filesystem>
#include <fstream>
#include <iterator>

namespace zs {

void ArchiveWriter::f64(double v) { u64(std::bit_cast<std::uint64_t>(v)); }

void ArchiveWriter::str(const std::string& s) {
  u32(static_cast<std::uint32_t>(s.size()));
  bytes_.insert(bytes_.end(), s.begin(), s.end());
}

double ArchiveReader::f64() { return std::bit_cast<double>(u64()); }

std::string ArchiveReader::str() {
  const std::uint32_t n = u32();
  need(n);
  std::string s(reinterpret_cast<const char*>(bytes_.data() + pos_), n);
  pos_ += n;
  return s;
}

std::uint64_t hash_bytes(const std::vector<std::uint8_t>& bytes) {
  std::uint64_t h = 0xcbf29ce484222325ULL;
  for (std::uint8_t b : bytes) {
    h ^= b;
    h *= 0x100000001b3ULL;
  }
  return h;
}

std::vector<std::uint8_t> read_file(const std::string& path) {
  std::ifstream in(path, std::ios::binary);
  if (!in) throw std::runtime_error("cannot open " + path);
  return {std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>()};
}

void write_file(const std::string& path, const std::vector<std::uint8_t>& bytes) {
  // Write to a temporary file, read it back, then rename: a crash never leaves a half-written save.
  const std::string tmp = path + ".tmp";
  {
    std::ofstream out(tmp, std::ios::binary | std::ios::trunc);
    if (!out) throw std::runtime_error("cannot write " + tmp);
    out.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    if (!out) throw std::runtime_error("write failed: " + tmp);
  }
  if (read_file(tmp) != bytes) throw std::runtime_error("save check failed: " + tmp);
  std::error_code ec;
  std::filesystem::rename(tmp, path, ec);
  if (ec) throw std::runtime_error("cannot replace " + path + ": " + ec.message());
}

}  // namespace zs
