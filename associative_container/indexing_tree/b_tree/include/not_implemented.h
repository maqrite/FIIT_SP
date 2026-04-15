#ifndef SYS_PROG_NOT_IMPLEMENTED_H
#define SYS_PROG_NOT_IMPLEMENTED_H

#include <stdexcept>
#include <string>

class not_implemented : public std::logic_error {
public:
  not_implemented(const std::string &method, const std::string &message)
      : std::logic_error("Not implemented: " + method + " - " + message) {}
};

#endif // SYS_PROG_NOT_IMPLEMENTED_H
