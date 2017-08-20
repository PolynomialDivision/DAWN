#include <libubus.h>
#include <stdio.h>

#include "datastorage.h"
#include "networksocket.h"
#include "ubus.h"
#include "dawn_uci.h"

/* SSL TESTNG */

#include <openssl/bio.h>
#include <openssl/evp.h>

BIO_METHOD *   BIO_f_base64(void);

#define BUFSIZE 17
#define BUFSIZE_DIR 256


#include "crypto.h"

int main(int argc, char **argv) {
    const char *ubus_socket = NULL;
    int ch;

    char opt_broadcast_ip[BUFSIZE];
    char opt_broadcast_port[BUFSIZE];
    char opt_hostapd_dir[BUFSIZE_DIR];

    char shared_key[BUFSIZE_DIR];
    char iv[BUFSIZE_DIR];

    while ((ch = getopt(argc, argv, "cs:p:i:b:o:h:i:k:v:")) != -1) {
        switch (ch) {
            case 's':
                ubus_socket = optarg;
                break;
            case 'p':
                snprintf(opt_broadcast_port, BUFSIZE, "%s", optarg);
                printf("broadcast port: %s\n", opt_broadcast_port);
                break;
            case 'i':
                snprintf(opt_broadcast_ip, BUFSIZE, "%s", optarg);
                printf("broadcast ip: %s\n", opt_broadcast_ip);
                break;
            case 'o':
                snprintf(sort_string, SORT_NUM, "%s", optarg);
                printf("sort string: %s\n", sort_string);
                break;
            case 'h':
                snprintf(opt_hostapd_dir, BUFSIZE_DIR, "%s", optarg);
                printf("hostapd dir: %s\n", opt_hostapd_dir);
                hostapd_dir_glob = optarg;
                break;
            case 'k':
                snprintf(shared_key, BUFSIZE_DIR, "%s", optarg);
                printf("Key: %s\n", shared_key);
                break;
            case 'v':
                snprintf(iv, BUFSIZE_DIR, "%s", optarg);
                printf("IV: %s\n", iv);
                break;
            default:
                break;
        }
    }

    /* ----
     *  Testing encryption
     * ----
     */

    BIO *bio, *b64;
    char message[] = "Hello World \n";

    b64 = BIO_new(BIO_f_base64());
    bio = BIO_new_fp(stdout, BIO_NOCLOSE);
    bio = BIO_push(b64, bio);
    BIO_write(bio, message, strlen(message));
    BIO_flush(bio);

    BIO_free_all(bio);

    char msg[] = "{\"bssid\":\"a4:2b:b0:de:f1:fd\",\"freq\":5180,\"ht_supported\":true,\"vht_supported\":true,\"clients\":{\"78:02:f8:bc:ac:0b\":{\"auth\":true,\"assoc\":true,\"authorized\":true,\"preauth\":false,\"wds\":false,\"wmm\":true,\"ht\":true,\"vht\":true,\"wps\":false,\"mfp\":false,\"aid\":1}}}";
    gcrypt_init();
    gcrypt_set_key_and_iv(shared_key, iv);
    printf("Encrypting msg: %s\n", msg);
    char *enc = gcrypt_encrypt_msg(msg, strlen(msg) + 1);

    printf("Decrypting msg: %s\n", enc);

    printf("Sizeof: %d, Strlen: %d, Acutal: %d\n", sizeof(enc) * sizeof(char), strlen(enc), strlen(msg) + 1);

    char *dec = gcrypt_decrypt_msg(enc, strlen(msg) + 1);//sizeof(enc));
    printf("Message decrypted: %s\n", dec);
    free(enc);
    free(dec);

    /*
     * ----
     */

    argc -= optind;
    argv += optind;

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

    build_decoding_table();


    init_socket_runopts(opt_broadcast_ip, opt_broadcast_port, 1);

    pthread_t tid_probe;
    pthread_create(&tid_probe, NULL, &remove_array_thread, NULL);

    //pthread_t tid_client;
    //pthread_create(&tid_client, NULL, &remove_client_array_thread, NULL);

    pthread_t tid_get_client;
    pthread_create(&tid_get_client, NULL, &update_clients_thread, NULL);

    pthread_t tid_kick_clients;
    pthread_create(&tid_kick_clients, NULL, &kick_clients_thread, NULL);

    //pthread_create(&tid, NULL, &remove_thread, NULL);

    dawn_init_ubus(ubus_socket, opt_hostapd_dir);

    // free ressources
    pthread_mutex_destroy(&list_mutex);
    pthread_mutex_destroy(&probe_array_mutex);
    pthread_mutex_destroy(&client_array_mutex);

    //free_list(probe_list_head);
    base64_cleanup();

    return 0;
}