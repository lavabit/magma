
/**
 * @file /magma/network/reverse.c
 *
 * @brief	Function to perform reverse DNS lookups.
 */

#include "magma.h"

/**
 * @brief	Queue a reverse DNS lookup on the specified connection, if one hasn't been performed.
 * @param	con		the connection object to be examined.
 * @return	This function returns no value.
 */
void con_reverse_enqueue(connection_t *con) {

	int_t pending;

	mutex_lock(&(con->lock));

	if ((pending = con->network.reverse.status) == REVERSE_EMPTY) {
		con->network.reverse.status = REVERSE_PENDING;
		con->refs++;
	}

	mutex_unlock(&(con->lock));

	if (pending == REVERSE_EMPTY) {
		enqueue(&con_reverse_lookup, con);
	}

	return;
}

/**
 * @brief	Set the domain name and reverse lookup status of a connection.
 * @note	Possible values for status include REVERSE_ERROR, REVERSE_EMPTY, REVERSE_PENDING, and REVERSE_COMPLETE.
 * @param	domain	the new value of the connection's hostname.
 * @param	status	the new value of the connection's reverse lookup status.
 * @return	This function returns no value.
 */
void con_reverse_domain(connection_t *con, stringer_t *domain, int_t status) {

	mutex_lock(&(con->lock));
	con->network.reverse.status = status;
	con->network.reverse.domain = domain;
	mutex_unlock(&(con->lock));

	return;
}

/*
 * @brief	Set the reverse lookup status of a connection.
 * @note	Possible values for status include REVERSE_ERROR, REVERSE_EMPTY, REVERSE_PENDING, and REVERSE_COMPLETE.
 * @param	con		the connection object to be adjusted.
 * @param	status	the new value of the connection's status code.
 * @return	This function returns no value.
 */
void con_reverse_status(connection_t *con, int_t status) {

	mutex_lock(&(con->lock));
	con->network.reverse.status = status;
	mutex_unlock(&(con->lock));

	return;
}

/**
 * @brief	Poll a connection periodically to see if a reverse DNS lookup operation has completed.
 * @note	Polling will occur every 100ms, for as many seconds as specified by the user.
 * @param	con		the specified connection object to be polled that is the target of the DNS lookup.
 * @param	timeout	the number of seconds to wait for the lookup operation to complete.
 * @return	NULL on failure, or a pointer to a managed string containing the connection's peer hostname on success.
 */
stringer_t * con_reverse_check(connection_t *con, uint32_t timeout) {

	stringer_t *result = NULL;
	int_t pending, counter = (timeout *10);
	struct timespec request = { .tv_sec= 0, .tv_nsec= 100000000 };

	do {

		mutex_lock(&(con->lock));

		if ((pending = con->network.reverse.status) == REVERSE_COMPLETE) {
			result = con->network.reverse.domain;
		}

		mutex_unlock(&(con->lock));

		// Sleep for 1/10th of a second.
		if (pending == REVERSE_PENDING && counter && status()) {
			nanosleep(&request, NULL);
		}

	} while (!result && pending == REVERSE_PENDING && counter-- && status());


	return result;
}

/**
 * @brief	Perform a reverse DNS lookup on the remote end of a connection, and save the hostname.
 * @param	con		the connection object to be queried.
 * @return	This function returns no value.
 */
void con_reverse_lookup(connection_t *con) {

	void *addr;
	int_t ecode;
	stringer_t *domain;
	sa_family_t family;
	struct sockaddr_in6 address;
	struct hostent host, *hostp;
	socklen_t len = sizeof(struct sockaddr_in6);

	// Perform the lookup.
	if (getsockname(con->network.sockd, &address, &len)) {
		con_reverse_status(con, REVERSE_ERROR);
		con_destroy(con);
		return;
	}

	if (address.sin6_family == AF_INET6) {
		family = address.sin6_family;
		addr = (void *)&(address.sin6_addr);
	}
	else if (address.sin6_family == AF_INET) {
		family = address.sin6_family;
		addr = (void *)&(((struct sockaddr_in *)&address)->sin_addr);
	}

	if (!(gethostbyaddr_r(addr, len, family, &host, bufptr, buflen, &hostp, &ecode)) &&
		(len = ns_length_get(host.h_name)) && (domain = st_import(host.h_name, len))) {
		con_reverse_domain(con, domain, REVERSE_COMPLETE);
	}
	else {
		con_reverse_status(con, REVERSE_ERROR);
	}

	con_destroy(con);

	return;
}
