#pragma once
#include "spdlog/sinks/base_sink.h"
#include <chrono>
#include <cstddef>
#include <memory>
#include <spdlog/common.h>
#include <spdlog/details/log_msg.h>
#include <spdlog/formatter.h>
#include <string>
#include <string_view>
#include <vector>

struct Log_packet {
  std::string text_;
  // source
  std::string_view file_;
  std::string_view function_name_;
  int line_{};
  // log info
  std::string_view logger_;
  spdlog::log_clock::time_point time_;
  size_t thread_id_;
  int level_{};
};
template <typename Mutex>
class Imgui_sink : public spdlog::sinks::base_sink<Mutex> {
protected:
  void sink_it_(const spdlog::details::log_msg &msg) override {
    spdlog::memory_buf_t formatted;
    spdlog::sinks::base_sink<Mutex>::formatter_->format(msg, formatted);

    auto str = fmt::to_string(formatted);
    *log_buffer_ += str;
    Log_packet pack;
    pack.file_ = msg.source.filename;
    pack.function_name_ = msg.source.funcname;
    pack.line_ = msg.source.line;

    pack.level_ = msg.level;
    pack.logger_ = {msg.logger_name.data(), msg.logger_name.size()};
    pack.thread_id_ = msg.thread_id;
    pack.time_ = msg.time;

    pack.text_ = std::move(str);
    log_packets_.push_back(std::move(pack));
  }

  void flush_() override {}

public:
  std::shared_ptr<std::string> log_buffer_ = std::make_shared<std::string>();
  std::vector<Log_packet> log_packets_;

  std::shared_ptr<std::vector<spdlog::details::log_msg>> msgs{
      std::make_shared<std::vector<spdlog::details::log_msg>>()};

  auto &get_formater() { return *spdlog::sinks::base_sink<Mutex>::formatter_; }
  void change_pattern(std::string fmt) {
    spdlog::sinks::base_sink<Mutex>::set_pattern_(fmt);
  }
};

#include "spdlog/details/null_mutex.h"
#include <mutex>

using Imgui_sink_mt = Imgui_sink<std::mutex>;
using Imgui_sink_st = Imgui_sink<spdlog::details::null_mutex>;

#include <imgui.h>
#include <imgui_stdlib.h>
#include <spdlog/spdlog.h>
class Imgui_log_window {
public:
  Imgui_log_window() {
    std::shared_ptr<Imgui_sink_st> sink = std::make_shared<Imgui_sink_st>();
    sink_ = sink;
    buffer_ = sink->log_buffer_;
    spdlog::default_logger()->sinks().push_back(sink);
    sink->set_pattern(pattern_);
  }

  void draw() {
    if (ImGui::Begin("Logs")) {
      if (ImGui::InputText("pattern", &pattern_)) {
        sink_->change_pattern(pattern_);
      }
      ImGui::Checkbox("time limit", &only_recent_logs_);

      auto current = std::chrono::high_resolution_clock::now();

      if (ImGui::BeginChild("Log")) {

        for (auto &msg : sink_->log_packets_) {

          if (only_recent_logs_) {
            if (current - msg.time_ > std::chrono::seconds{10}) {
              continue;
            }
          }

          auto &str = msg.text_;
          if (msg.level_ > SPDLOG_LEVEL_INFO) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImColor(255, 0, 0).Value);
          } else {
            ImGui::PushStyleColor(ImGuiCol_Text, ImColor(0, 255, 0).Value);
          }
          ImGui::Text("%s", str.c_str());
          ImGui::PopStyleColor();
          if (ImGui::BeginItemTooltip()) {
            ImGui::Text("thread:%zu", msg.thread_id_);
            ImGui::Text("position:%s:%d", msg.file_.data(), msg.line_);
            ImGui::EndTooltip();
          }
        }
      }
    }
    ImGui::EndChild();

    ImGui::End();
  }

private:
  std::shared_ptr<std::string> buffer_;
  std::shared_ptr<Imgui_sink_st> sink_;

  // max align is 64.
  std::string pattern_ = "[%T.%f %L] %-64v [+%6oms]";
  bool only_recent_logs_{};
};