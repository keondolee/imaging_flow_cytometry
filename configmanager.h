#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <string>
#include <fstream>

class ConfigManager
{
private:
  const std::string conf_path = "cell_sorter.conf";

public:
  std::string search_conf(std::string );
  int StringtoInt(std::string);
  double StringtoDouble(std::string);
  void update_conf(int, int, int, int, int, int, int, int, int, int, int, int, int, double, double, double, std::string, std::string);
  void default_conf();
  void reset_conf();
};

#endif // CONFIGMANAGER_H
