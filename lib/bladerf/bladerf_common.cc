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
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <boost/lexical_cast.hpp>
#include <libbladeRF.h>
#include "bladerf_common.h"

#define BLADERF_FIFO_SIZE_ENV   "BLADERF_SAMPLE_FIFO_SIZE"

bladerf_common::bladerf_common() : running(true)
{
    const char *env_fifo_size;
    size_t fifo_size;

    /* 1 Sample = i,q (2 int16_t's) */
    this->raw_sample_buf = new int16_t[2 * BLADERF_SAMPLE_BLOCK_SIZE];
    if (!raw_sample_buf) {
        throw std::runtime_error( std::string(__FUNCTION__) +
                "has failed to allocate a raw sample buffer!" ) ;
    }

    env_fifo_size = getenv(BLADERF_FIFO_SIZE_ENV);
    fifo_size = BLADERF_SAMPLE_FIFO_SIZE;

    if (env_fifo_size != NULL) {
        try {
            fifo_size = boost::lexical_cast<size_t>(env_fifo_size);
        } catch (const boost::bad_lexical_cast &e) {
            std::cerr << "Warning: \"" << BLADERF_FIFO_SIZE_ENV
                      << "\" is invalid" << "... defaulting to "
                      << fifo_size;
        }

        if (fifo_size < BLADERF_SAMPLE_FIFO_MIN_SIZE) {
            fifo_size = BLADERF_SAMPLE_FIFO_MIN_SIZE;
            std::cerr << "Warning: \"" << BLADERF_FIFO_SIZE_ENV
                      << "\" is too small" << "... defaulting to "
                      << BLADERF_SAMPLE_FIFO_MIN_SIZE;
        }
    }

    this->sample_fifo = new boost::circular_buffer<gr_complex>(fifo_size);
    if (!this->sample_fifo)
        throw std::runtime_error( std::string(__FUNCTION__) +
                                    "has failed to allocate a sample FIFO!" ) ;
}

bladerf_common::~bladerf_common()
{
    delete[] this->raw_sample_buf;
    delete this->sample_fifo;
}

void bladerf_common::setup_device()
{
    gpio_write( this->dev, 0x57 );
    lms_spi_write( this->dev, 0x05, 0x3e );
    lms_spi_write( this->dev, 0x59, 0x21 );
    lms_spi_write( this->dev, 0x64, 0x36 );
    return;
}

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

bool bladerf_common::is_running()
{
  boost::shared_lock<boost::shared_mutex> lock(this->state_lock);
  return this->running;
}

void bladerf_common::set_running(bool is_running)
{
  boost::unique_lock<boost::shared_mutex> lock(this->state_lock);
  this->running = is_running;
}
