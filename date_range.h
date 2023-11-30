#pragma once
#include "imgui.h"
#include <ctime>
#include <string>
#include <vector>

// Utility functions
int DaysInMonth(int year, int month) {
  // Array of days in months
  static const int daysInMonth[12] = {31, 28, 31, 30, 31, 30,
                                      31, 31, 30, 31, 30, 31};
  if (month == 2 && ((year % 4 == 0 && year % 100 != 0) ||
                     year % 400 == 0)) // Leap year check for February
    return 29;
  return daysInMonth[month - 1];
}

int DayOfWeek(int year, int month, int day) {
  // Zeller's Congruence Algorithm to find the day of the week
  if (month < 3) {
    month += 12;
    year -= 1;
  }
  int k = year % 100;
  int j = year / 100;
  return (day + 13 * (month + 1) / 5 + k + k / 4 + j / 4 + 5 * j) % 7;
}

// ImGui Calendar Picker Widget
bool ImGuiCalendarPicker(const char *label, struct tm *date) {
  static const char *daysOfWeek[7] = {"Sun", "Mon", "Tue", "Wed",
                                      "Thu", "Fri", "Sat"};
  static const char *months[12] = {
      "January", "February", "March",     "April",   "May",      "June",
      "July",    "August",   "September", "October", "November", "December"};

  bool dateChanged = false;
  char buf[32];
  std::strftime(buf, sizeof(buf), "%Y-%m-%d", date);
  if (ImGui::Button(buf)) {
    ImGui::OpenPopup("Calendar");
  }

  if (ImGui::BeginPopup("Calendar")) {
    // Month and Year navigation
    if (ImGui::Button("<")) {
      if (--date->tm_mon < 0) {
        date->tm_mon += 12;
        date->tm_year--;
      }
    }
    ImGui::SameLine();
    // 9 is max len of the name of days in the week.
    ImGui::Text("%9s %d", months[date->tm_mon], 1900 + date->tm_year);
    ImGui::SameLine();
    if (ImGui::Button(">")) {
      if (++date->tm_mon > 11) {
        date->tm_mon -= 12;
        date->tm_year++;
      }
    }

    ImGui::Separator();

    // Days of the week
    for (int i = 0; i < 7; ++i) {
      if (i > 0)
        ImGui::SameLine();
      ImGui::Text("%s", daysOfWeek[i]);
    }

    int year = 1900 + date->tm_year;
    int month = 1 + date->tm_mon;
    int days = DaysInMonth(year, month);
    int dayOfWeek = DayOfWeek(year, month, 1);

    // Offset days from the start of the week
    for (int i = 0; i < dayOfWeek; ++i) {
      if (i > 0)
        ImGui::SameLine();
      ImGui::Text("   ");
      //   ImGui::PushID(i);
      //   ImGui::InvisibleButton(" 1", {0, 0});
      //   ImGui::PopID();
    }

    // Days
    for (int day = 1; day <= days; ++day) {
      if ((day + dayOfWeek - 1) % 7 > 0)
        ImGui::SameLine();
      std::sprintf(buf, "%2d", day);
      if (ImGui::Button(buf)) {
        date->tm_mday = day;
        dateChanged = true;
        ImGui::CloseCurrentPopup();
      }
    }

    ImGui::EndPopup();
  }

  return dateChanged;
}

// Usage Example
void render_date() {
  static struct tm selectedDate = {};
  if (selectedDate.tm_year == 0) {
    // Initialize the date to the current date if not already set
    time_t t = time(NULL);
    localtime_r(&t, &selectedDate);
  }

  if (ImGuiCalendarPicker("Select Date", &selectedDate)) {
    // Date changed logic here
  }
}
