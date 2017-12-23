#include <libubus.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

#include "datastorage.h"
#include "networksocket.h"
#include "ubus.h"
#include "dawn_uci.h"
#include "crypto.h"
#include "dawn_iwinfo.h"

void daemon_shutdown();

void signal_handler(int sig);

int init_mutex();

struct sigaction signal_action;

void daemon_shutdown() {

    // kill threads
    close_socket();
    uci_clear();
    printf("Cancelling Threads!\n");
    uloop_cancelled = true;

    // free ressources
    printf("Freeing mutex ressources\n");
    pthread_mutex_destroy(&list_mutex);
    pthread_mutex_destroy(&probe_array_mutex);
    pthread_mutex_destroy(&client_array_mutex);
    pthread_mutex_destroy(&ap_array_mutex);
}

void signal_handler(int sig) {
    switch (sig) {
        case SIGHUP:
            daemon_shutdown();
            break;
        case SIGINT:
            daemon_shutdown();
            break;
        case SIGTERM:
            daemon_shutdown();
            exit(EXIT_SUCCESS);
        default:
            daemon_shutdown();
            break;
    }
}

int init_mutex()
{
    if (pthread_mutex_init(&list_mutex, NULL) != 0) {
        printf("\n mutex init failed\n");
        return 1;
    }

    if (pthread_mutex_init(&probe_array_mutex, NULL) != 0) {
        printf("\n mutex init failed\n");
        return 1;
    }

    if (pthread_mutex_init(&client_array_mutex, NULL) != 0) {
        printf("\n mutex init failed\n");
        return 1;
    }

    if (pthread_mutex_init(&ap_array_mutex, NULL) != 0) {
        printf("\n mutex init failed\n");
        return 1;
    }
    return 0;
}

int main(int argc, char **argv) {

    const char *ubus_socket = NULL;

    argc -= optind;
    argv += optind;

    // connect signals
    signal_action.sa_handler = signal_handler;
    sigemptyset(&signal_action.sa_mask);
    signal_action.sa_flags = 0;
    sigaction(SIGHUP, &signal_action, NULL);
    sigaction(SIGTERM, &signal_action, NULL);
    sigaction(SIGINT, &signal_action, NULL);

    uci_init();
    struct network_config_s net_config = uci_get_dawn_network();
    printf("Broadcst bla: %s\n", net_config.broadcast_ip);

    gcrypt_init();
    gcrypt_set_key_and_iv(net_config.shared_key, net_config.iv);

    get_essid("wlan0", NULL);

    struct time_config_s time_config = uci_get_time_config();
    timeout_config = time_config; // TODO: Refactor...

    hostapd_dir_glob = uci_get_dawn_hostapd_dir();
    sort_string = (char*) uci_get_dawn_sort_order();

    init_mutex();

    init_socket_runopts(net_config.broadcast_ip, net_config.broadcast_port, net_config.bool_multicast);

    insert_macs_from_file();
    dawn_init_ubus(ubus_socket, hostapd_dir_glob);

    return 0;
}