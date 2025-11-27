#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080

char* parse_url(const char* request) {
    // Allouer de la mémoire pour travailler sur une copie
    char* copy = malloc(strlen(request) + 1);
    if (!copy) return NULL; // Vérifier l'allocation
    strcpy(copy, request);

    // Tokeniser
    strtok(copy, " ");        // Premier mot (ex: "GET")
    char* url = strtok(NULL, " "); // Deuxième mot (l'URL)
    if (!url) {
        free(copy);
        return NULL;
    }

    // Faire une copie du résultat pour le retourner
    char* result = malloc(strlen(url) + 1);
    if (!result) {
        free(copy);
        return NULL;
    }
    strcpy(result, url);

    free(copy); // libérer la copie temporaire
    return result;
}

int main() {
    int server_fd, client_fd;
    struct sockaddr_in server, client;
    socklen_t client_len = sizeof(client);

    char buffer[4096];

    // 1. Création de la socket TCP
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    // 2. Configuration de l’adresse
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);

    // 3. Association (bind)
    bind(server_fd, (struct sockaddr *)&server, sizeof(server));

    // 4. Mise en écoute
    listen(server_fd, 10);

    printf("Serveur HTTP lancé sur http://localhost:%d\n", PORT);

    while (1) {
        // 5. Attendre un client
        client_fd = accept(server_fd, (struct sockaddr *)&client, &client_len);

        // 6. Lire la requête
        int bytes = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
        buffer[bytes] = '\0';
        char* url = parse_url(buffer);
        printf("%s \n",url);
        printf("===== Requête reçue =====\n%s\n", buffer);

        // 7. Réponse HTTP minimale
        char *body = "Hello world!";
        char response[4096];

        sprintf(
            response,
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: %ld\r\n"
            "\r\n"
            "%s",
            strlen(body),
            body
        );

        // 8. Envoyer
        send(client_fd, response, strlen(response), 0);

        // 9. Fermer la connexion client
        close(client_fd);
    }

    close(server_fd);
    return 0;
}
