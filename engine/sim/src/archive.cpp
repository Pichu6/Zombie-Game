#include "zs/archive.h"

#include <bit>
#include <cassert>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iterator>

#ifndef _WIN32
#include <fcntl.h>
#include <unistd.h>
#endif

namespace zs {

// A NaN in saved state is a bug: x86 and ARM write different NaN bits, so it would hash differently (spec 06 rule 19).
void ArchiveWriter::f64(double v) {
  assert(v == v && "NaN in saved state");
  u64(std::bit_cast<std::uint64_t>(v));
}

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

namespace {

// Opened with the CRT's commit flag on Windows ("c": fflush writes through to the disk), plain elsewhere.
#ifdef _WIN32
constexpr const char* kWriteMode = "wbc";
#else
constexpr const char* kWriteMode = "wb";
#endif

// Pushes a written file's bytes past the OS cache to the disk (spec 06 rules 13, 15).
bool sync_file(std::FILE* f) {
  if (std::fflush(f) != 0) return false;
#ifdef _WIN32
  return true;  // the commit flag made the fflush durable
#else
  return fsync(fileno(f)) == 0;
#endif
}

// On POSIX a rename is durable only once its directory is synced; Windows has no such step.
void sync_dir(const std::filesystem::path& file) {
#ifndef _WIN32
  const std::filesystem::path dir = file.has_parent_path() ? file.parent_path() : std::filesystem::path(".");
  const int fd = open(dir.c_str(), O_RDONLY);
  if (fd >= 0) {
    fsync(fd);
    close(fd);
  }
#else
  (void)file;
#endif
}

}  // namespace

void write_file(const std::string& path, const std::vector<std::uint8_t>& bytes) {
  // Write to a temporary file, sync it to disk, read it back, then rename and sync the directory: a crash or a
  // full disk never leaves a half-written save in place of the old one.
  const std::string tmp = path + ".tmp";
  {
    std::FILE* out = std::fopen(tmp.c_str(), kWriteMode);
    if (!out) throw std::runtime_error("cannot write " + tmp);
    const bool ok = std::fwrite(bytes.data(), 1, bytes.size(), out) == bytes.size() && sync_file(out);
    if (std::fclose(out) != 0 || !ok) throw std::runtime_error("write failed: " + tmp);
  }
  if (read_file(tmp) != bytes) throw std::runtime_error("save check failed: " + tmp);
  std::error_code ec;
  std::filesystem::rename(tmp, path, ec);
  if (ec) throw std::runtime_error("cannot replace " + path + ": " + ec.message());
  sync_dir(path);
}

}  // namespace zs
