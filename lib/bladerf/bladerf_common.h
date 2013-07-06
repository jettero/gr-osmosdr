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
#ifndef INCLUDED_BLADERF_COMMON_H
#define INCLUDED_BLADERF_COMMON_H

#include <boost/circular_buffer.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/condition_variable.hpp>
#include <vector>
#include <string>
#include <gr_complex.h>
#include <libbladeRF.h>

/*
 * Default size of sample FIFO, in entries.
 * This can be overridden by the environment variable BLADERF_SAMPLE_FIFO_SIZE.
 */
#ifndef BLADERF_SAMPLE_FIFO_SIZE
#   define BLADERF_SAMPLE_FIFO_SIZE   (2 * 1024 * 1024)
#endif

/* We currently read/write 1024 samples (pairs of 16-bit signed ints) */
#define BLADERF_SAMPLE_BLOCK_SIZE     (2 * 1024)
#define BLADERF_SAMPLE_BLOCK_SIZE_BYTES \
    (BLADERF_SAMPLE_BLOCK_SIZE * sizeof(int16_t))

/* Minimum of 3 "blocks" of data from library */
#define BLADERF_SAMPLE_FIFO_MIN_SIZE  (3 * 4096)

class bladerf_common
{
  public:
    bladerf_common();
    ~bladerf_common();

  protected:
    static std::vector< std::string > devices();
    bool is_running();
    void set_running(bool is_running);

    bladerf *dev;

    int16_t *raw_sample_buf;
    boost::circular_buffer<gr_complex> *sample_fifo;
    boost::mutex sample_fifo_lock;
    boost::condition_variable samples_available;

  private:
    bool running;
    boost::shared_mutex state_lock;

};

#endif
