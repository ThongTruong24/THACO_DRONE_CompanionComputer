#pragma once
#include <filesystem>
#include <map>
#include <string>
#include <vector>
namespace cc {
struct Parameter {
  std::string name;
  double value, minimum, maximum;
  bool integral, live;
};
struct SetResult { bool accepted; bool pending; std::string reason; };
class ParameterStore {
public:
  explicit ParameterStore(std::filesystem::path directory);
  void add(Parameter parameter);
  void load();
  SetResult set(const std::string & name, double value, bool enabled);
  const std::vector<Parameter> & all() const { return parameters_; }
  const Parameter * find(const std::string & name) const;
  double active(const std::string & name) const;
private:
  void persist(const std::vector<Parameter> & candidate) const;
  std::filesystem::path directory_;
  std::vector<Parameter> parameters_;
  std::map<std::string,double> active_;
};
}
