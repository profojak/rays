#define MDSPAN_USE_PAREN_OPERATOR 1

#include <experimental/mdspan>

#include <experimental/linalg>

#include <print>
#include <span>
#include <vector>

namespace rays {

void test() {
  std::vector<float> data = {1.0f, 2.0f, 3.0f};
  std::mdspan<float, std::extents<int, std::dynamic_extent>> ms(data.data(),
                                                                data.size());
  std::experimental::linalg::scaled(2.0f, ms);
  std::println("{}, {}, {}", ms(0), ms(1), ms(2));
}

} // namespace rays
