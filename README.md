XXX-RADIO


XXX-radio, a command line FM demodulator for the SDRplay and AIRSpy

Input - depending on the configuration - is through an SDRplay device
(either RSP1 or RSP2) or an Airspy device. All parameters
are set using the command line. Output is to a file (or stdout if
no file is selected). The inputrate is internally converted to
a working rate of 192000, so FM stereo decoding is possible.

------------------------------------------------------------------------

The "radio" is modelled after the fm radio for the RTLSDR device.
All rights gratefully acknowledged.

------------------------------------------------------------------------

The command line parameters are

	-f Frequency in Hz
	-F Frequency in kHz
	A frequency HAS to be specified, it is the only value for
	which there is no default value.
	-p ppm_correction (default: 0)

	-M Mode select, default fm-mono. Other modes are
	   fm-stero, and HARDLY TESTED: am, usb and lsb and raw.
	   In the latter mode
	   the signal is just band filtered to the audiorate,
	   decimated and passed on.
	-D enables de-emphasis (default: off), useful for FM
	-Z enables lowpass filtering (default: off) of the audiosignal
	-s audio frequency, default is 22050
	-g tuner_gain, range 1 .. 100 (default: 35)
	-O output filename ( a '-' dumps output to stdout, as does
	   omitting a filename.

For the SDRplay we have additionally the following parameters

	-d device index, default: 0
	-A antenna selector (only for RSP 2, default antenna A)
	-G autogain (default: off)

The program produces signed 16 bit int values, single channel,
unless mode fm-stereo is selected, in which case two channels
interleaved are produced.

	XXX_radio -f 94700000 | play -t raw -r 22050 -es -b 16 -c 1 
	XXX_radio -f 94700000 -M fm-stereo | play -t raw -r 22050 -es -b 16 -c 2 
	XXX_radio -f 169650000 [-p3] |multimon-ng -a FLEX -t raw /dev/stdin

The last line is the command (again, for XXX_radio read sdrplay_radio or airspy_radio whichever you configured) for getting the messages of the P2000 system in the Netherlands. It - obviously - assumes that you have installed multimon.

In the (sub)directory P2000 a python script is given (obtained from

https://nl.oneguyoneblog.com/2016/08/09/p2000-ontvangen-decoderen-raspberry-pi/)
and adapted for use with the XXX-Radio to decode P2000 messages.


To listen - in the Netherlands - to the ONLY classical music station in the FM broadcast band use

sdrplay-radio -f 94700000 -g 30 -M fm-stereo -D -Z |aplay -r 22050 -f S16_LE -t raw -c 2

---------------------------------------------------------------------------

Creating an executable

---------------------------------------------------------------------------

The CMakeLists.txt file can be used  create Makefiles with cmake.

	mkdir build
	cd build
	cmake .. -DSDRPLAY=ON
	make
	sudo make install

is the normal way of generating an executable and
installing it in /usr/local/bin

-----------------------------------------------------------------------------

Packages needed

-----------------------------------------------------------------------------

To create the executable the following packages are needed

*	a. fftw			used in filtering
*	b. pthreads
*	c. libsndfile 
*	d. libsamplerate	used in non integer decimation

# Copyright

        Copyright (C)  2013, 2014, 2015, 2016, 2017
        Jan van Katwijk (J.vanKatwijk@gmail.com)
        Lazy Chair Computing

        The Qt-DAB software is made available under the GPL-2.0.
        The SDR-J software, of which the Qt-DAB software is a part,
        is distributed in the hope that it will be useful,
        but WITHOUT ANY WARRANTY; without even the implied warranty of
        MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
        GNU General Public License for more details.


