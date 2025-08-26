/* cc backup_pro.c -o backup_pro */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>
#include <unistd.h>

#define BACKUP_DIR "backups"
#define RETENTION_DAYS 7

// Funkcja tworzy katalog jesli nie istnieje
void ensure_backup_dir() {
    struct stat st;
    if (stat(BACKUP_DIR, &st) == -1) {
        if (mkdir(BACKUP_DIR, 0755) != 0) {
            perror("mkdir");
            exit(1);
        }
    }
}

// Funkcja usuwa stare backupy
void cleanup_old_backups() {
    char cmd[256];
    snprintf(cmd, sizeof(cmd),
        "find %s -type f -name 'backup_*.tar.gz' -mtime +%d -exec rm -v {} \\;",
        BACKUP_DIR, RETENTION_DAYS);
    system(cmd);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <source_dir>\n", argv[0]);
        return 1;
    }

    const char *src = argv[1];

    ensure_backup_dir();
    cleanup_old_backups();

    // Pobranie daty i godziny
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char timestamp[64];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d_%H-%M-%S", t);

    // Nazwa pliku backupu w katalogu backups/
    char backup_file[256];
    snprintf(backup_file, sizeof(backup_file), "%s/backup_%s.tar.gz", BACKUP_DIR, timestamp);

    // Polecenie tar
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "tar -czf %s -C %s .", backup_file, src);

    // Tworzenie backupu
    printf("Creating backup: %s\n", backup_file);
    int ret = system(cmd);

    if (ret == 0) {
        printf("Backup completed successfully!\n");
    } else {
        fprintf(stderr, "Backup failed with code %d\n", ret);
        return ret;
    }

    // Zapis logu w katalogu backupów
    char log_file[256];
    snprintf(log_file, sizeof(log_file), "%s/backup.log", BACKUP_DIR);
    FILE *log = fopen(log_file, "a");
    if (log) {
        fprintf(log, "%s: Backup created -> %s\n", timestamp, backup_file);
        fclose(log);
    } else {
        perror("backup.log");
    }

    return 0;
}
