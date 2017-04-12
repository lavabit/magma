
/**
 * @file /check/magma/network/network_check.c
 *
 * @brief Check the network module, and the various protocol agnostic connection/client interfaces.
 */

#include "magma_check.h"

Suite * suite_check_network(void) {

	Suite *s = suite_create("\tNetwork");

	// The IP address checks were the only thing handled by this suite. Those checks have since moved to
	// to core. The empty suite remains to remind us what needs doing.

	/// MEDIUM: Write checks which stress the connection accept logic.
	/// MEDIUM: Write checks for the con_write/con_read/con_read_line interfaces.
	/// MEDIUM: Write checks for the client_write/client_read/client_read_line interfaces.
	///
	return s;
}
