#pragma once

#include <optional>
#include <span>
enum TimeUnit { Seconds, Minutes, Hours, Days, Years, TimeUnitCount };

inline const char *timeUnitNames[TimeUnitCount] = {"Seconds", "Minutes",
                                                   "Hours", "Days", "Years"};
#include "imgui.h"
#include <string>
#include <vector>
inline std::optional<int> text_combo(const std::string &combo_name,
                                     std::span<const std::string> items,
                                     int current_index) {
  if (items.empty()) {
    assert(false);
    return {};
  }
  std::optional<int> result_index{};
  if (ImGui::BeginCombo(combo_name.c_str(), items[current_index].c_str())) {
    for (int i = 0; i < TimeUnitCount; i++) {
      bool isSelected = (current_index == i);
      if (ImGui::Selectable(items[i].c_str(), isSelected)) {
        result_index = i;
      }

      if (isSelected) {
        ImGui::SetItemDefaultFocus();
      }
    }
    ImGui::EndCombo();
  }
  if (!result_index) {
    if (ImGui::IsItemHovered() && ImGui::GetIO().MouseWheel != 0) {
      int new_index =
          current_index + static_cast<int>(ImGui::GetIO().MouseWheel);
      if (new_index < 0)
        new_index = 0;
      if (new_index >= items.size())
        new_index = items.size() - 1;
      result_index = new_index;
    }
  }
  return result_index;
}
// Function to create a time input widget
inline void TimeInputWidget(const char *label, float *timeValue,
                            TimeUnit *timeUnit) {
  ImGui::PushID(label); // Ensure unique ID in case of multiple widgets
  ImGui::PushItemWidth(300);

  // Create a field for the time value
  ImGui::InputFloat("Value", timeValue);
  ImGui::SameLine();
  // Combo box for selecting the time unit
  if (ImGui::BeginCombo("Unit", timeUnitNames[*timeUnit])) {
    for (int i = 0; i < TimeUnitCount; i++) {
      bool isSelected = (*timeUnit == i);
      if (ImGui::Selectable(timeUnitNames[i], isSelected)) {
        *timeUnit = static_cast<TimeUnit>(i);
      }

      if (isSelected) {
        ImGui::SetItemDefaultFocus();
      }
    }
    ImGui::EndCombo();
  }
  // Handle mouse wheel event for changing the current item
  if (ImGui::IsItemHovered() && ImGui::GetIO().MouseWheel != 0) {
    int new_index = *timeUnit + static_cast<int>(ImGui::GetIO().MouseWheel);
    if (new_index < 0)
      new_index = 0;
    if (new_index >= TimeUnitCount)
      new_index = TimeUnitCount - 1;
    *timeUnit = static_cast<TimeUnit>(new_index);
  }

  ImGui::PopItemWidth();
  ImGui::PopID(); // Pop the unique ID
}
