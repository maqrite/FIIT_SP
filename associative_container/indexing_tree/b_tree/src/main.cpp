#include "b_tree.h"
#include "pp_allocator.h"
#include <iostream>
#include <string>

int main() {
  try {
    test_mem_resource mr;
    pp_allocator<std::pair<const int, std::string>> alloc(&mr);

    B_tree<int, std::string, std::less<int>, 3> tree(alloc);

    std::cout << "Дерево успешно создано." << std::endl;

    int key = 10;
    if (tree.contains(key)) {
      std::cout << "Критическая ошибка: ключ найден в пустом дереве!"
                << std::endl;
    } else {
      std::cout << "Поиск в пустом дереве работает корректно (ключ не найден)."
                << std::endl;
    }

  } catch (const std::exception &e) {
    std::cerr << "Поймали исключение: " << e.what() << std::endl;
  }

  return 0;
}
