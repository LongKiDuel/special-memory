#pragma once
#include "spdlog/sinks/base_sink.h"
#include <memory>
#include <string>

template <typename Mutex>
class Imgui_sink : public spdlog::sinks::base_sink<Mutex> {
protected:
  void sink_it_(const spdlog::details::log_msg &msg) override {

    // log_msg is a struct containing the log entry info like level, timestamp,
    // thread id etc. msg.raw contains pre formatted log

    // If needed (very likely but not mandatory), the sink formats the message
    // before sending it to its final destination:
    spdlog::memory_buf_t formatted;
    spdlog::sinks::base_sink<Mutex>::formatter_->format(msg, formatted);
    *log_buffer_ += fmt::to_string(formatted);
  }

  void flush_() override {}

public:
  std::shared_ptr<std::string> log_buffer_ = std::make_shared<std::string>();
};

#include "spdlog/details/null_mutex.h"
#include <mutex>

using Imgui_sink_mt = Imgui_sink<std::mutex>;
using Imgui_sink_st = Imgui_sink<spdlog::details::null_mutex>;

#include <imgui.h>
#include <spdlog/spdlog.h>
class Imgui_log_window {
public:
  Imgui_log_window() {
    auto sink = std::make_shared<Imgui_sink_st>();
    buffer_ = sink->log_buffer_;
    spdlog::default_logger()->sinks().push_back(sink);
  }

  void draw() {
    if (ImGui::Begin("Logs")) {
      ImGui::Text("%s", buffer_->c_str());
    }
    ImGui::End();
  }

private:
  std::shared_ptr<std::string> buffer_;
};