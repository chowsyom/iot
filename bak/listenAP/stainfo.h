#include <IPAddress.h>
#include <vector>
extern "C" {
#include "user_interface.h"
}

struct StationInfo {
  StationInfo(const struct station_info *stat_info) : ip(*(uint32_t*)&stat_info->ip) {
    memcpy(mac, stat_info->bssid, 6);
  }
  friend bool operator<(const StationInfo& a, const StationInfo& b)
  {
    return memcmp((void*)a.mac, (void*)b.mac, 6) < 0;
  }
  uint8_t mac[6];
  IPAddress ip;
};

class StationInfoManager {
public:
  typedef void (*notify_func_t)(const StationInfo& stainfo, bool connected);
  StationInfoManager() {}
  void begin(notify_func_t notify_func);
  void loop();
  const std::vector<StationInfo>& list();
private:
   std::vector<StationInfo> stalist;
   notify_func_t notify_func;
};