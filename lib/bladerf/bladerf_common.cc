#include <string>
#include <iomanip>
#include <sstream>
#include <bladerf.h>
#include "bladerf_common.h"

std::vector< std::string > bladerf_common::devices()
{
  int status;
  struct ::bladerf_devinfo *devices;
  ssize_t n_devices;
  std::vector< std::string > ret;

  n_devices = bladerf_get_device_list(&devices);

  if (n_devices > 0) {
    for (ssize_t i = 0; i < n_devices; i++)  {

      std::stringstream s;
      std::string dev(devices[i].path);

      s << "bladerf=" << dev.substr(dev.find_first_of("01234567890")) << ","
        << "label='nuand bladeRF (" << std::setfill('0') << std::setw(16)
        << devices[0].serial << ")'";

      ret.push_back(s.str());
    }
  }

  return ret;
}
