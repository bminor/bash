/*   
 * netopen.c -- functions to make tcp/udp connections
 *
 * Chet Ramey
 * chet@ins.CWRU.Edu
 */

/* Copyright (C) 1987,1991 Free Software Foundation, Inc.

   This file is part of GNU Bash, the Bourne Again SHell.

   Bash is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   Bash is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with Bash; see the file COPYING.  If not, write to the Free
   Software Foundation, 59 Temple Place, Suite 330, Boston, MA 02111 USA. */

#include <config.h>

#if defined (HAVE_NETWORK)

#include <stdio.h> 
#include <sys/types.h>

#if defined (HAVE_SYS_SOCKET_H)
#  include <sys/socket.h>
#endif

#if defined (HAVE_NETINET_IN_H)
#  include <netinet/in.h>
#endif

#if defined (HAVE_NETDB_H)
#  include <netdb.h>
#endif

#if defined (HAVE_ARPA_INET_H)
#  include <arpa/inet.h>
#endif

#include <bashansi.h>
#include <ctype.h>
#include <errno.h>

#ifndef errno
extern int errno;
#endif

#if !defined (HAVE_INET_ATON)
extern int inet_aton ();
#endif

extern char *xmalloc ();

/* Stuff the internet address corresponding to HOST into AP, in network
   byte order.  Return 1 on success, 0 on failure. */

static int
_getaddr (host, ap)
     char *host;
     struct in_addr *ap;
{
  struct hostent *h;
  int r;

  r = 0;
  if (isdigit (host[0]))
    {
      /* If the first character is a digit, guess that it's an
	 Internet address and return immediately if inet_aton succeeds. */
      r = inet_aton (host, ap);
      if (r)
	return r;
    }
#if !defined (HAVE_GETHOSTBYNAME)
  return 0;
#else
  h = gethostbyname (host);
  if (h && h->h_addr)
    {
      bcopy(h->h_addr, (char *)ap, h->h_length);
      return 1;
    }
#endif
  return 0;
  
}

/* Return 1 if SERV is a valid port number and stuff the converted value into
   PP in network byte order. */   
static int
_getserv (serv, proto, pp)
     char *serv;
     int proto;
     unsigned short *pp;
{
  long l;
  unsigned short s;

  if (legal_number (serv, &l))
    {
      if (l > 65535)
	return 0;
      s = (unsigned short)(l & 0xFFFF);
      s = htons (s);
      if (pp)
	*pp = s;
      return 1;
    }
  else
#if defined (HAVE_GETSERVBYNAME)
    {
      struct servent *se;

      se = getservbyname (serv, (proto == 't') ? "tcp" : "udp");
      if (se == 0)
	return 0;
      if (pp)
	*pp = se->s_port;	/* ports returned in network byte order */
      return 1;
    }
#else /* !HAVE_GETSERVBYNAME */
    return 0;
#endif /* !HAVE_GETSERVBYNAME */
}

static int 
_netopen(host, serv, typ)
     char *host, *serv;
     int typ;
{
  struct in_addr ina;
  struct sockaddr_in sin;
  unsigned short p;
  int s, e;
  char **cp;

  if (_getaddr(host, &ina) == 0)
    {
      internal_error ("%s: host unknown", host);
      errno = EINVAL;
      return -1;
    }

  if (_getserv(serv, typ, &p) == 0)
    {
      internal_error("%s: invalid service", serv);
      errno = EINVAL;
      return -1;
    }
	
  bzero ((char *)&sin, sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_port = p;
  sin.sin_addr = ina;

  s = socket(AF_INET, (typ == 't') ? SOCK_STREAM : SOCK_DGRAM, 0);
  if (s < 0)
    {
      sys_error ("socket");
      return (-1);
    }

  if (connect (s, (struct sockaddr *)&sin, sizeof (sin)) < 0)
    {
      e = errno;
      sys_error("connect");
      close(s);
      errno = e;
      return (-1);
    }

  return(s);
}

/*
 * Open a TCP or UDP connection given a path like `/dev/tcp/host/port' to
 * host `host' on port `port' and return the connected socket.
 */
int
netopen (path)
     char *path;
{
  char *np, *s, *t;
  int fd;

  np = xmalloc (strlen (path) + 1);
  strcpy (np, path);

  s = np + 9;
  t = strchr (s, '/');
  if (t == 0)
    {
      internal_error ("%s: bad network path specification", path);
      return -1;
    }
  *t++ = '\0';
  fd = _netopen (s, t, path[5]);
  free (np);

  return fd;
}

#if 0
/*
 * Open a TCP connection to host `host' on the port defined for service
 * `serv' and return the connected socket.
 */
int
tcpopen (host, serv)
     char *host, *serv;
{
  return (_netopen (host, serv, 't'));
}

/*
 * Open a UDP connection to host `host' on the port defined for service
 * `serv' and return the connected socket.
 */
int
udpopen (host, serv)
     char *host, *serv;
{
  return _netopen (host, serv, 'u');
}
#endif

#else /* !HAVE_NETWORK */

int
netopen (path)
     char *path;
{
  internal_error ("network operations not supported");
  return -1;
}

#endif /* !HAVE_NETWORK */
