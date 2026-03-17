#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <quadmath.h>   // требуется компиляция с -lquadmath

#define DECIMAL_PLACES 100  // Точность бесконечного числа

// Печать битов в строку для записи в файл
void sprint_ieee754(char *out, const void *ptr, size_t size, int exp_len) {
    const unsigned char *bytes = (const unsigned char *)ptr;
    int bit_idx = 0;
    int pos = 0;
    
    for (int i = (int)size - 1; i >= 0; --i) {
        for (int j = 7; j >= 0; --j) {
            int bit = (bytes[i] >> j) & 1;
            pos += sprintf(out + pos, "%d", bit);
            bit_idx++;
            if (bit_idx == 1 || bit_idx == 1 + exp_len) {
                pos += sprintf(out + pos, " ");
            }
        }
    }
}

// Строковое вычитание для поиска погрешности
void get_abs_diff(const char *s1, const char *s2, char *res, int len) {
    // Определяем, какая строка больше 
    const char *max_s = s1;
    const char *min_s = s2;
    if (strcmp(s1, s2) < 0) {
        max_s = s2;
        min_s = s1;
    }

    int borrow = 0;
    for (int i = len - 1; i >= 0; i--) {
        if (max_s[i] == '.') {
            res[i] = '.';
            continue;
        }
        // Начало числа (до точки) может быть короче, но у нас orig имеет формат "0.xxxx..."
        // Поэтому считаем, что все цифры присутствуют и i не выходит за пределы.
        int diff = (max_s[i] - '0') - (min_s[i] - '0') - borrow;
        if (diff < 0) {
            diff += 10;
            borrow = 1;
        } else {
            borrow = 0;
        }
        res[i] = (char)(diff + '0');
    }
    res[len] = '\0';
}

void generate_variant(int variant_id, int m, FILE *ans_file) {
    char filename[32];
    sprintf(filename, "variant_%d.txt", variant_id);
    FILE *v_file = fopen(filename, "w");
    if (!v_file) {
        perror("fopen");
        return;
    }
    
    fprintf(ans_file, "=== ВАРИАНТ %d ===\n", variant_id);
    
    for (int j = 1; j <= m; j++) {
        // Генерируем случайное число
        char orig[DECIMAL_PLACES + 4]; // + место для "0." и '\0'
        orig[0] = '0';
        orig[1] = '.';
        for (int k = 2; k < 2 + DECIMAL_PLACES; k++) {
            orig[k] = (char)('0' + (rand() % 10));
        }
        orig[2 + DECIMAL_PLACES] = '\0';

        // Записываем в файл ученику
        fprintf(v_file, "%s\n", orig);

        // Считаем ответы для преподавателя
        float f = strtof(orig, NULL);
        double d = strtod(orig, NULL);
        __float128 q = strtoflt128(orig, NULL);

        char f_bits[150], d_bits[150], q_bits[200];
        sprint_ieee754(f_bits, &f, 4, 8);
        sprint_ieee754(d_bits, &d, 8, 11);
        sprint_ieee754(q_bits, &q, 16, 15);

        char f_str[DECIMAL_PLACES+3], d_str[DECIMAL_PLACES+3], q_str[DECIMAL_PLACES+3];
        char f_err[DECIMAL_PLACES+4], d_err[DECIMAL_PLACES+4], q_err[DECIMAL_PLACES+4];
        
        snprintf(f_str, sizeof(f_str), "%.*f", DECIMAL_PLACES, f);
        snprintf(d_str, sizeof(d_str), "%.*f", DECIMAL_PLACES, d);
        quadmath_snprintf(q_str, sizeof(q_str), "%.*Qf", DECIMAL_PLACES, q);

        get_abs_diff(orig, f_str, f_err, DECIMAL_PLACES + 2);
        get_abs_diff(orig, d_str, d_err, DECIMAL_PLACES + 2);
        get_abs_diff(orig, q_str, q_err, DECIMAL_PLACES + 2);

        // Печатаем в answers.txt в виде таблицы
        fprintf(ans_file, "\n+--------------------------------------------------"
                          "-----------------------------+\n");
        fprintf(ans_file, "| Число %d: %-55s |\n", j, orig);
        fprintf(ans_file, "+-----------------+----------------------------------"
                          "-------------------+----------------------+\n");
        fprintf(ans_file, "| Тип             | Битовое представление (IEEE 754) "
                          "                    | Абсолютная погрешность |\n");
        fprintf(ans_file, "+-----------------+----------------------------------"
                          "-------------------+----------------------+\n");
        fprintf(ans_file, "| 32-bit (float)  | %-75s | %-20s |\n", f_bits, f_err);
        fprintf(ans_file, "| 64-bit (double) | %-75s | %-20s |\n", d_bits, d_err);
        fprintf(ans_file, "| 128-bit (quad)  | %-75s | %-20s |\n", q_bits, q_err);
        fprintf(ans_file, "+-----------------+----------------------------------"
                          "-------------------+----------------------+\n\n");
    }
    
    fclose(v_file);
    printf("Создан файл %s\n", filename);
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Использование: %s кол-во вариантов n чисел в варианте\n", argv[0]);
        return 1;
    }

    int n = atoi(argv[1]);
    int m = atoi(argv[2]);
    srand((unsigned int)time(NULL));

    FILE *ans_file = fopen("answers.txt", "w");
    if (!ans_file) {
        perror("fopen answers.txt");
        return 1;
    }

    for (int i = 1; i <= n; i++) {
        generate_variant(i, m, ans_file);
    }

    fclose(ans_file);
    printf("\nГотово! Ответы сохранены в answers.txt\n");
    return 0;
}
