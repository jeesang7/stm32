/**
  ******************************************************************************
  * @file    net_tcp_lwip.c
  * @author  MCD Application Team
  * @brief   Network abstraction at transport layer level. TCP implementation on
             LwIP.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2017 STMicroelectronics International N.V.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "net_internal.h"

#ifdef USE_LWIP
#include "lwip/netdb.h"

#if !LWIP_SO_RCVTIMEO || !LWIP_SO_RCVRCVTIMEO_NONSTANDARD
#error  lwipopt.h must define LWIP_SO_RCVTIMEO so that the socket read timeout is supported.
#endif /* !LWIP_SO_RCVTIMEO */

#if !LWIP_SO_SNDTIMEO || !LWIP_SO_SNDRCVTIMEO_NONSTANDARD
#error  lwipopt.h must define LWIP_SO_SNDTIMEO so that the socket write timeout is supported.
#endif /* !LWIP_SO_RCVTIMEO */

/* Private defines -----------------------------------------------------------*/

/** In case of HW link disconnection:
  *   NET_OK for relying on the underlying socket,
  *   NET_EOF for an immediate explicit error. */
#define UNPLUG_RETURN_CODE  NET_OK

/* Private typedef -----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
int net_sock_create_lwip(net_hnd_t nethnd, net_sockhnd_t * sockhnd, net_proto_t proto);
int net_sock_open_lwip(net_sockhnd_t sockhnd, const char * hostname, int remoteport, int localport);
int net_sock_recv_tcp_lwip(net_sockhnd_t sockhnd, uint8_t * buf, size_t len);
int net_sock_recvfrom_udp_lwip(net_sockhnd_t sockhnd, uint8_t * const buf, size_t len, net_ipaddr_t * remoteaddress, int * remoteport);
int net_sock_send_tcp_lwip( net_sockhnd_t sockhnd, const uint8_t * buf, size_t len);
int net_sock_sendto_udp_lwip(net_sockhnd_t sockhnd, const uint8_t * buf, size_t len,  net_ipaddr_t * remoteaddress, int remoteport);
int net_sock_close_tcp_lwip(net_sockhnd_t sockhnd);
int net_sock_destroy_tcp_lwip(net_sockhnd_t sockhnd);
int net_get_hostaddress_lwip(net_hnd_t nethnd, net_ipaddr_t * ipAddress, const char * host);

/* Functions Definition ------------------------------------------------------*/

int net_sock_create_lwip(net_hnd_t nethnd, net_sockhnd_t * sockhnd, net_proto_t proto)
{
  int rc = NET_ERR;
  net_ctxt_t *ctxt = (net_ctxt_t *) nethnd;
  net_sock_ctxt_t *sock = NULL;

  sock = net_malloc(sizeof(net_sock_ctxt_t));
  if (sock == NULL)
  {
    msg_error("net_sock_create allocation failed.\n");
    rc = NET_ERR;
  }
  else
  {
    memset(sock, 0, sizeof(net_sock_ctxt_t));
    sock->net = ctxt;
    sock->next = ctxt->sock_list;
    sock->methods.open            = (net_sock_open_lwip);
    switch(proto)
    {
      case NET_PROTO_TCP:
        sock->methods.recv        = (net_sock_recv_tcp_lwip);
        sock->methods.send        = (net_sock_send_tcp_lwip);
        break;
      case NET_PROTO_UDP:
        sock->methods.recvfrom    = (net_sock_recvfrom_udp_lwip);
        sock->methods.sendto      = (net_sock_sendto_udp_lwip);
        break;
      default:
        free(sock);
        return NET_PARAM;
    }
    sock->methods.close           =  (net_sock_close_tcp_lwip);
    sock->methods.destroy         =  (net_sock_destroy_tcp_lwip);
    sock->proto             = proto;
    sock->blocking          = NET_DEFAULT_BLOCKING;
    sock->read_timeout      = NET_DEFAULT_BLOCKING_READ_TIMEOUT;
    sock->write_timeout     = NET_DEFAULT_BLOCKING_WRITE_TIMEOUT;
    ctxt->sock_list         = sock; /* Insert at the head of the list */
    *sockhnd = (net_sockhnd_t) sock;

    rc = NET_OK;
  }

  return rc;
}


int net_sock_open_lwip(net_sockhnd_t sockhnd, const char * hostname, int dstport, int localport)
{
  int rc = NET_OK;
  net_sock_ctxt_t *sock = (net_sock_ctxt_t * ) sockhnd;

  char portBuffer[6];
  struct addrinfo hints;
  struct addrinfo *list = NULL;
  struct addrinfo *current = NULL;
  int socket = -1;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;        /* TODO: Add IP version to the create() function parameter list. */

  switch (sock->proto)
  {
    case NET_PROTO_TCP:
      if (localport != 0)
      { /* TCP local port setting is not implemented */
        rc = NET_PARAM;
      }
      else
      {
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;
        snprintf(portBuffer, 5, "%d", dstport);
      }
      break;
    case NET_PROTO_UDP:
      if (dstport != 0)
      { /* UDP default remote port setting is not implemented */
        rc = NET_PARAM;
      }
      else
      {
        hints.ai_socktype = SOCK_DGRAM;
        hints.ai_protocol = IPPROTO_UDP;
        hints.ai_flags    = AI_PASSIVE;
        snprintf(portBuffer, sizeof(portBuffer), "%d", localport);
      }
      break;
    default:
      return NET_PARAM;
  }

  if (rc != NET_OK)
  {
    return rc;
  }

  if( (getaddrinfo(hostname, portBuffer, &hints, &list) != 0) || (list == NULL) )
  {
    msg_info("The address of %s could not be resolved.\n", hostname);
    rc = NET_NOT_FOUND;
  }
  else
  {
    for(current = list; (current != NULL) && (socket == -1); current = current->ai_next)
    {
      socket = (int) socket(current->ai_family, current->ai_socktype, current->ai_protocol);
      if(socket >= 0)
      {
        if ( (sock->read_timeout != 0) && sock->blocking )
        {
          int opt = sock->read_timeout;
          if (0 != lwip_setsockopt (socket, SOL_SOCKET, SO_RCVTIMEO, &opt, sizeof(opt)))
          {
            msg_error("Could not set the read timeout.\n");
            rc = NET_ERR;
          }
        }

        if ( (rc == NET_OK) && (sock->write_timeout != 0) && sock->blocking )
        {
          int opt = sock->write_timeout;
          if (0 != lwip_setsockopt (socket, SOL_SOCKET, SO_SNDTIMEO, &opt, sizeof(opt)))
          {
            msg_error("Could not set the write timeout.\n");
            rc = NET_ERR;
          }
        }

        if (rc == NET_OK)
        {
          switch(sock->proto)
          {
            case NET_PROTO_TCP:
            {
              if (0 == connect(socket, current->ai_addr, (uint32_t)current->ai_addrlen))
              {
                sock->underlying_sock_ctxt = (net_sockhnd_t) socket;
                rc = NET_OK;
              }
              else
              {
                msg_error("connect() failed with error: %d\n", errno);
              }
              break;
            }
            case NET_PROTO_UDP:
            {
              if (0 == bind(socket, current->ai_addr, current->ai_addrlen))
              {
                sock->underlying_sock_ctxt = (net_sockhnd_t) socket;
                rc = NET_OK;
              }
              else
              {
                msg_error("bind() failed with error: %d\n", errno);
              }
              break;
            }
            default:
              return NET_PARAM;
          }
        }
        else
        {
          close(socket);
          socket = -1;
          rc = NET_NOT_FOUND;
        }
      }
    }
    freeaddrinfo(list);
  }

  return rc;
}


int net_sock_recv_tcp_lwip(net_sockhnd_t sockhnd, uint8_t * buf, size_t len)
{
  int rc = NET_OK;
  net_sock_ctxt_t *sock = (net_sock_ctxt_t * ) sockhnd;

  if (sock->underlying_sock_ctxt < 0)
  {
    rc = NET_PARAM;
  }
  else if(netif_is_link_up(&sock->net->lwip_netif) != true)
  {
    msg_error("Please connect the network cable.\n");
    rc = UNPLUG_RETURN_CODE;
  }

  if (rc == NET_OK)
  {
    int ret = -1;

    if ( (sock->read_timeout != 0) && sock->blocking )
    {
      int opt = sock->read_timeout;
      if (0 != lwip_setsockopt ((int)sock->underlying_sock_ctxt, SOL_SOCKET, SO_RCVTIMEO, &opt, sizeof(opt)))
      {
        msg_error("Could not set the read timeout.\n");
        rc = NET_ERR;
      }
    }

    if (rc == NET_OK) do
    {
      ret = recv((int)sock->underlying_sock_ctxt, buf, len, ((sock->read_timeout != 0) && sock->blocking) ? 0 : MSG_DONTWAIT);
      if (ret > 0)
      {
        rc = ret;
      }
      else if (ret == 0)
      {
        rc = NET_EOF;
      }
      else
      {
        switch(errno)
        {
          case EWOULDBLOCK:
            /* LwIP internally translates timeouts into retries. */
            if ((sock->read_timeout != 0) && sock->blocking)
            {
              rc = NET_TIMEOUT;
            }
            break;
          case EINTR:
            /* Incomplete read. The caller should try again. */
            break;
          case EPIPE:
          case ECONNRESET:
            rc = NET_EOF;
            break;
          case ENOTCONN:    /* The network cable is unplugged. */
            rc = UNPLUG_RETURN_CODE;
            break;
          default:
            rc = NET_ERR;
        }
      }
    } while ( ((sock->read_timeout != 0) && sock->blocking) && (rc == NET_OK) );
  }

  return rc;
}


int net_sock_recvfrom_udp_lwip(net_sockhnd_t sockhnd, uint8_t * const buf, size_t len, net_ipaddr_t * remoteaddress, int * remoteport)
{
  int rc = NET_OK;
  net_sock_ctxt_t *sock = (net_sock_ctxt_t * ) sockhnd;

  if (sock->underlying_sock_ctxt < 0)
  {
    rc = NET_PARAM;
  }
  else if(netif_is_link_up(&sock->net->lwip_netif) != true)
  {
    msg_error("Please connect the network cable.\n");
    rc = UNPLUG_RETURN_CODE;
  }

  if (rc == NET_OK)
  {
    int ret = -1;
    struct sockaddr from;
    socklen_t fromlen = sizeof(from);
    memset(&from, 0, sizeof(from));

    if ( (sock->read_timeout != 0) && sock->blocking )
    {
      int opt = sock->read_timeout;
      if (0 != lwip_setsockopt ((int)sock->underlying_sock_ctxt, SOL_SOCKET, SO_RCVTIMEO, &opt, sizeof(opt)))
      {
      msg_error("Could not set the read timeout.\n");
      rc = NET_ERR;
      }
    }

    if (rc == NET_OK)
    {
      ret = recvfrom((int)sock->underlying_sock_ctxt, buf, len, ((sock->read_timeout != 0) && sock->blocking) ? 0 : MSG_DONTWAIT,
                     &from, &fromlen);
      if (ret > 0)
      {
        if (from.sa_family == AF_INET)
        {
          rc = ret;
          struct sockaddr_in *saddr = (struct sockaddr_in *) &from;
          remoteaddress->ipv = NET_IP_V4;
          memset(remoteaddress->ip, 0xFF, sizeof(remoteaddress->ip));
          memcpy(&remoteaddress->ip[12], &saddr->sin_addr, 4);
          *remoteport = ntohs(saddr->sin_port);
        }
        else
        {
          /* IPv6 not implemented. */
          rc = NET_ERR;
        }
      }
      else if (ret == 0)
      {
        rc = NET_EOF;
      }
      else
      {
        switch(errno)
        {
          case EWOULDBLOCK:
            if ((sock->read_timeout != 0) && sock->blocking)
            {
              rc = NET_TIMEOUT;
            }
            else
            {
              msg_error("Recv timeout raised while the operation is non-blocking.\n");
            }
            break;
          case EINTR:
            break;
          case EPIPE:
          case ECONNRESET:
            rc = NET_EOF;
            break;
          case ENOTCONN:
            rc = UNPLUG_RETURN_CODE;
            break;
          default:
            rc = NET_ERR;
        }
      }
    }
  }

  return rc;
}


int net_sock_send_tcp_lwip( net_sockhnd_t sockhnd, const uint8_t * buf, size_t len)
{
  int rc = NET_OK;
  net_sock_ctxt_t *sock = (net_sock_ctxt_t * ) sockhnd;

  if (sock->underlying_sock_ctxt < 0)
  {
    rc = NET_PARAM;
  }
  else if(netif_is_link_up(&sock->net->lwip_netif) != true)
  {
    msg_error("Please connect the network cable.\n");
    rc = UNPLUG_RETURN_CODE;
  }

  if (rc == NET_OK)
  {
    int ret = -1;

    if ( (sock->write_timeout != 0) && sock->blocking )
    {
      int opt = sock->write_timeout;
      if (0 != lwip_setsockopt ((int)sock->underlying_sock_ctxt, SOL_SOCKET, SO_SNDTIMEO, &opt, sizeof(opt)))
      {
        msg_error("Could not set the write timeout.\n");
        rc = NET_ERR;
      }
    }

    do
    {
      ret = send((int)sock->underlying_sock_ctxt, buf, len, ((sock->write_timeout != 0) && sock->blocking) ? 0 : MSG_DONTWAIT);
      if (ret > 0)
      {
        rc = ret;
      }
      else if (ret == 0)
      {
        rc = NET_EOF;
      }
      else
      {
        switch(errno)
        {
          case EWOULDBLOCK:
            /* LwIP internally translates timeouts into retries. */
            if ((sock->write_timeout != 0) && sock->blocking)
            {
              rc = NET_TIMEOUT;
            }
            break;
          case EINTR:
           /* Incomplete send. The caller should try again. */
            break;
          case EPIPE:
          case ECONNRESET:
            rc = NET_EOF;
            break;
          case ENOTCONN:
            rc = UNPLUG_RETURN_CODE;
            break;
          default:
            rc = NET_ERR;
        }
      }
    } while ( ((sock->write_timeout != 0) && sock->blocking) && (rc == NET_OK) );
  }

  return rc;
}


int net_sock_sendto_udp_lwip(net_sockhnd_t sockhnd, const uint8_t * buf, size_t len, net_ipaddr_t * remoteaddress, int remoteport)
{
  int rc = NET_OK;
  net_sock_ctxt_t *sock = (net_sock_ctxt_t * ) sockhnd;

  if (sock->underlying_sock_ctxt < 0)
  {
    rc = NET_PARAM;
  }
  else if(netif_is_link_up(&sock->net->lwip_netif) != true)
  {
    msg_error("Please connect the network cable.\n");
    rc = UNPLUG_RETURN_CODE;
  }

  if (rc == NET_OK)
  {
    int ret = -1;

    if ( (sock->write_timeout != 0) && sock->blocking )
    {
      int opt = sock->write_timeout;
      if (0 != lwip_setsockopt ((int)sock->underlying_sock_ctxt, SOL_SOCKET, SO_SNDTIMEO, &opt, sizeof(opt)))
      {
        msg_error("Could not set the write timeout.\n");
        rc = NET_ERR;
      }
    }

    do
    {
      struct sockaddr_in to;
      memset(&to, 0, sizeof(to));

      to.sin_len = 4;
      to.sin_family = (remoteaddress->ipv == NET_IP_V4) ? AF_INET : AF_UNSPEC;
      to.sin_port = htons(remoteport);
      memcpy(&to.sin_addr.s_addr, &remoteaddress->ip[12], 4);

      ret = sendto((int)sock->underlying_sock_ctxt, buf, len, ((sock->write_timeout != 0) && sock->blocking) ? 0 : MSG_DONTWAIT,
                   (struct sockaddr *) &to, sizeof(to));
      if(ret > 0)
      {
        rc = ret;
      }
      else if (ret == 0)
      {
        rc = NET_EOF;
      }
      else
      {
        switch(ret)
        {
          case EINTR:
           /* Incomplete write. The caller should try again. */
            break;
          case EWOULDBLOCK:
            /* LwIP internally translates timeouts into retries. */
            if ((sock->write_timeout != 0) && sock->blocking)
            {
              rc = NET_TIMEOUT;
            }
            break;
          case EPIPE:
          case ECONNRESET:
            rc = NET_EOF;
            break;
          case ENOTCONN:
            rc = UNPLUG_RETURN_CODE;
            break;
          default:
            rc = NET_ERR;
        }
      }
    } while ( ((sock->write_timeout != 0) && sock->blocking) && (rc == NET_OK) );
  }

  return rc;
}


int net_sock_close_tcp_lwip(net_sockhnd_t sockhnd)
{
  int rc = NET_ERR;
  net_sock_ctxt_t *sock = (net_sock_ctxt_t * ) sockhnd;

  if(((int) sock->underlying_sock_ctxt) >= 0)
  {
    if(0 == shutdown((int)sock->underlying_sock_ctxt, SHUT_RDWR))
    {
      if (0 == close((int)sock->underlying_sock_ctxt))
      {
        sock->underlying_sock_ctxt = (net_sockhnd_t) -1;
        rc = NET_OK;
      }
      else
      {
        msg_error("Could not close the socket %d. Error: %d\n", (int)sock->underlying_sock_ctxt, errno);
      }
    }
    else
    {
      msg_error("Could not shutdown the socket %d. Error: %d - %s\n", (int)sock->underlying_sock_ctxt, errno, lwip_strerr(errno));
    }
  }
  else
  {
    msg_warning("Underlying socket already closed. Skipping net_sock_close_tcp_lwip().");
    rc = NET_OK;
  }

  return rc;
}


int net_sock_destroy_tcp_lwip(net_sockhnd_t sockhnd)
{
  int rc = NET_ERR;
  net_sock_ctxt_t *sock = (net_sock_ctxt_t * ) sockhnd;
  net_ctxt_t *ctxt = sock->net;

  /* Find the parent in the linked list.
   * Unlink and free.
   */
  if (sock == ctxt->sock_list)
  {
    ctxt->sock_list = sock->next;
    rc = NET_OK;
  }
  else
  {
    net_sock_ctxt_t *cur = ctxt->sock_list;
    do
    {
      if (cur->next == sock)
      {
        cur->next = sock->next;
        rc = NET_OK;
        break;
      }
      cur = cur->next;
    } while(cur->next != NULL);
  }
  if (rc == NET_OK)
  {
    net_free(sock);
  }

  return rc;
}


int net_get_hostaddress_lwip(net_hnd_t nethnd, net_ipaddr_t * ipAddress, const char * host)
{
  net_ctxt_t *ctxt = (net_ctxt_t *) nethnd;
  int rc = NET_ERR;

  if ((ipAddress == NULL) || (host == NULL))
  {
    rc = NET_PARAM;
  }
  else
  {
    if (ctxt->lwip_netif.ip_addr.addr == 0)
    {
      /* The network interface is not configured. */
      rc = NET_PARAM;
    }
    else
    {
      int ret = 0;
      struct addrinfo hints;
      struct addrinfo *servinfo;

      memset(&hints, 0, sizeof hints);
      hints.ai_family = AF_INET;
      hints.ai_socktype = SOCK_DGRAM;
      hints.ai_flags = AI_PASSIVE;

      ret = getaddrinfo(host, NULL, &hints, &servinfo);
      if (ret != 0)
      {
          msg_error("getaddrinfo error: %d.\n", ret);
          rc = NET_ERR;
      }
      /* servinfo now points to a linked list of 1 or more struct addrinfos */

      ipAddress->ipv = NET_IP_V4;
      memset(ipAddress->ip, 0xFF, sizeof(ipAddress->ip));
      memcpy(&ipAddress->ip[12], &((struct sockaddr_in *)(servinfo->ai_addr))->sin_addr, 4);
      rc = NET_OK;
      freeaddrinfo(servinfo);
    }
  }

  return rc;
}

#endif /* USE_LWIP */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
