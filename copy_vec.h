#pragma oncd

void copy_xyz(const auto &src, auto &dest) {
  dest.x = src.x;
  dest.y = src.y;
  dest.z = src.z;
}
void copy_xy(const auto &src, auto &dest) {
  dest.x = src.x;
  dest.y = src.y;
}