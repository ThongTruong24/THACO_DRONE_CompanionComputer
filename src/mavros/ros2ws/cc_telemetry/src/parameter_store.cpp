#include "cc_telemetry/parameter_store.hpp"
#include <nlohmann/json.hpp>
#include <cmath>
#include <fstream>
#include <stdexcept>
#include <fcntl.h>
#include <unistd.h>
namespace cc {
ParameterStore::ParameterStore(std::filesystem::path directory):directory_(std::move(directory)) {}
void ParameterStore::add(Parameter p) {
  if(p.name.empty() || p.name.size()>16 || find(p.name) || !std::isfinite(p.value) ||
     p.minimum>p.maximum || p.value<p.minimum || p.value>p.maximum)
    throw std::invalid_argument("Invalid parameter definition");
  active_[p.name]=p.value; parameters_.push_back(std::move(p));
}
const Parameter * ParameterStore::find(const std::string & name) const {
  for(const auto & p:parameters_) if(p.name==name) return &p;
  return nullptr;
}
double ParameterStore::active(const std::string & name) const { return active_.at(name); }
void ParameterStore::persist(const std::vector<Parameter> & candidate) const {
  std::filesystem::create_directories(directory_);
  nlohmann::json j;
  for(const auto & p:candidate) j[p.name]=p.value;
  const auto data=j.dump(2)+"\n";
  const auto tmp=directory_/"parameters.json.tmp";
  int fd=::open(tmp.c_str(),O_WRONLY|O_CREAT|O_TRUNC|O_NOFOLLOW,0600);
  if(fd<0) throw std::runtime_error("Cannot open parameter state");
  size_t offset=0;
  while(offset<data.size()) {
    auto n=::write(fd,data.data()+offset,data.size()-offset);
    if(n<=0) { ::close(fd); ::unlink(tmp.c_str()); throw std::runtime_error("State write failed"); }
    offset+=static_cast<size_t>(n);
  }
  int synced=::fsync(fd);int closed=::close(fd);
  if(synced || closed) {::unlink(tmp.c_str());throw std::runtime_error("State flush failed");}
  std::filesystem::rename(tmp,directory_/"parameters.json");
  int dir=::open(directory_.c_str(),O_RDONLY|O_DIRECTORY);
  if(dir>=0) {::fsync(dir);::close(dir);}
}
SetResult ParameterStore::set(const std::string & name,double value,bool enabled) {
  if(!enabled) return {false,false,"remote writes disabled"};
  auto candidate=parameters_;
  for(auto & p:candidate) if(p.name==name) {
    if(!std::isfinite(value) || value<p.minimum || value>p.maximum || (p.integral && std::trunc(value)!=value))
      return {false,false,"invalid value or range"};
    if(p.value==value) return {true,!p.live,"unchanged"};
    p.value=value;
    try {persist(candidate);} catch(const std::exception &) {return {false,false,"persistence failed"};}
    parameters_=std::move(candidate);
    const bool live=find(name)->live;
    if(live) active_[name]=value;
    return {true,!live,live?"applied":"saved desired; restart adapter required"};
  }
  return {false,false,"unknown or read-only parameter"};
}
void ParameterStore::load() {
  auto path=directory_/"parameters.json";
  if(!std::filesystem::exists(path)) return;
  std::ifstream input(path);auto j=nlohmann::json::parse(input);
  auto candidate=parameters_;
  for(auto & p:candidate) if(j.contains(p.name)) {
    const double v=j.at(p.name).get<double>();
    if(!std::isfinite(v)||v<p.minimum||v>p.maximum||(p.integral&&std::trunc(v)!=v))
      throw std::runtime_error("Invalid persisted parameter");
    p.value=v;
  }
  parameters_=std::move(candidate);
  for(const auto & p:parameters_) if(p.live) active_[p.name]=p.value;
}
}
