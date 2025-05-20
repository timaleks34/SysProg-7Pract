#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>

void find_in_documents(const char *document_path, const char *target) {
    FILE *document = fopen(document_path, "r");
    if (!document) {
        perror("Не смог открыть документ");
        return;
    }

    char text_line[1024];
    char original_line[1024];
    int counter = 0;
    const char *separators = " \t\n\r.,;:!?\"'(){}[]";

    while (fgets(text_line, sizeof(text_line), document)) {
        counter++;
        strcpy(original_line, text_line);
        
        size_t length = strlen(original_line);
        if (length > 0 && original_line[length-1] == '\n') {
            original_line[length-1] = '\0';
        }

        char *part = strtok(text_line, separators);
        int match = 0;
        
        while (part != NULL && !match) {
            if (strcasecmp(part, target) == 0) {
                printf("%s:%d:%s\n", document_path, counter, original_line);
                match = 1;
            }
            part = strtok(NULL, separators);
        }
    }

    fclose(document);
}

void find_in_folder(const char *folder_path, const char *target) {
    DIR *folder = opendir(folder_path);
    if (!folder) {
        perror("Не смог открыть папку");
        return;
    }

    struct dirent *item;
    while ((item = readdir(folder)) != NULL) {
        if (strcmp(item->d_name, ".") == 0 || strcmp(item->d_name, "..") == 0) {
            continue;
        }

        char full_path[PATH_MAX];
        snprintf(full_path, sizeof(full_path), "%s/%s", folder_path, item->d_name);

        struct stat item_info;
        if (lstat(full_path, &item_info) == -1) continue;

        if (S_ISDIR(item_info.st_mode)) {
            find_in_folder(full_path, target);
        } else if (S_ISREG(item_info.st_mode)) {
            find_in_documents(full_path, target);
        }
    }

    closedir(folder);
}

void show_help(char *program_name) {
    printf("Использование: %s [ДИРЕКТОРИЯ] ИСКОМОЕ_СЛОВО [ОПЦИИ]\n", program_name);
    printf("Опции:\n");
    printf("  -h, --help     Показывает справку\n");
    printf("Описание:\n");
    printf("  Ищет СЛОВО во всех текстовых файлах в указанной ДИРЕКТОРИИ и её поддиректориях\n");
    printf("  Если ДИРЕКТОРИЯ не указана, по умолчанию используется ~/files\n");
}

int main(int argc, char *argv[]) {
    for (int i = 1; i < argc; i++) {
        if ((strcmp(argv[i], "--help") == 0) || (strcmp(argv[i], "-h") == 0)) {
            show_help(argv[0]);
            return EXIT_SUCCESS;
        }
    }

    const char *home_folder = getenv("HOME");
    if (!home_folder) {
        fprintf(stderr, "Не определена домашняя папка\n");
        return EXIT_FAILURE;
    }

    char default_folder[PATH_MAX];
    snprintf(default_folder, sizeof(default_folder), "%s/files", home_folder);

    const char *search_folder = default_folder;
    const char *search_target = NULL;

    if (argc == 2) {
        search_target = argv[1];
    } else if (argc >= 3) {
        search_folder = argv[1];
        search_target = argv[2];
    }

    if (!search_target) {
        fprintf(stderr, "Нет поискового запроса\n");
        show_help(argv[0]);
        return EXIT_FAILURE;
    }

    struct stat folder_info;
    if (stat(search_folder, &folder_info) != 0 || !S_ISDIR(folder_info.st_mode)) {
        fprintf(stderr, "Папки '%s' не существует\n", search_folder);
        return EXIT_FAILURE;
    }

    find_in_folder(search_folder, search_target);
    return EXIT_SUCCESS;
}
