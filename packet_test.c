#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <stdarg.h>
#include "config.h"
#include "gpsd.h"

static int verbose = 0;

void gpsd_report(int errlevel, const char *fmt, ... )
/* assemble command in printf(3) style, use stderr or syslog */
{
    if (errlevel <= verbose) {
	char buf[BUFSIZ];
	va_list ap;

	buf[0] = '\0';
	va_start(ap, fmt) ;
	(void)vsnprintf(buf + strlen(buf), sizeof(buf)-strlen(buf), fmt, ap);
	va_end(ap);

	(void)fputs(buf, stderr);
    }
}

int main(int argc, char *argv[])
{
    struct map {
	char	*legend;
	char	test[MAX_PACKET_LENGTH+1];
	size_t	testlen;
	int	garbage_offset;
	int	type;
    };
    /*@ -initallelements +charint -usedef @*/
    struct map tests[] = {
	/* NMEA tests */
	{
	    "NMEA packet with checksum (1)",
	    "$GPVTG,308.74,T,,M,0.00,N,0.0,K*68\r\n",
	    36,
	    0,
	    NMEA_PACKET,
	},
	{
	    "NMEA packet with checksum (2)",
	    "$GPGGA,110534.994,4002.1425,N,07531.2585,W,0,00,50.0,172.7,M,-33.8,M,0.0,0000*7A\r\n",
	    82,
	    0,
	    NMEA_PACKET,
	},
	{
	    "NMEA packet with checksum and 4 chars of leading garbage",
	    "\xff\xbf\x00\xbf$GPVTG,308.74,T,,M,0.00,N,0.0,K*68\r\n",
	    40,
	    4,
	    NMEA_PACKET,
	},
	{
	    "NMEA packet without checksum",
	    "$PSRF105,1\r\n",
	    12,
	    0,
	    NMEA_PACKET,
	},
	{
	    "NMEA packet with wrong checksum",
	    "$GPVTG,308.74,T,,M,0.00,N,0.0,K*28\r\n",
	    36,
	    0,
	    BAD_PACKET,
	},
	/* SiRF tests */
	{
	    "SiRF WAAS version ID",
	    {
		0xA0, 0xA2, 0x00, 0x15, 
		0x06, 0x06, 0x31, 0x2E, 0x32, 0x2E, 0x30, 0x44, 
		0x4B, 0x49, 0x54, 0x31, 0x31, 0x39, 0x20, 0x53, 
		0x4D, 0x00, 0x00, 0x00, 0x00,
		0x03, 0x82, 0xB0, 0xB3},
	    29,
	    0,
	    SIRF_PACKET,
	},
	{
	    "SiRF WAAS version ID with 3 chars of leading garbage",
	    {
		0xff, 0x00, 0xff,
		0xA0, 0xA2, 0x00, 0x15, 
		0x06, 0x06, 0x31, 0x2E, 0x32, 0x2E, 0x30, 0x44, 
		0x4B, 0x49, 0x54, 0x31, 0x31, 0x39, 0x20, 0x53, 
		0x4D, 0x00, 0x00, 0x00, 0x00,
		0x03, 0x82, 0xB0, 0xB3},
	    32,
	    3,
	    SIRF_PACKET,
	},
	{
	    "SiRF WAAS version ID with wrong checksum",
	    {
		0xA0, 0xA2, 0x00, 0x15, 
		0x06, 0x06, 0x31, 0x2E, 0x32, 0x2E, 0x30, 0x44, 
		0x4B, 0x49, 0x54, 0x31, 0x31, 0x39, 0x20, 0x53, 
		0x4D, 0x00, 0x00, 0x00, 0x00,
		0x03, 0x00, 0xB0, 0xB3},
	    29,
	    0,
	    BAD_PACKET,
	},
	{
	    "SiRF WAAS version ID with bad length",
	    {
		0xA0, 0xA2, 0xff, 0x15, 
		0x06, 0x06, 0x31, 0x2E, 0x32, 0x2E, 0x30, 0x44, 
		0x4B, 0x49, 0x54, 0x31, 0x31, 0x39, 0x20, 0x53, 
		0x4D, 0x00, 0x00, 0x00, 0x00,
		0x03, 0x82, 0xB0, 0xB3},
	    29,
	    0,
	    BAD_PACKET,
	},
	/* Zodiac tests */
	{
	    "Zodiac binary 1000 Geodetic Status Output Message",
	    {
		0xff, 0x81, 0xe8, 0x03, 0x31, 0x00, 0x00, 0x00, 0xe8, 0x79, 
		0x74, 0x0e, 0x00, 0x00, 0x24, 0x00, 0x24, 0x00, 0x04, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x90, 0x03, 0x23, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1d, 0x00, 0x06, 0x00, 
		0xcd, 0x07, 0x00, 0x00, 0x00, 0x00, 0x16, 0x00, 0x7b, 0x0d, 
		0x00, 0x00, 0x12, 0x6b, 0xa7, 0x04, 0x41, 0x75, 0x32, 0xf8, 
		0x03, 0x1f, 0x00, 0x00, 0xe6, 0xf2, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x11, 0xf6, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x40, 
		0xd9, 0x12, 0x90, 0xd0, 0x03, 0x00, 0x00, 0xa3, 0xe1, 0x11, 
		0x10, 0x27, 0x00, 0x00, 0x00, 0x00, 0x00, 0xa3, 0xe1, 0x11, 
		0x00, 0x00, 0x00, 0x00, 0xe0, 0x93, 0x04, 0x00, 0x04, 0xaa},
	    110,
	    0,
	    ZODIAC_PACKET,
	},
    };
    /*@ +initallelements -charint +usedef @*/

    struct map *mp;
    struct gps_device_t state;
    ssize_t st;

    if (argc > 1)
	verbose = atoi(argv[1]); 

    for (mp = tests; mp < tests + sizeof(tests)/sizeof(tests[0]); mp++) {
	state.packet_type = BAD_PACKET;
	state.packet_state = 0;
	state.inbuflen = 0;
	/*@i@*/memcpy(state.inbufptr = state.inbuffer, mp->test, mp->testlen);
	/*@ -compdef -uniondef -usedef @*/
	st = packet_parse(&state, mp->testlen);
	if (state.packet_type != mp->type)
	    printf("%s test FAILED (packet type %d wrong).\n", mp->legend, (int)st);
	else if (memcmp(mp->test + mp->garbage_offset, state.outbuffer, state.outbuflen))
	    printf("%s test FAILED (data garbled).\n", mp->legend);
	else
	    printf("%s test succeeded.\n", mp->legend);
#ifdef DUMPIT
	for (cp = state.outbuffer; 
	     cp < state.outbuffer + state.outbuflen; 
	     cp++) {
	    if (st != NMEA_PACKET)
		(void)printf(" 0x%02x", *cp);
	    else if (*cp == '\r')
		(void)fputs("\\r", stdout);
	    else if (*cp == '\n')
		(void)fputs("\\n", stdout);
	    else if (isprint(*cp))
		(void)putchar(*cp);
	    else
		(void)printf("\\x%02x", *cp);
	}
	(void)putchar('\n');
#endif /* DUMPIT */
	/*@ +compdef +uniondef +usedef @*/
    }

    exit(0);
}
