/* libgps_sock.c -- client interface library for the gpsd daemon
 *
 * This file is Copyright (c) 2010 by the GPSD project
 * BSD terms apply: see the file COPYING in the distribution root for details.
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <locale.h>
#include <assert.h>
#include <sys/time.h>	 /* expected to have a select(2) prototype a la SuS */
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <unistd.h>

#ifndef USE_QT
#include <sys/socket.h>
#else
#include <QTcpSocket>
#endif /* USE_QT */

#include "gps.h"
#include "gpsd.h"
#include "libgps.h"
#include "strfuncs.h"
#ifdef SOCKET_EXPORT_ENABLE
#include "gps_json.h"


/* External Implementation functions Splitout for QT vs netlib socket handling */
extern int gps_sock_open_internal(const char *host, const char *port, struct gps_data_t *gpsdata);
extern bool gps_sock_waiting_internal(const struct gps_data_t *gpsdata, int timeout);
extern int gps_sock_close_internal(struct gps_data_t *gpsdata);
extern int gps_sock_read_internal(struct gps_data_t *gpsdata);
extern int gps_sock_send_internal(struct gps_data_t *gpsdata, const char *buf);


int gps_sock_open_internal(const char *host, const char *port,
		  struct gps_data_t *gpsdata)
{
    if (!host)
	return -2;
    if (!port)
	return -3;

#ifndef USE_QT
	if ((gpsdata->gps_fd =
	    netlib_connectsock(AF_UNSPEC, host, port, "tcp")) < 0) {
	    errno = gpsdata->gps_fd;
	    libgps_debug_trace((DEBUG_CALLS, "netlib_connectsock() returns error %d\n", errno));
	    return -1;
        }
	else
	    libgps_debug_trace((DEBUG_CALLS, "netlib_connectsock() returns socket on fd %d\n", gpsdata->gps_fd));
#else
	QTcpSocket *sock = new QTcpSocket();
	gpsdata->gps_fd = sock;
	sock->connectToHost(host, QString(port).toInt());
	if (!sock->waitForConnected())
        {
	    qDebug() << "libgps::connect error: " << sock->errorString();
            return -1;
	}
	else
	    qDebug() << "libgps::connected!";
#endif /* USE_QT */

    return 0;
}

bool gps_sock_waiting_internal(const struct gps_data_t *gpsdata, int timeout)
/* is there input waiting from the GPS? */
/* timeout is in uSec */
{
#ifndef USE_QT
    fd_set rfds;
    struct timeval tv;

    libgps_debug_trace((DEBUG_CALLS, "gps_waiting(%d): %d\n", timeout, PRIVATE(gpsdata)->waitcount++));
    if (PRIVATE(gpsdata)->waiting > 0)
	return true;

    /* we might want to check for EINTR if this returns false */
    errno = 0;

    FD_ZERO(&rfds);
    FD_SET(gpsdata->gps_fd, &rfds);
    tv.tv_sec = timeout / 1000000;
    tv.tv_usec = timeout % 1000000;
    /* all error conditions return "not waiting" -- crude but effective */
    return (select(gpsdata->gps_fd + 1, &rfds, NULL, NULL, &tv) == 1);
#else
    return ((QTcpSocket *) (gpsdata->gps_fd))->waitForReadyRead(timeout / 1000);
#endif
}

int gps_sock_close_internal(struct gps_data_t *gpsdata)
/* close a gpsd connection */
{
#ifndef USE_QT
    int status;

    free(PRIVATE(gpsdata));
    gpsdata->privdata = NULL;
    status = close(gpsdata->gps_fd);
    gpsdata->gps_fd = -1;
    return status;
#else
    QTcpSocket *sock = (QTcpSocket *) gpsdata->gps_fd;
    sock->disconnectFromHost();
    delete sock;
    gpsdata->gps_fd = NULL;
    return 0;
#endif
}

int gps_sock_read_internal(struct gps_data_t *gpsdata)
/* wait for and read data being streamed from the daemon */
{
    int status = -1;

#ifndef USE_QT
/* read data: return -1 if no data waiting or buffered, 0 otherwise */
    status = (int)recv(gpsdata->gps_fd,
		   PRIVATE(gpsdata)->buffer + PRIVATE(gpsdata)->waiting,
		   sizeof(PRIVATE(gpsdata)->buffer) - PRIVATE(gpsdata)->waiting, 0);
#else
    status =
        ((QTcpSocket *) (gpsdata->gps_fd))->read(PRIVATE(gpsdata)->buffer +
					     PRIVATE(gpsdata)->waiting,
					     sizeof(PRIVATE(gpsdata)->buffer) -
					     PRIVATE(gpsdata)->waiting);
#endif
    return status;
}

int gps_sock_send_internal(struct gps_data_t *gpsdata, const char *buf)
/* send a command to the gpsd instance */
{
#ifndef USE_QT
    if (write(gpsdata->gps_fd, buf, strlen(buf)) == (ssize_t) strlen(buf))
	return 0;
    else
	return -1;
#else
    QTcpSocket *sock = (QTcpSocket *) gpsdata->gps_fd;
    sock->write(buf, strlen(buf));
    if (sock->waitForBytesWritten())
	return 0;
    else {
	qDebug() << "libgps::send error: " << sock->errorString();
	return -1;
    }
#endif
}

#endif /* SOCKET_EXPORT_ENABLE */

/* end */
