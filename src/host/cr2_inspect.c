/* SPDX-License-Identifier: GPL-2.0-only */
/* Inspecao SOMENTE LEITURA de CR2 no computador, sem tocar no CF.
 * Usa biblioteca upstream Soup, nao e um firmware Canon.
 */
#include "rebobina/cr2_host.h"
#include <stdio.h>
#include <stdlib.h>

#define RB_MAX_CR2_BYTES (64u * 1024u * 1024u)

int main(int argc, char **argv)
{
    FILE *input;
    unsigned char *data;
    rb_cr2_host_preview preview = {0};
    long n;
    int rc;
    if (argc != 2) {
        fputs("Uso: rebobina-cr2-inspect <arquivo.CR2>\n", stderr);
        return 2;
    }
    input = fopen(argv[1], "rb");
    if (!input) {
        fputs("Erro: nao foi possivel abrir o arquivo para leitura.\n", stderr);
        return 2;
    }
    if (fseek(input, 0, SEEK_END) != 0 ||
        (n = ftell(input)) < 16 || (unsigned long)n > RB_MAX_CR2_BYTES ||
        fseek(input, 0, SEEK_SET) != 0) {
        fputs("Erro: arquivo invalido ou maior que 64 MiB.\n", stderr);
        (void)fclose(input);
        return 2;
    }
    data = (unsigned char *)malloc((size_t)n);
    if (!data) {
        fputs("Erro: sem memoria no host.\n", stderr);
        (void)fclose(input);
        return 2;
    }
    if (fread(data, 1, (size_t)n, input) != (size_t)n) {
        fputs("Erro de leitura.\n", stderr);
        free(data);
        (void)fclose(input);
        return 2;
    }
    (void)fclose(input);
    rc = rb_cr2_host_find_preview(data, (size_t)n, &preview);
    if (rc == 0)
        printf("Preview JPEG localizado: %d x %d; offset=%u; bytes=%u\n",
               preview.width, preview.height, preview.offset, preview.bytes);
    else
        fputs("Nao foi possivel validar o preview JPEG do CR2.\n", stderr);
    free(data);
    return rc ? 1 : 0;
}
