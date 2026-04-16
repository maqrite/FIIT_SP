#include "b_tree.h"
#include "pp_allocator.h"
#include <iostream>
#include <string>

void print_const_tree(const B_tree<int, std::string, std::less<int>, 3> &tree) {
  std::cout << "Константный обход: " << std::endl;
  for (auto it = tree.cbegin(); it != tree.cend(); ++it) {
    std::cout << it->first << " : " << it->second << std::endl;
  }
}

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

    tree.insert({10, "десять"});
    tree.insert({20, "двадцать"});
    tree.insert({5, "пять"});

    std::cout << "Данные вставлены успешно" << std::endl;
    std::cout << "Текущий размер дерева: " << tree.size() << std::endl;

    std::cout << "Содержимое дерева:" << std::endl;
    for (auto it = tree.begin(); it != tree.end(); ++it) {
      std::cout << it->first << " => " << it->second << std::endl;
    }

    print_const_tree(tree);

  } catch (const std::exception &e) {
    std::cerr << "Поймали исключение: " << e.what() << std::endl;
  }

  return 0;
}
