#pragma once
#include "spdlog/sinks/base_sink.h"
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

    // log_msg is a struct containing the log entry info like level, timestamp,
    // thread id etc. msg.raw contains pre formatted log

    // If needed (very likely but not mandatory), the sink formats the message
    // before sending it to its final destination:
    spdlog::memory_buf_t formatted;
    spdlog::sinks::base_sink<Mutex>::formatter_->format(msg, formatted);

    msgs->push_back(msg);

    *log_buffer_ += fmt::to_string(formatted);
  }

  void flush_() override {}

public:
  std::shared_ptr<std::string> log_buffer_ = std::make_shared<std::string>();

  std::shared_ptr<std::vector<spdlog::details::log_msg>> msgs{
      std::make_shared<std::vector<spdlog::details::log_msg>>()};

  auto &get_formater() { return *spdlog::sinks::base_sink<Mutex>::formatter_; }
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
    std::shared_ptr<Imgui_sink_st> sink = std::make_shared<Imgui_sink_st>();
    sink_ = sink;
    buffer_ = sink->log_buffer_;
    msgs_ = sink->msgs;
    spdlog::default_logger()->sinks().push_back(sink);
  }

  void draw() {
    if (ImGui::Begin("Logs")) {
      //   ImGui::Text("%s", buffer_->c_str());
      for (auto &msg : *msgs_) {
        spdlog::memory_buf_t formatted;

        sink_->get_formater().format(msg, formatted);
        auto str = fmt::to_string(formatted);
        if (msg.source.line < 400) {
          ImGui::PushStyleColor(ImGuiCol_Text, ImColor(255, 0, 0).Value);
        } else {
          ImGui::PushStyleColor(ImGuiCol_Text, ImColor(0, 255, 0).Value);
        }
        ImGui::Text("%s", str.c_str());
        ImGui::PopStyleColor();
        if (ImGui::BeginItemTooltip()) {
          ImGui::Text("Thread: %p", msg.thread_id);
          ImGui::Text("In: %s", msg.source.funcname);
          ImGui::EndTooltip();
        }
      }
    }
    ImGui::End();
  }

private:
  std::shared_ptr<std::string> buffer_;
  std::shared_ptr<std::vector<spdlog::details::log_msg>> msgs_;
  std::shared_ptr<Imgui_sink_st> sink_;
};