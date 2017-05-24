XXX-RADIO


XXX-radio, a command line demodulator for SDRplay or Airspy

Input - depending on the configuration - is through an SDRplay device
(either RSP1 or RSP2) or an Airspy device. All parameters
are set through the command line. Output is to a file (or stdout if
no file is selected). The inputrate is internally converted to
a working rate of 192000, so FM stereo decoding is possible.

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

The last line is the command (again, for XXX_radio read SDRplay_radio or airspy_radio whichever you configured) for getting the messages of the P2000 system in the Netherlands. It - obviously - assumes that you have installed multimon.

To listen - in the Netherlands - to the only classical music station in the FM broadcast band use
sdrplay-radio -f 94700000 -g 30 -M fm-stereo -D -Z |aplay -r 22050 -f S16_LE -t raw -c 2

Creating an executable

The CMakeLists.txt file can be used wirh cmake to create Makefiles. 

	mkdir build
	cd build
	cmake .. -DSDRPLAY=ON
	make
	sudo make install

is the normal way of generating an executable and installing it in /usr/local/bin



