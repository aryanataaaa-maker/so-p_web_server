// Nama      : Arya Nata Adidjaya
// NIM       : 251524039
// Kelas     : 2B D4
// Deskripsi : Simple web server menggunakan bahasa C


// Serba-serbi library dan/atau header
#include <sys/socket.h>     // Untuk fungsi-fungsi inti socket: socket, bind, listen, accept, dll.
#include <netinet/in.h>     // Untuk membuat struktur alamat: sockaddr_in, htons, dan lainnya.
#include <unistd.h>         // Menyediakan tools seperti read(), write(), close(), dll.
#include <stdio.h>          // Menyediakan tools standar untuk print dan parsing teks (sscanf).
#include <stdlib.h>         // Menyediakan tools standar untuk memori dan exit().
#include <string.h>         // Menyediakan tools standar untuk penanganan teks (strcmp, strlen).
#include <arpa/inet.h>      // Digunakan untuk menulis dan mengonversi alamat IP spesifik.

int server_fd;
struct sockaddr_in server_addr;

int main() {

    // 1. Membuat suatu socket. AF_INET untuk IPv4 dan SOCK_STREAM untuk TCP.
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket gagal");
        exit(EXIT_FAILURE); // Tombol emergency jika terjadi error saat membuat socket
    } else {
        printf("Socket berhasil dibuat!\n");
    }

    // 2. Bind suatu socket. Mengaitkan socket dengan alamat IP dan nomor port.
    server_addr.sin_family = AF_INET;           // Address family yang digunakan (IPv4)
    server_addr.sin_addr.s_addr = INADDR_ANY;   // Menerima koneksi dari semua IP lokal
    server_addr.sin_port = htons(8080);         // Port 8080 diubah ke network byte order

    if (bind(server_fd,
             (struct sockaddr *)&server_addr,
             sizeof(server_addr)) < 0) {
        perror("bind gagal");
        exit(EXIT_FAILURE);
    }

    // 3. Menunggu untuk semua koneksi masuk (maksimal 10 antrean).
    if (listen(server_fd, 10) < 0) {
        perror("listen gagal");
        exit(EXIT_FAILURE);
    }
    printf("Sukses mendengar lewat port 8080\n");

    // 4. Menerima koneksi dan memproses siklus HTTP secara terus-menerus.
    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);

        // Mengambil satu klien dari antrean dan membuat jalur khusus (client_fd)
        int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
        if (client_fd < 0) {
            perror("accept gagal");
            continue;
        }

        // 5. Penanganan Request dari Client:
        // a) Menerima request mentah dari browser ke dalam buffer
        char method[8];
        char path[256];
        char buffer[1024];
        ssize_t baca = read(client_fd, buffer, sizeof(buffer) - 1);

        if (baca > 0) {
            buffer[baca] = '\0'; // Menandai akhir string agar aman dibaca
        } else {
            close(client_fd);
            continue;
        }

        // b) Parsing request: mengambil metode (GET) dan alamat tujuan (path)
        int jumlah_berhasil = sscanf(buffer, "%7s %255s", method, path);
        if (jumlah_berhasil < 2) {
            close(client_fd);
            continue;
        }

        // c) Menentukan response berdasarkan alamat URL yang diminta
        const char *response;
        if (strcmp(path, "/") == 0) {
            // Halaman Utama (200 OK)
            response = "HTTP/1.1 200 OK\r\n"
                       "Content-Type: text/html; charset=UTF-8\r\n"
                       "\r\n"
                       "<html><body><h1>Halaman Utama</h1><p>Halo dari server C!</p><a href='/tentang'>Ke Tentang</a></body></html>";
        } else if (strcmp(path, "/tentang") == 0) {
            // Halaman Tentang (200 OK)
            response = "HTTP/1.1 200 OK\r\n"
                       "Content-Type: text/html; charset=UTF-8\r\n"
                       "\r\n"
                       "<html><body><h1>Halaman Tentang</h1><p>Web server sederhana bahasa C.</p><a href='/'>Kembali</a></body></html>";
        } else {
            // Halaman Tidak Ditemukan (404 Not Found)
            response = "HTTP/1.1 404 Not Found\r\n"
                       "Content-Type: text/html; charset=UTF-8\r\n"
                       "\r\n"
                       "<html><body><h1>404 Not Found</h1><p>Halaman tidak ditemukan.</p></body></html>";
        }

        // 6. Pengiriman Response ke Client:
        // a) Mengirimkan HTTP header dan body HTML melalui socket klien
        write(client_fd, response, strlen(response));

        // b) Menutup koneksi klien setelah selesai dilayani
        close(client_fd);
    }

    close(server_fd);
    return 0;
}

/*
    Catatan:

        a) Program ini ditulis di OS Linux (Arch Linux).
        b) fd bermakna file descriptor. Pada Linux semuanya diperlakukan seperti file,
           jadi ketika kita memanggil socket(), sistem operasi akan mengembalikan bilangan bulat.
           Bilangan bulat itu berguna kapanpun kita ingin melakukan operasi bind, listen, atau close server socket nantinya,
           kita wajib memberikan nomor server_fd tersebut agar Linux tahu mana koneksi yang kita maksud.
        c) struct sockaddr_in server_addr; merupakan kontainer (sebuah struct) dari <netinet/in.h> yang dibuat
           secara spesifik untuk menyimpan detail alamat IPv4 dan _in di akhir berarti internet. Yang ditampung adalah address family,
           IP address, dan nomor port. Nantinya kontainer ini akan dilempar ke bind() agar Linux tahu di mana untuk memasang socket.
        d) Format HTTP response wajib memisahkan bagian Header dan Body dengan baris kosong (\r\n\r\n)
           agar browser dapat mengenali dan merender tag HTML dengan benar.
*/
