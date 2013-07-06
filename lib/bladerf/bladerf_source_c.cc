/* -*- c++ -*- */
/*
 * Copyright 2013 Nuand
 * This file shamelessly borrow significant portions from the rtl, osmosdr,
 * uhd, etc. code by Dimitri Stolnikov <horiz0n@gmx.net>
 *
 * GNU Radio is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option) * any later version.
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
bladerf_source_c::bladerf_source_c (const std::string &args)
  : gr_sync_block ("bladerf_source_c",
        gr_make_io_signature (MIN_IN, MAX_IN, sizeof (gr_complex)),
        gr_make_io_signature (MIN_OUT, MAX_OUT, sizeof (gr_complex)))
{

  /* Setup our sample rates */
  this->sample_range = osmosdr::meta_range_t( 160e3, 40e6 );

  /* Setup frequency range */
  this->freq_range = osmosdr::freq_range_t( 300e6, 3.8e9 );

  /* Setup the LNA range */
  this->lna_range = osmosdr::gain_range_t( 0, 6, 3 );

  /* Setup the VGA1/Mixer gain range */
  this->vga1_range = osmosdr::gain_range_t( 2, 30 );

  /* Setup the VGA2 range */
  this->vga2_range = osmosdr::gain_range_t( 0, 60, 3 );

  /* Open a handle to the free device */
  this->dev = bladerf_open( "/dev/bladerf1" );
  if( NULL == this->dev ) {
    throw std::runtime_error( std::string(__FUNCTION__)
            + " has failed to get a device .. any device!" );
  }

  /* Set the multiples for calls to this block to be 4096 samples */
  this->set_output_multiple(1024);
  //this->set_max_noutput_items(1024);

  if (this->dev) {
      this->thread = gruel::thread(read_task_dispatch, this);
  }
}

/*
 * Our virtual destructor.
 */
bladerf_source_c::~bladerf_source_c ()
{
  this->set_running(false);
  this->thread.join();

  /* Close the device */
  bladerf_close( this->dev );
}

void bladerf_source_c::read_task_dispatch(bladerf_source_c *obj)
{
    obj->read_task();
}

void bladerf_source_c::read_task()
{
  int16_t si, sq;
  ssize_t n_samples_bytes;
  size_t n_samples, n_avail, to_copy;
  bool source_running;


  while (this->is_running()) {

    //std::cout << "Running task..." << std::endl;

    /* FIXME This is going to change to n_samples (not bytes!) in a
     * an upcoming fix to libbladeRF */
    n_samples_bytes = bladerf_read_c16(this->dev, this->raw_sample_buf,
                                       BLADERF_SAMPLE_BLOCK_SIZE_BYTES);

    if (n_samples_bytes < 0) {
      std::cerr << "Failed to read samples: "
        << bladerf_strerror(n_samples_bytes) << std::endl;

      this->set_running(false);

    } else {

      if (n_samples_bytes != BLADERF_SAMPLE_BLOCK_SIZE_BYTES) {
        std::cerr << "Warning: received truncated sample block of "
                  << n_samples << " bytes!"<< std::endl;
      } else {

        /* Samples are interleaved I/Q Values, with a "sample" being a pair */
        n_samples = n_samples_bytes / (2 * sizeof(int16_t));
        n_avail = this->sample_fifo->capacity() - this->sample_fifo->size();
        to_copy = (n_avail < n_samples ? n_avail : n_samples);

        this->sample_fifo_lock.lock();

        for (size_t i = 0; i < to_copy; ++i) {
          /* Sign extend the 12-bit IQ values, if needed */
          si &= 0xfff; sq &= 0xfff;
          if( si & 0x800 ) si |= 0xf000;
          if( sq & 0x800 ) sq |= 0xf000;

          gr_complex sample((float)si * (1.0f/2048.0f),
                            (float)sq * (1.0f/2048.0f));
          this->sample_fifo->push_back(sample);
        }

        this->sample_fifo_lock.unlock();
        this->samples_available.notify_one();

        /* Indicate overrun, if neccesary */
        if (to_copy < n_samples) {
          std::cerr << "O";
        }
      }
    }
  }
}


/* Main work function, pull samples from the driver */
int bladerf_source_c::work( int noutput_items,
                        gr_vector_const_void_star &input_items,
                        gr_vector_void_star &output_items )
{
  int n_samples_avail;

  if( this->dev && noutput_items >= 0) {
    gr_complex *out = (gr_complex *)output_items[0];
    boost::unique_lock<boost::mutex> lock(this->sample_fifo_lock);

    /* Wait until we have the requested number of samples */
    n_samples_avail = this->sample_fifo->size();

    while (n_samples_avail < noutput_items) {
      this->samples_available.wait(lock);
      n_samples_avail = this->sample_fifo->size();
    }

    for(int i = 0; i < noutput_items; ++i) {
      out[i] = this->sample_fifo->at(0);
      this->sample_fifo->pop_front();
    }
  } else {
    std::cout << "Device is not open!" << std::endl;
    noutput_items = -1;
  }

  return noutput_items;
}

std::vector<std::string> bladerf_source_c::get_devices()
{
  return bladerf_common::devices();
}

size_t bladerf_source_c::get_num_channels()
{
  /* We only support a single channel for each bladeRF */
  return 1;
}

osmosdr::meta_range_t bladerf_source_c::get_sample_rates()
{
  return this->sample_range;
}

double bladerf_source_c::set_sample_rate(double rate)
{
  /* Set the Si5338 to be 2x this sample rate */
  if( this->dev ) {
    /* Check to see if the sample rate is an integer */
    if( (uint32_t)round(rate) == (uint32_t)rate )
    {
        int ret;
        uint32_t actual;
        ret = bladerf_set_sample_rate( this->dev, RX, (uint32_t)rate, &actual );
        if( ret ) {
            throw std::runtime_error( std::string(__FUNCTION__)
                    + " has failed to set integer rate" );
        }
    } else {
        /* TODO: Fractional sample rate */
        int ret;
        uint32_t actual;
        ret = bladerf_set_sample_rate( this->dev, RX, (uint32_t)rate, &actual );
        if( ret ) {
            throw std::runtime_error( std::string(__FUNCTION__)
                    + " has failed to set fractional rate" );
        }
    }
  } else {
    throw std::runtime_error( std::string(__FUNCTION__)
            + " has failed due to lack of device" );
  }
  return this->get_sample_rate();
}

double bladerf_source_c::get_sample_rate()
{
  int ret;
  unsigned int rv = -1;

  if( this->dev ) {
    ret = bladerf_get_sample_rate( this->dev, RX, &rv );
    if( ret ) {
        throw std::runtime_error( std::string(__FUNCTION__)
                + " has failed to get sample rate" );
    }
  } else {
    throw std::runtime_error( std::string(__FUNCTION__)
            + " has failed due to lack of device" );
  }
  return (double)rv;
}

// TODO
osmosdr::freq_range_t bladerf_source_c::get_freq_range( size_t chan )
{
  /* Frequency range of device */
  return this->freq_range;
}

double bladerf_source_c::set_center_freq( double freq, size_t chan )
{
  if( this->dev ) {
    int ret;
    ret = bladerf_set_frequency( this->dev, TX, (uint32_t)freq );
    if( ret ) {
      throw std::runtime_error( std::string(__FUNCTION__)
              + " failed to set center frequency " );
    }
  } else {
    throw std::runtime_error( std::string(__FUNCTION__)
            + " failed due to lack of device" );
  }
  return get_center_freq( chan );
}

double bladerf_source_c::get_center_freq( size_t chan )
{
  uint32_t freq;
  int ret;
  if( this->dev ) {
    ret = bladerf_get_frequency( this->dev, RX, &freq );
    if( ret ) {
      throw std::runtime_error( std::string(__FUNCTION__)
              + " failed to read center frequency" );
    }
  } else {
    throw std::runtime_error( std::string(__FUNCTION__)
            + " failed to read center frequency due to lack of device" );
  }
  return (double)freq;
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
  names += "VGA2";
  names += "VGA1";

  return names;
}

// TODO: Figure out if this is for VGA2 or what?
osmosdr::gain_range_t bladerf_source_c::get_gain_range( size_t chan )
{
  return this->vga2_range;
}

osmosdr::gain_range_t
bladerf_source_c::get_gain_range( const std::string & name, size_t chan )
{
  osmosdr::gain_range_t range;
  if( name == "LNA" ) {
    range = this->lna_range;
  } else if( name == "VGA2" ) {
    range = this->vga2_range;
  } else if( name == "VGA1" ) {
    range = this->vga1_range;
  } else {
    throw std::runtime_error( std::string(__FUNCTION__) +
            " requested an invalid gain element " + name );
  }
  return range;
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

// TODO: Figure out if this is for VGA2 of what?
double bladerf_source_c::set_gain( double gain, size_t chan )
{
  return set_gain( gain, "VGA2", chan );
}

double
bladerf_source_c::set_gain( double gain, const std::string & name, size_t chan)
{
  if( this->dev ) {
    if( name == "LNA" ) {
      bladerf_lna_gain g;
      if( gain == 0.0 ) {
        g = LNA_BYPASS;
      } else if( gain == 3.0 ) {
        g = LNA_MID;
      } else if( gain == 6.0 ) {
        g = LNA_MAX;
      } else {
        std::cout << "Invalid LNA gain requested: "
                  << gain << std::endl << "Setting to LNA_MAX" << std::endl;
        g = LNA_MAX;
      }
      bladerf_set_lna_gain( this->dev, g );
    } else if( name == "VGA1" ) {
      bladerf_set_rxvga1( this->dev, (int)gain );
    } else if( name == "VGA2" ) {
      bladerf_set_rxvga2( this->dev, (int)gain );
    } else {
      throw std::runtime_error( std::string(__FUNCTION__)
                              + " requested an invalid gain element " + name );
    }
  } else {
    throw std::runtime_error( std::string(__FUNCTION__)
            + " failed due to not having a valid device open" );
  }
  return get_gain( name, 1 );
}

// TODO: Figure out if this is for VGA2 or what?
double bladerf_source_c::get_gain( size_t chan )
{
  return get_gain( "VGA2", chan );
}

// TODO
double bladerf_source_c::get_gain( const std::string & name, size_t chan )
{
  int g;
  if( this->dev ) {
    if( name == "LNA" ) {
        bladerf_lna_gain lna_g;
        bladerf_get_lna_gain( this->dev, &lna_g );
        g = lna_g == LNA_BYPASS ? 0 : lna_g == LNA_MID ? 3 : 6;
    } else if( name == "VGA1" ) {
        bladerf_get_rxvga1( this->dev, &g );
    } else if( name == "VGA2" ) {
        bladerf_get_rxvga2( this->dev, &g );
    } else {
      throw std::runtime_error( std::string(__FUNCTION__)
            + " requested to get the gain of an unknown gain element " + name );
    }
  } else {
    throw std::runtime_error( std::string(__FUNCTION__)
            + " failed due to not having a valid device open" );
  }
  return (double)g;
}

// TODO: Figure out if this is meant for VGA2 or what?
double bladerf_source_c::set_if_gain(double gain, size_t chan)
{
  return set_gain( gain, "VGA2", chan );
}

// TODO: Figure out if this is right
std::vector< std::string > bladerf_source_c::get_antennas( size_t chan )
{
  std::vector< std::string > antennas;

  antennas += get_antenna( chan );

  return antennas;
}

// TODO: Figure out if this is right
std::string bladerf_source_c::set_antenna( const std::string & antenna, size_t chan )
{
  return get_antenna( chan );
}

// TODO: Figure out if this is right
std::string bladerf_source_c::get_antenna( size_t chan )
{
  return "RX";
}
