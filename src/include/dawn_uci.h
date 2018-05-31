#ifndef DAWN_UCI_H
#define DAWN_UCI_H

/**
 * Init uci. Call this function before using the other functions!
 * @return if call was successful.
 */
int uci_init();

/**
 * Clear uci. Call this function after using uci!
 * @return if call was successful.
 */
int uci_clear();

/**
 * Function that returns the metric for the load balancing sheme using uci.
 * @return the load balancing metric.
 */
struct probe_metric_s uci_get_dawn_metric();

/**
 * Function that returns a struct with all the time config values.
 * @return the time config values.
 */
struct time_config_s uci_get_time_config();

/**
 * Function that returns all the network informations.
 * @return the network config values.
 */
struct network_config_s uci_get_dawn_network();

/**
 * Function that returns the hostapd directory reading from the config file.
 * @return the hostapd directory.
 */
const char *uci_get_dawn_hostapd_dir();

/**
 * Function that returns the sort order.
 * @return the sort order.
 */
const char *uci_get_dawn_sort_order();

int uci_set_network(char* uci_cmd);

#endif //DAWN_UCI_H_H
