
/**
 * @file /magma/web/portal/methods.h
 *
 * @brief	Definitions for all supported web portal methods and their calling interfaces.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#ifndef MAGMA_WEB_PORTAL_METHODS_H
#define MAGMA_WEB_PORTAL_METHODS_H

/*

  New method checklist:

  - Create portal_methods entry.
  - Create method handler stub.
  - Add method to error enumerator.
  - Create sample json request/responses in web project.
  - Add method test request to camel script.
*/

command_t portal_methods[] = {
{ .string = "auth" , .length = 4, .function = &portal_endpoint_auth },
{ .string = "ad" , .length = 2, .function = &portal_endpoint_ad },
{ .string = "alert.list" , .length = 10, .function = &portal_endpoint_alert_list },
{ .string = "alert.acknowledge" , .length = 17, .function = &portal_endpoint_alert_acknowledge },
{ .string = "aliases", .length = 7, .function = &portal_endpoint_aliases },
{ .string = "cookies", .length = 7, .function = &portal_endpoint_cookies },
{ .string = "scrape.add" , .length = 10, .function = &portal_endpoint_scrape_add },
{ .string = "scrape" , .length = 6, .function = &portal_endpoint_scrape },
{ .string = "attachments.add" , .length = 15, .function = &portal_endpoint_attachments_add },
{ .string = "attachments.progress" , .length = 20, .function = &portal_endpoint_attachments_progress },
{ .string = "attachments.remove" , .length = 18, .function = &portal_endpoint_attachments_remove },
{ .string = "config.load" , .length = 11, .function = &portal_endpoint_config_load },
{ .string = "config.edit" , .length = 11, .function = &portal_endpoint_config_edit },
{ .string = "contacts.add" , .length = 12, .function = &portal_endpoint_contacts_add },
{ .string = "contacts.edit" , .length = 13, .function = &portal_endpoint_contacts_edit },
{ .string = "contacts.list" , .length = 13, .function = &portal_endpoint_contacts_list },
{ .string = "contacts.load" , .length = 13, .function = &portal_endpoint_contacts_load },
{ .string = "contacts.move" , .length = 13, .function = &portal_endpoint_contacts_move },
{ .string = "contacts.copy" , .length = 13, .function = &portal_endpoint_contacts_copy },
{ .string = "contacts.remove" , .length = 15, .function = &portal_endpoint_contacts_remove },
{ .string = "folders.add" , .length = 11, .function = &portal_endpoint_folders_add },
{ .string = "folders.list" , .length = 12, .function = &portal_endpoint_folders_list },
{ .string = "folders.tags" , .length = 12, .function = &portal_endpoint_folders_tags },
{ .string = "folders.remove" , .length = 14, .function = &portal_endpoint_folders_remove },
{ .string = "folders.rename" , .length = 14, .function = &portal_endpoint_folders_rename },
{ .string = "messages.compose" , .length = 16, .function = &portal_endpoint_messages_compose },
{ .string = "messages.copy" , .length = 13, .function = &portal_endpoint_messages_copy },
{ .string = "messages.flag" , .length = 13, .function = &portal_endpoint_messages_flag },
{ .string = "messages.list" , .length = 13, .function = &portal_endpoint_messages_list },
{ .string = "messages.load" , .length = 13, .function = &portal_endpoint_messages_load },
{ .string = "messages.move" , .length = 13, .function = &portal_endpoint_messages_move },
{ .string = "messages.remove" , .length = 15, .function = &portal_endpoint_messages_remove },
{ .string = "messages.send" , .length = 13, .function = &portal_endpoint_messages_send },
{ .string = "messages.tag" , .length = 12, .function = &portal_endpoint_messages_tag },
{ .string = "messages.tags" , .length = 13, .function = &portal_endpoint_messages_tags },
{ .string = "search" , .length = 6, .function = &portal_endpoint_search },
{ .string = "logout" , .length = 6, .function = &portal_endpoint_logout },
{ .string = "settings.identity" , .length = 17, .function = &portal_settings_identity },
{ .string = "meta", .length = 4, .function = &portal_meta},
{ .string = "debug", .length = 5, .function = &portal_debug},
};

#endif

