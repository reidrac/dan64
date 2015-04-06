#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>

#include "storage.h"

#include <sndfile.h>

#define VERSION			"1.0"

void
raw_write(int16_t data, void *fd)
{
	int8_t byte = data >> 8;
	fwrite(&byte, sizeof(byte), 1, (FILE *)fd);
}

void
wav_write(int16_t data, void *fd)
{
	sf_write_short((SNDFILE *)fd, &data, 1);
}

int
encode(char *input, char *output, uint8_t raw)
{
	SNDFILE *sndout = NULL;
	SF_INFO sfinfo;
	uint16_t data_len;

	uint8_t byte;
	uint16_t count = 0;

	struct encoder_struct enc;
	FILE *fdi, *fdo = NULL;

	fdi = fopen(input, "r");
	if (!fdi)
	{
		fprintf(stderr, "Failed to open %s\n", input);
		return 1;
	}

	if (raw)
	{
		fdo = fopen(output, "w");
		if (!fdo)
		{
			fclose(fdi);
			fprintf(stderr, "Failed to open %s\n", output);
			return 1;
		}
		enc.write = &raw_write;
		enc.param = fdo;
		enc.volume = 16000;
	}
	else
	{
		sfinfo.samplerate = SAMPLERATE;
		sfinfo.channels = 1;
		sfinfo.format = SF_FORMAT_WAV | SF_FORMAT_PCM_U8;

		sndout = sf_open(output, SFM_WRITE, &sfinfo);
		if (!sndout)
		{
			fclose(fdi);
			fprintf(stderr, "Failed to open %s\n", output);
			return 1;
		}
		enc.write = &wav_write;
		enc.param = sndout;
		enc.volume = 32000;
	}

	fseek(fdi, 0L, SEEK_END);
	data_len = ftell(fdi);
	fseek(fdi, 0L, SEEK_SET);

	encode_header(&enc, data_len);

	while(!feof(fdi))
	{
		if (fread(&byte, sizeof(uint8_t), 1, fdi) > 0)
		{
			encode_byte(&enc, byte);
			count++;
		}
	}

	encode_end(&enc);

	fclose(fdi);

	if (raw)
		fclose(fdo);
	else
		sf_close(sndout);

	return 0;
}

void
help(char *argv0)
{
	fprintf(stderr,"Encode a data file into audio storage (wav or raw)\n"
			       "Copyright (C) 2015 Juan J. Martinez <jjm@usebox.net>\n\n"
			       "Usage: %s [-h] [-r] [-o output] input\n\n"
				   "   input        input filename\n"
				   "   -h           this help screen\n"
				   "   -v           print version an exit\n"
				   "   -r           raw output (default: wav)\n"
				   "   -o output    output filename (default: sound.wav/raw)\n\n"
				   , argv0);
}

int
main(int argc, char *argv[])
{
	int opt;
	uint8_t raw = 0;
	char *output = NULL;

	while ((opt = getopt(argc, argv, "vhro:")) != -1)
	{
		switch(opt)
		{
			case 'r':
				raw = 1;
				break;
			case 'o':
				output = strdup(optarg);
				break;
			case 'h':
				help(argv[0]);
				exit(0);
			case 'v':
				fprintf(stderr,  VERSION "\n");
				exit(0);
			default:
				fprintf(stderr, "\n");
				help(argv[0]);
				exit(1);
		}
	}

	if (optind >= argc)
	{
		fprintf(stderr, "Expected input filename to encode\n");
		exit(1);
	}

	if (!output)
	{
		if (raw)
			output = strdup("sound.raw");
		else
			output = strdup("sound.wav");
	}

	exit(encode(argv[optind], output, raw));
}

