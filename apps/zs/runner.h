// Runs a command as a child process and keeps its output (spec 55 rule 4): every Try button runs the same executable
// with the same flags a terminal would, so what the button shows is what the flag prints.
#pragma once
#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace zs::atlas {

class Run {
 public:
  Run(std::vector<std::string> argv, std::string cwd);  // argv[0] is the program
  ~Run();
  bool done() const { return done_.load(); }
  int exit_code() const { return exit_code_.load(); }
  std::string output() const;  // everything so far, stdout and stderr interleaved
  const std::vector<std::string>& argv() const { return argv_; }
  std::string command_line() const;
  double seconds() const { return seconds_.load(); }

 private:
  void pump();
  std::vector<std::string> argv_;
  std::string cwd_;
  std::thread thread_;
  mutable std::mutex mutex_;
  std::string out_;
  std::atomic<bool> done_{false};
  std::atomic<int> exit_code_{-1};
  std::atomic<double> seconds_{0};
};

// The program's own path, for running itself.
std::string self_path();

}  // namespace zs::atlas
