#ifndef __AGENT_COMM_H__
#define __AGENT_COMM_H__

#define PACKET_LEN 2000
#define NW_STATUS_OK 0
#define NW_STATUS_DISCONNECTED 1


/**
 * Connect network
 * Return 0 if connect success
 */
int nw_connect();

/**
 * Destroy network
 */
void nw_destroy();

/**
 * Check network okay
 * Return 1 if network okay
 */
inline int nw_okay();

/**
 * Write data to network
 * return Number of bytes written
 */
int nw_write(const char *buff, size_t length);

/**
 * Read data to buffer
 * Return Number of bytes read
 */
int nw_read(char *buff);

/**
 * Disconnect network
 */
void nw_disconnect(void);

int nw_encrypt(const char *buff);

#endif
