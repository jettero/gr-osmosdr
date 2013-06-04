/* -*- c++ -*- */
/*
 * Copyright 2013 Nuand
 * This file shamelessly borrow significant portions from the rtl, osmosdr,
 * uhd, etc. code by Dimitri Stolnikov <horiz0n@gmx.net>
 *
 * GNU Radio is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * GNU Radio is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */
#include <string>
#include <iomanip>
#include <sstream>
#include <libbladeRF.h>
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
