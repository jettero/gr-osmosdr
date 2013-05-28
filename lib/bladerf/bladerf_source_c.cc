/* -*- c++ -*- */
/*
 * Copyright 2013 Jon Szymaniak <jon.szymaniak+gpg@gmail.com>
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

/*
 * config.h is generated by configure.  It contains the results
 * of probing for features, options etc.  It should be the first
 * file included in your .cc file.
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <iostream>
#include <boost/assign.hpp>
#include <gnuradio/gr_io_signature.h>
#include <osmosdr_arg_helpers.h>
#include "bladerf_source_c.h"

using namespace boost::assign;

/*
 * Create a new instance of bladerf_source_c and return
 * a boost shared_ptr.  This is effectively the public constructor.
 */
bladerf_source_c_sptr make_bladerf_source_c (const std::string &args)
{
  return gnuradio::get_initial_sptr(new bladerf_source_c (args));
}

/*
 * Specify constraints on number of input and output streams.
 * This info is used to construct the input and output signatures
 * (2nd & 3rd args to gr_block's constructor).  The input and
 * output signatures are used by the runtime system to
 * check that a valid number and type of inputs and outputs
 * are connected to this block.  In this case, we accept
 * only 0 input and 1 output.
 */
static const int MIN_IN = 0;	// mininum number of input streams
static const int MAX_IN = 0;	// maximum number of input streams
static const int MIN_OUT = 1;	// minimum number of output streams
static const int MAX_OUT = 1;	// maximum number of output streams

/*
 * The private constructor
 */
// TODO
bladerf_source_c::bladerf_source_c (const std::string &args)
  : gr_sync_block ("bladerf_source_c",
        gr_make_io_signature (MIN_IN, MAX_IN, sizeof (gr_complex)),
        gr_make_io_signature (MIN_OUT, MAX_OUT, sizeof (gr_complex)))
{
  std::cout << "Hello world, from bladeRF source!" << std::endl;
}

/*
 * Our virtual destructor.
 */
// TODO
bladerf_source_c::~bladerf_source_c ()
{
}

// TODO
int bladerf_source_c::work( int noutput_items,
                        gr_vector_const_void_star &input_items,
                        gr_vector_void_star &output_items )
{

  return noutput_items;
}

// TODO
std::vector<std::string> bladerf_source_c::get_devices()
{
  std::vector<std::string> devices;
  std::string args = "bladerf=0, label='nuand bladeRF'";
  devices.push_back(args);

  return devices;
}

size_t bladerf_source_c::get_num_channels()
{
  return 1;
}

// TODO
osmosdr::meta_range_t bladerf_source_c::get_sample_rates()
{
  osmosdr::meta_range_t range;
  return range;
}

// TODO
double bladerf_source_c::set_sample_rate(double rate)
{
  return 0;
}

// TODO
double bladerf_source_c::get_sample_rate()
{
  return 0;
}

// TODO
osmosdr::freq_range_t bladerf_source_c::get_freq_range( size_t chan )
{
  osmosdr::freq_range_t range;

  /* there is a (temperature dependent) gap between 1100 to 1250 MHz */
  range += osmosdr::range_t( 52e6, 2.2e9 );

  return range;
}

// TODO
double bladerf_source_c::set_center_freq( double freq, size_t chan )
{
  return get_center_freq( chan );
}

// TODO
double bladerf_source_c::get_center_freq( size_t chan )
{
  return 0;
}

// TODO
double bladerf_source_c::set_freq_corr( double ppm, size_t chan )
{
  return get_freq_corr( chan );
}

// TODO
double bladerf_source_c::get_freq_corr( size_t chan )
{
  return 0;
}

// TODO
std::vector<std::string> bladerf_source_c::get_gain_names( size_t chan )
{
  std::vector< std::string > names;

  names += "LNA";
  names += "IF";

  return names;
}

// TODO
osmosdr::gain_range_t bladerf_source_c::get_gain_range( size_t chan )
{
  osmosdr::gain_range_t range;
  return range;
}

// TODO
osmosdr::gain_range_t bladerf_source_c::get_gain_range( const std::string & name, size_t chan )
{
  return get_gain_range( chan );
}

// TODO
bool bladerf_source_c::set_gain_mode( bool automatic, size_t chan )
{
  return false;
}

// TODO
bool bladerf_source_c::get_gain_mode( size_t chan )
{
  return false;
}

// TODO
double bladerf_source_c::set_gain( double gain, size_t chan )
{
  return get_gain( chan );
}

// TODO
double bladerf_source_c::set_gain( double gain, const std::string & name, size_t chan)
{
  return set_gain( gain, chan );
}

// TODO
double bladerf_source_c::get_gain( size_t chan )
{
  return 0;
}

// TODO
double bladerf_source_c::get_gain( const std::string & name, size_t chan )
{
  return get_gain( chan );
}

// TODO
double bladerf_source_c::set_if_gain(double gain, size_t chan)
{
  return gain;
}

// TODO
std::vector< std::string > bladerf_source_c::get_antennas( size_t chan )
{
  std::vector< std::string > antennas;

  antennas += get_antenna( chan );

  return antennas;
}

// TODO
std::string bladerf_source_c::set_antenna( const std::string & antenna, size_t chan )
{
  return get_antenna( chan );
}

// TODO
std::string bladerf_source_c::get_antenna( size_t chan )
{
  return "RX";
}