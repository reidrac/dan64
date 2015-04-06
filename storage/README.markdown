Data encoding scheme
====================

Each bit is encoded with a square wave half pulse using the distance between
edges to differentiate between zeroes and ones.

As different programs are expected to be stored in different audio files and
the playback will happen from a reliable medium (at least more reliable than a
cassette; eg, mp3 player), a very basic sync method is used. The signal always
starts with a short low that translates in two changes that will be discarded.

There are two types of pulse: "short" and "long".

 - short: period ~362 microseconds, denotes 0
 - long: period ~833 microseconds, denotes 1

The data is encoded as follows:

 - Header:
   - Magic (1 byte): 0xff
   - Data length (2 bytes, MSB first)
   - Parity (1 byte): XOR of data length bytes
 - Body:
   - Data: n bytes
   - Parity (1 byte): XOR of data bytes


Audio in
--------

The most common nominal level for consumer audio is -10 dBV, that is 0.447 volts
of peak amplitude. The audio signal is shifted ~2.272 volts to avoid noise
triggered edges.

Reference: http://en.wikipedia.org/wiki/Line_level

Audio out
---------

Audio out is more unreliable than I thought. Large programs can be loaded as long
as they're encoded in a PC (see `tools/encode`), but programs saved with the
microcomputer may not load back. It is OK for up to a couple of KBs, but more than
that is probably not usable.

Also the device used to record the audio out may not register enough volume level
so it can be read back by the microcomputer.

