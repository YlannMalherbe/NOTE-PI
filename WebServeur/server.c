#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <arpa/inet.h>

#define PORT 8080
#define MAX_LEN 4096

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

const char *get_filename_ext(const char *filename) {
    const char *dot = strrchr(filename, '.');
    if(!dot || dot == filename) return "";
    return dot + 1;
}

const char* get_mime_type(const char* ext) {
    if (strcmp(ext, "html") == 0) return "text/html";
    if (strcmp(ext, "css")  == 0) return "text/css";
    if (strcmp(ext, "js")   == 0) return "application/javascript";
    if (strcmp(ext, "png")  == 0) return "image/png";
    if (strcmp(ext, "jpg")  == 0) return "image/jpeg";
    return "application/octet-stream";
}

int send_file(int client_fd, const char* filename) {

    FILE* f = fopen(filename, "rb");
    if (!f) {
        printf("Impossible d’ouvrir %s\n", filename);
        return -1;
    }

    // Taille du fichier avec stat()
    struct stat st;
    stat(filename, &st);
    long filesize = st.st_size;

    // Lecture du fichier entier
    char* body = malloc(filesize);
    fread(body, 1, filesize, f);
    fclose(f);

    // Déduire le MIME
    const char* ext = get_filename_ext(filename);
    const char* mime = get_mime_type(ext);

    // Construire l’entête
    char header[512];
    sprintf(header,
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %ld\r\n"
        "\r\n",
        mime, filesize
    );

    // Envoyer header + contenu
    send(client_fd, header, strlen(header), 0);
    send(client_fd, body, filesize, 0);

    free(body);
    return 0;
}


int main() {
    int server_fd, client_fd;
    struct sockaddr_in server, client;
    socklen_t client_len = sizeof(client);

    char buffer[MAX_LEN];

    // 1. Création de la socket TCP
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    // 2. Configuration de l’adresse
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));


    // 3. Association (bind)
    if (bind(server_fd, (struct sockaddr *)&server, sizeof(server)) == -1) {
        perror("bind");
        close(server_fd);
        return -1;
    }


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

        if (strcmp(url, "/") == 0) {
            send_file(client_fd, "/home/ylann/ProjetsPerso/NOTE-PI/WebPages/index.html");
            send_file(client_fd, "/home/ylann/ProjetsPerso/NOTE-PI/WebPages/style.css");
        }
        else {
            char path[512];
            sprintf(path, "/home/ylann/ProjetsPerso/NOTE-PI/WebPages/%s", url + 1);
            send_file(client_fd, path);
        }

        // 9. Fermer la connexion client
        close(client_fd);
    }

    close(server_fd);
    return 0;
}
