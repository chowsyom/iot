#include "stainfo.h"
#include <iterator>
#include <algorithm>
#include <utility>

static inline void _getStationList(std::vector<StationInfo>& stalist)
{
  struct station_info *stat_info;
  stalist.clear();
    for (stat_info = wifi_softap_get_station_info(); stat_info; stat_info = STAILQ_NEXT(stat_info, next))
      stalist.emplace_back(stat_info);
    wifi_softap_free_station_info();
    sort(stalist.begin(), stalist.end());
}

void StationInfoManager::begin(notify_func_t notify_func)
{
  _getStationList(stalist);
  this->notify_func = notify_func;
}

void StationInfoManager::loop()
{
  std::vector<StationInfo> stalist_old = std::move(stalist);
  _getStationList(stalist);
  std::vector<StationInfo> staconnect, stadisconnect;
  std::set_difference(stalist.begin(),stalist.end(),stalist_old.begin(),stalist_old.end(),std::inserter(staconnect, staconnect.begin()));
  std::set_difference(stalist_old.begin(),stalist_old.end(),stalist.begin(),stalist.end(),std::inserter(stadisconnect, stadisconnect.begin())); 
  for (StationInfo& stainfo : staconnect)
    notify_func(stainfo, true);
  for (StationInfo& stainfo : stadisconnect)
    notify_func(stainfo, false);
}

const std::vector<StationInfo>& StationInfoManager::list()
{
  return stalist;
}