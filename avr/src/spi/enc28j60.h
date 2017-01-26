/*
 * This file is part of PlipUltimate.
 * License: GPLv2
 * Full license: https://github.com/tehKaiN/plipUltimate/blob/master/LICENSE
 * Authors list: https://github.com/tehKaiN/plipUltimate/blob/master/AUTHORS
 */

#ifndef ENC28J60_H
#define ENC28J60_H

/**
 * ENC28J60 online status.
 * 1 if ENC is set up correctly, otherwise 0
 */
uint8_t g_ubEncOnline;

uint8_t enc28j60_init(const uint8_t macaddr[6], uint8_t flags);
void enc28j60_exit(void);
uint8_t enc28j60_send(const uint8_t *data, uint16_t size);
uint8_t enc28j60_recv(uint8_t *data, uint16_t max_size, uint16_t *got_size);
uint8_t enc28j60_has_recv(void);
uint8_t enc28j60_status(uint8_t status_id, uint8_t *value);
uint8_t enc28j60_control(uint8_t control_id, uint8_t value);

#endif
