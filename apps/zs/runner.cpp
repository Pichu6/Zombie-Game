#include "runner.h"

#include <chrono>
#include <cstdio>
#include <cstring>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#else
#include <sys/wait.h>
#include <unistd.h>
#endif

#include "worlds.h"

namespace zs::atlas {

namespace {

#ifdef _WIN32
std::wstring widen(const std::string& s) {
  if (s.empty()) return {};
  const int n = MultiByteToWideChar(CP_UTF8, 0, s.data(), static_cast<int>(s.size()), nullptr, 0);
  std::wstring w(static_cast<std::size_t>(n), 0);
  MultiByteToWideChar(CP_UTF8, 0, s.data(), static_cast<int>(s.size()), w.data(), n);
  return w;
}

// One argument quoted the way CommandLineToArgv reads it back.
std::string quote_arg(const std::string& a) {
  if (!a.empty() && a.find_first_of(" \t\"") == std::string::npos) return a;
  std::string out = "\"";
  unsigned backslashes = 0;
  for (char c : a) {
    if (c == '\\') { ++backslashes; continue; }
    if (c == '"') {
      out.append(backslashes * 2 + 1, '\\');
      out += '"';
      backslashes = 0;
      continue;
    }
    out.append(backslashes, '\\');
    backslashes = 0;
    out += c;
  }
  out.append(backslashes * 2, '\\');
  return out + "\"";
}
#endif

}  // namespace

std::string self_path() {
#ifdef _WIN32
  wchar_t buf[MAX_PATH * 2];
  const DWORD n = GetModuleFileNameW(nullptr, buf, static_cast<DWORD>(std::size(buf)));
  if (n > 0 && n < std::size(buf)) {
    const int m = WideCharToMultiByte(CP_UTF8, 0, buf, static_cast<int>(n), nullptr, 0, nullptr, nullptr);
    std::string s(static_cast<std::size_t>(m), 0);
    WideCharToMultiByte(CP_UTF8, 0, buf, static_cast<int>(n), s.data(), m, nullptr, nullptr);
    return s;
  }
  return "zs.exe";
#else
  return (worlds::exe_dir() / "zs").string();
#endif
}

std::string Run::command_line() const {
  std::string s;
  for (std::size_t i = 0; i < argv_.size(); ++i) {
    if (i) s += ' ';
#ifdef _WIN32
    s += quote_arg(argv_[i]);
#else
    s += argv_[i].find(' ') == std::string::npos ? argv_[i] : "'" + argv_[i] + "'";
#endif
  }
  return s;
}

Run::Run(std::vector<std::string> argv, std::string cwd) : argv_(std::move(argv)), cwd_(std::move(cwd)) {
  thread_ = std::thread([this] { pump(); });
}

Run::~Run() {
  if (thread_.joinable()) thread_.join();
}

std::string Run::output() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return out_;
}

void Run::pump() {
  const auto t0 = std::chrono::steady_clock::now();
  auto append = [&](const char* data, std::size_t n) {
    std::lock_guard<std::mutex> lock(mutex_);
    out_.append(data, n);
  };
#ifdef _WIN32
  SECURITY_ATTRIBUTES sa{};
  sa.nLength = sizeof sa;
  sa.bInheritHandle = TRUE;
  HANDLE read = nullptr, write = nullptr;
  if (!CreatePipe(&read, &write, &sa, 0)) {
    append("could not open a pipe\n", 22);
    done_ = true;
    return;
  }
  SetHandleInformation(read, HANDLE_FLAG_INHERIT, 0);
  STARTUPINFOW si{};
  si.cb = sizeof si;
  si.dwFlags = STARTF_USESTDHANDLES;
  si.hStdOutput = write;
  si.hStdError = write;
  si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
  PROCESS_INFORMATION pi{};
  std::wstring cmd = widen(command_line());
  const std::wstring wcwd = widen(cwd_);
  const BOOL ok = CreateProcessW(nullptr, cmd.data(), nullptr, nullptr, TRUE, CREATE_NO_WINDOW, nullptr,
                                 wcwd.empty() ? nullptr : wcwd.c_str(), &si, &pi);
  CloseHandle(write);
  if (!ok) {
    const std::string msg = "could not start " + argv_[0] + " (error " + std::to_string(GetLastError()) + ")\n";
    append(msg.data(), msg.size());
    CloseHandle(read);
    done_ = true;
    return;
  }
  char buf[4096];
  DWORD n = 0;
  while (ReadFile(read, buf, sizeof buf, &n, nullptr) && n > 0) {
    append(buf, n);
    seconds_ = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
  }
  CloseHandle(read);
  WaitForSingleObject(pi.hProcess, INFINITE);
  DWORD code = 0;
  GetExitCodeProcess(pi.hProcess, &code);
  exit_code_ = static_cast<int>(code);
  CloseHandle(pi.hProcess);
  CloseHandle(pi.hThread);
#else
  int fds[2];
  if (pipe(fds) != 0) {
    append("could not open a pipe\n", 22);
    done_ = true;
    return;
  }
  const pid_t pid = fork();
  if (pid == 0) {
    dup2(fds[1], 1);
    dup2(fds[1], 2);
    close(fds[0]);
    close(fds[1]);
    if (!cwd_.empty() && chdir(cwd_.c_str()) != 0) _exit(126);
    std::vector<char*> args;
    for (auto& a : argv_) args.push_back(a.data());
    args.push_back(nullptr);
    execvp(args[0], args.data());
    _exit(127);
  }
  close(fds[1]);
  char buf[4096];
  ssize_t n;
  while ((n = ::read(fds[0], buf, sizeof buf)) > 0) {
    append(buf, static_cast<std::size_t>(n));
    seconds_ = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
  }
  close(fds[0]);
  int status = 0;
  waitpid(pid, &status, 0);
  exit_code_ = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
#endif
  seconds_ = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
  done_ = true;
}

}  // namespace zs::atlas
